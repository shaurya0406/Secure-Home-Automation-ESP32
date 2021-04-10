#include <malloc.h>
#include "common_infra.h"
#include "nextion/nextion.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool nextion_comm_command_write(nextion_handle_t handle, const char *command, size_t command_length);
    bool nextion_comm_message_read(nextion_handle_t handle, uint8_t *buffer, size_t buffer_length, size_t *bytes_read);
    bool nextion_comm_message_read_timeout(nextion_handle_t handle, uint8_t *buffer, size_t buffer_length, size_t *bytes_read, bool *has_timed_out);
    bool nextion_comm_block_read_strict(nextion_handle_t handle, uint8_t *buffer, size_t buffer_length);
    bool nextion_comm_event_dispatch(nextion_handle_t handle, const uint8_t *buffer, const size_t buffer_length);

    /**
     * @struct nextion_t
     * @brief Holds control data for a context.
     */
    struct nextion_t
    {
        nextion_comm_t *comm;                    /*!< Pointer for a communication port. */
        size_t transparent_data_mode_size;       /*!< How many bytes are expected to be written while in "Transparent Data Mode". */
        nextion_event_callback_t event_callback; /*!< Callbacks for events. */
        bool transparent_data_mode_active;       /*!< If the device is in "Transparent Data Mode". */
        bool init_ok;                            /*!< If the device has initiated successfully. */
    };

    nextion_handle_t nextion_create(nextion_comm_t *comm)
    {
        NEX_CHECK((comm != NULL), "comm error(NULL)", NULL);

        nextion_t *nextion = (nextion_t *)malloc(sizeof(nextion_t));

        nextion->comm = comm;
        nextion->transparent_data_mode_active = false;
        nextion->transparent_data_mode_size = 0;
        nextion->init_ok = false;

        return nextion;
    }

    bool nextion_free(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, false);
        NEX_CHECK((handle->comm->vtable->free(handle->comm) == NEX_COMM_OK), "comm error(free method)", NEX_FAIL);

        handle->comm = NULL;

        free(handle);

        return true;
    }

    nex_err_t nextion_init(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        // This method might be called after reseting the device ("nextion_reset").
        // As only the device is reset and not the communication port, there is no
        // need to re-init the communication port.

        if (!handle->init_ok)
        {
            NEX_CHECK((handle->comm->vtable->init(handle->comm) == NEX_COMM_OK), "comm error(init method)", NEX_FAIL);

            handle->init_ok = true;
        }

        // All logic relies on receiving responses in all cases.

        return nextion_command_send(handle, "bkcmd=3", 7);
    }

    nextion_comm_t *nextion_comm_get(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NULL);

        return handle->comm;
    }

    nex_err_t nextion_command_send(nextion_handle_t handle,
                                   const char *command,
                                   size_t command_length)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((command != NULL), "command error(NULL)", NEX_FAIL);
        NEX_CHECK((handle->transparent_data_mode_active == false), "state error(in transparent data mode)", NEX_FAIL);

        uint8_t response[NEX_DVC_CMD_ACK_LENGTH];
        size_t bytes_read;

        if (!nextion_command_send_raw(handle,
                                      command,
                                      command_length,
                                      response,
                                      NEX_DVC_CMD_ACK_LENGTH,
                                      NEXTION_RESP_MODE_NORMAL,
                                      &bytes_read))
        {
            NEX_LOGE("Failure on sending raw command");

            return NEX_FAIL;
        }

        if (bytes_read != NEX_DVC_CMD_ACK_LENGTH)
        {
            NEX_LOGE("Response length less than %d", NEX_DVC_CMD_ACK_LENGTH);

            return NEX_FAIL;
        }

        uint8_t code = response[0];

        if (code == NEX_DVC_INSTRUCTION_OK)
            return NEX_OK;

        if (code == NEX_DVC_INSTRUCTION_FAIL)
        {
            NEX_LOGE("Device failure or command not recognized");

            return NEX_FAIL;
        }

        return code;
    }

    bool nextion_command_send_raw(nextion_handle_t handle,
                                  const char *command,
                                  size_t command_length,
                                  uint8_t *response_buffer,
                                  size_t response_buffer_size,
                                  nextion_response_mode_type_t response_mode,
                                  size_t *bytes_read)
    {
        NEX_CHECK_HANDLE(handle, false);
        NEX_CHECK((command != NULL), "command error(NULL)", false);
        NEX_CHECK((response_buffer != NULL), "response_buffer error(NULL)", false);
        NEX_CHECK((handle->transparent_data_mode_active == false), "state error(in transparent data mode)", false);
        NEX_CHECK((response_buffer_size >= NEX_DVC_CMD_ACK_LENGTH || response_mode == NEXTION_RESP_MODE_STRICT_SIZE), "response_buffer_size error(<NEX_DVC_CMD_ACK_LENGTH)", false)

        // Remove events from the comm port buffer.

        if (!nextion_event_process(handle))
        {
            NEX_LOGE("Failure on processing events");

            return false;
        }

        // Send the command to the device,

        if (!nextion_comm_command_write(handle, command, command_length))
        {
            NEX_LOGE("Failure on writing command");

            return false;
        }

        size_t length;

        if (response_mode == NEXTION_RESP_MODE_STRICT_SIZE)
        {
            // On strict mode we just read enough bytes to fill the response buffer.
            // It must fill the response buffer.

            if (!nextion_comm_block_read_strict(handle, response_buffer, response_buffer_size))
            {
                NEX_LOGE("Failure on reading strict block");

                return false;
            }

            length = response_buffer_size;
        }
        else
        {
            bool has_timed_out;
            uint8_t code;

            if (!nextion_comm_message_read_timeout(handle, response_buffer, response_buffer_size, &length, &has_timed_out))
            {
                NEX_LOGE("Failure on reading response");

                return false;
            }

            if (has_timed_out)
            {
                if (response_mode == NEXTION_RESP_MODE_ACCEPT_TIMEOUT)
                {
                    code = NEX_DVC_INSTRUCTION_OK;
                    length = 0;
                }
                else
                {
                    NEX_LOGE("Response timed out");

                    return false;
                }
            }
            else
            {
                code = response_buffer[0];
            }

            // It may happen that a message is an event and not a response.
            // We always need to check.
            // Getting an event here is an error, because we've already
            // removed all events from the ringbuffer calling "nextion_event_process"
            // before trying to parse the response.

            if (!NEX_DVC_CODE_IS_RESPONSE(code, length))
            {
                NEX_LOGE("Response code not expected");

                return false;
            }
        }

        if (bytes_read != NULL)
        {
            *bytes_read = length;
        }

        return true;
    }

    bool nextion_event_callback_set(nextion_handle_t handle, nextion_event_callback_t event_callback)
    {
        NEX_CHECK_HANDLE(handle, false);

        handle->event_callback = event_callback;

        return true;
    }

    bool nextion_event_process(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, false);
        NEX_CHECK((handle->transparent_data_mode_active == false), "state error(in transparent data mode)", false);

        uint8_t buffer[NEX_DVC_EVT_MAX_RESPONSE_LENGTH];
        size_t bytes_read;

        do
        {
            bool has_timed_out;

            if (!nextion_comm_message_read_timeout(handle, buffer, NEX_DVC_EVT_MAX_RESPONSE_LENGTH, &bytes_read, &has_timed_out))
            {
                NEX_LOGE("Failure on reading response");

                return false;
            }

            if (has_timed_out)
                return true;

            // It may happen that a message is a response and not an event.
            // We always need to check.
            // Getting a response here is not a problem, we just stop processing
            // and live the buffer untouched.

            if (NEX_DVC_CODE_IS_EVENT(buffer[0], bytes_read) && !nextion_comm_event_dispatch(handle, buffer, bytes_read))
            {
                return false;
            }

            // It will end when:
            // A - No more data; timeout.
            // B - Parsed a response message instead of an event.

            break;

        } while (true);

        return true;
    }

    nex_err_t nextion_transparent_data_mode_begin(nextion_handle_t handle,
                                                  const char *command,
                                                  size_t command_length,
                                                  size_t data_size)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((command != NULL), "command error(NULL)", NEX_FAIL);
        NEX_CHECK((handle->transparent_data_mode_active == false), "state error(in transparent data mode)", NEX_FAIL);
        NEX_CHECK((data_size > 0), "data_size error(<1)", NEX_FAIL);
        NEX_CHECK(((command_length + NEX_DVC_CMD_END_LENGTH + data_size) < NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE), "data_size error((command_length + NEX_DVC_CMD_END_LENGTH + data_size) >= NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE)", NEX_FAIL);

        nex_err_t result = nextion_command_send(handle, command, command_length);

        if (result == NEX_DVC_RSP_TRANSPARENT_DATA_READY)
        {
            handle->transparent_data_mode_active = true;
            handle->transparent_data_mode_size = data_size;

            return NEX_OK;
        }

        return result;
    }

    nex_err_t nextion_transparent_data_mode_write(nextion_handle_t handle, uint8_t value)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((handle->transparent_data_mode_active), "state error(not in transparent data mode)", NEX_FAIL);
        NEX_CHECK((handle->transparent_data_mode_size > 0), "state error(all data was written)", NEX_FAIL);

        const uint8_t buffer[1] = {value};

        if (handle->comm->vtable->write(handle->comm, buffer, 1) != NEX_COMM_OK)
        {
            NEX_LOGE("Failure on writing to the communication port");

            return NEX_FAIL;
        }

        handle->transparent_data_mode_size--;

        return NEX_OK;
    }

    nex_err_t nextion_transparent_data_mode_end(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((handle->transparent_data_mode_active), "state error(not in transparent data mode)", NEX_FAIL);
        NEX_CHECK((handle->transparent_data_mode_size == 0), "state error(not all data was written)", NEX_FAIL);

        uint8_t buffer[NEX_DVC_CMD_ACK_LENGTH];
        uint8_t code;
        size_t bytes_read;

        if (!nextion_comm_message_read(handle, buffer, NEX_DVC_CMD_ACK_LENGTH, &bytes_read))
        {
            NEX_LOGE("Failure on reading response");

            return NEX_FAIL;
        }

        if (bytes_read < NEX_DVC_CMD_ACK_LENGTH)
        {
            NEX_LOGE("Response length less than %d", NEX_DVC_CMD_ACK_LENGTH);

            return NEX_FAIL;
        }

        code = buffer[0];

        // Here we must have a response message.
        // When in transparent data mode, the device does not process
        // anything but the data written while in said mode.

        if (!NEX_DVC_CODE_IS_RESPONSE(code, bytes_read))
        {
            NEX_LOGE("Response code not expected");

            return NEX_FAIL;
        }

        if (code != NEX_DVC_RSP_TRANSPARENT_DATA_FINISHED)
        {
            NEX_LOGE("Response code is not data finished");

            return NEX_FAIL;
        }

        handle->transparent_data_mode_active = false;

        return NEX_OK;
    }

    /* ========================================
     *       Internal Communication Functions
     * ========================================*/

    /**
     * @brief Writes a command to a communication port.
     *
     * @param handle Nextion context pointer.
     * @param command Command to be sent.
     * @param command_length Command length.
     *
     * @return True if success, otherwise false.
     */
    bool nextion_comm_command_write(nextion_handle_t handle, const char *command, size_t command_length)
    {
        const uint8_t END_SEQUENCE[NEX_DVC_CMD_END_LENGTH] = {NEX_DVC_CMD_END_SEQUENCE};

        if (handle->comm->vtable->write(handle->comm, (const uint8_t *)command, command_length) != NEX_COMM_OK)
        {

            NEX_LOGE("Failure on writing command body to the communication port");
            return false;
        }

        if (handle->comm->vtable->write(handle->comm, END_SEQUENCE, NEX_DVC_CMD_END_LENGTH) != NEX_COMM_OK)
        {
            NEX_LOGE("Failure on writing command ending to the communication port");

            return false;
        }

        return true;
    }

    /**
     * @brief Reads a block of bytes from a communication port.
     * @param handle Nextion context pointer.
     * @param buffer Buffer to store the bytes.
     * @param buffer_length Buffer length.
     * @param bytes_read Bytes read.
     * @return True if success, otherwise false.
     */
    bool nextion_comm_message_read(nextion_handle_t handle, uint8_t *buffer, size_t buffer_length, size_t *bytes_read)
    {
        bool has_timed_out;

        if (!nextion_comm_message_read_timeout(handle, buffer, buffer_length, bytes_read, &has_timed_out))
        {
            NEX_LOGE("Failure on reading response");

            return false;
        }

        return !has_timed_out;
    }

    /**
     * @brief Reads a block of bytes from a communication port, with timeout handling.
     * @param handle Nextion context pointer.
     * @param buffer Buffer to store the bytes.
     * @param buffer_length Buffer length.
     * @param bytes_read Bytes read.
     * @param has_timed_out If the read timed out while waiting for a payload.
     * @return True if success, otherwise false.
     */
    bool nextion_comm_message_read_timeout(nextion_handle_t handle, uint8_t *buffer, size_t buffer_length, size_t *bytes_read, bool *has_timed_out)
    {
        *has_timed_out = false;
        *bytes_read = 0;

        uint8_t end_count = 0;
        size_t index = 0;
        uint8_t value = 0;

        while (index < buffer_length)
        {
            nex_comm_err_t read_response = handle->comm->vtable->read(handle->comm, &value);

            if (read_response == NEX_COMM_FAIL)
            {
                NEX_LOGE("Failure on reading from communication port");

                return false;
            }

            // Some commands, even with the "bkcmd=3" will
            // return a response only when failed.

            if (read_response == NEX_COMM_TIMEOUT && index == 0)
            {
                *has_timed_out = true;

                return true;
            }

            buffer[index] = value;

            index++;

            if (value == NEX_DVC_CMD_END_VALUE)
            {
                end_count++;

                if (end_count == NEX_DVC_CMD_END_LENGTH)
                {
                    break;
                }

                continue;
            }

            end_count = 0;
        }

        *bytes_read = index;

        return true;
    }

    /**
     * @brief Reads a specific length of bytes from a communication port.
     * @param handle Nextion context pointer.
     * @param buffer Buffer to store the bytes.
     * @param buffer_length Buffer length.
     * @return True if success, otherwise false.
     */
    bool nextion_comm_block_read_strict(nextion_handle_t handle, uint8_t *buffer, size_t buffer_length)
    {
        size_t bytes_read = 0;

        if (!nextion_comm_message_read(handle, buffer, buffer_length, &bytes_read))
        {
            NEX_LOGE("Failure on reading");

            return false;
        }

        return bytes_read == buffer_length;
    }

    /**
     * @brief Dispatches an event to a callback.
     * @param handle Nextion context pointer.
     * @param buffer Buffer containing event data.
     * @param buffer_length Buffer length.
     * @return True if success, otherwise, false.
     */
    bool nextion_comm_event_dispatch(nextion_handle_t handle, const uint8_t *buffer, const size_t buffer_length)
    {
        if (buffer_length < NEX_DVC_CMD_ACK_LENGTH)
            return false;

        const uint8_t code = buffer[0];

        switch (code)
        {
        case NEX_DVC_EVT_TOUCH_OCCURRED:
            if (buffer_length == 7 && handle->event_callback.on_touch != NULL)
            {
                handle->event_callback.on_touch(handle, buffer[1], buffer[2], buffer[3]);
            }
            break;

        case NEX_DVC_EVT_TOUCH_COORDINATE_AWAKE:
        case NEX_DVC_EVT_TOUCH_COORDINATE_ASLEEP:
            if (buffer_length == 9 && handle->event_callback.on_touch_coord != NULL)
            {
                // Coordinates: 2 bytes and unsigned = uint16_t.
                // Sent in big endian format.

                handle->event_callback.on_touch_coord(handle,
                                                      (uint16_t)(((uint16_t)buffer[1] << 8) | (uint16_t)buffer[2]),
                                                      (uint16_t)(((uint16_t)buffer[3] << 8) | (uint16_t)buffer[4]),
                                                      buffer[5],
                                                      code == NEX_DVC_EVT_TOUCH_COORDINATE_ASLEEP);
            }
            break;

        default:
            if (handle->event_callback.on_device != NULL)
            {
                handle->event_callback.on_device(handle, code);
            }
        }

        return true;
    }

#ifdef __cplusplus
}
#endif
