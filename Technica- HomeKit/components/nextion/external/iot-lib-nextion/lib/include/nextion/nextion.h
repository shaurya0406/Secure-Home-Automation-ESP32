#ifndef __NEXTION_H__
#define __NEXTION_H__

#include <stdint.h>
#include "base/constants.h"
#include "base/codes.h"
#include "base/types.h"
#include "base/events.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Creates a new Nextion context.
     * @param comm Pointer to a communication port.
     * @return Pointer to a Nextion context or NULL if it failed.
     */
    nextion_handle_t nextion_create(nextion_comm_t *comm);

    /**
     * @brief Frees a Nextion context and its resources.
     * @param handle Nextion context pointer.
     * @return True if success, otherwise false.
     */
    bool nextion_free(nextion_handle_t handle);

    /**
     * @brief Does the initializations necessary before any action can be done.
     * @param handle Nextion context pointer.
     * @return NEX_OK if success, otherwise NEX_FAIL.
     */
    nex_err_t nextion_init(nextion_handle_t handle);

    /**
     * @brief Gets the communication port, which was passed on the "nextion_create".
     * @param handle Nextion context pointer.
     * @return Communication port if success, or NULL.
     */
    nextion_comm_t *nextion_comm_get(nextion_handle_t handle);

    /**
     * @brief Sends a command that waits for a simple response (ACK).
     *
     * @param handle Nextion context pointer.
     * @param command Command to be sent (null-terminated).
     * @param command_length Command length.
     *
     * @return NEX_OK if success, otherwise any NEX_DVC_ERR_* value.
     */
    nex_err_t nextion_command_send(nextion_handle_t handle,
                                   const char *command,
                                   size_t command_length);

    /**
     * @brief Sends a command and let the response handling to the caller.
     *
     * @details Will wait until a complete message is returned. Will not analyze
     * the response or handle the result.
     *
     * @note Use only if you know what you're doing.
     *
     * @param handle Nextion context pointer.
     * @param command Command to be sent.
     * @param command_length Command length.
     * @param response_buffer Buffer where the bytes received will be stored.
     * @param response_buffer_size Response buffer size, in bytes. Must be at least NEX_DVC_CMD_ACK_LENGTH.
     * @param response_mode Response mode.
     * @param bytes_read Location where the count of bytes read will be stored. Can be NULL.
     *
     * @return True if success, otherwise false.
     */
    bool nextion_command_send_raw(nextion_handle_t handle,
                                  const char *command,
                                  size_t command_length,
                                  uint8_t *response_buffer,
                                  size_t response_buffer_size,
                                  nextion_response_mode_type_t response_mode,
                                  size_t *bytes_read);

    /**
     * @brief Set events callbacks.
     * @param handle Nextion context pointer.
     * @param event_callback Callbacks used to handle device events.
     * @return True if success, otherwise false.
     */
    bool nextion_event_callback_set(nextion_handle_t handle, nextion_event_callback_t event_callback);

    /**
     * @brief Process all events in queue.
     * @param handle Nextion context pointer.
     * @return True if success, otherwise false.
     */
    bool nextion_event_process(nextion_handle_t handle);

    /**
     * @brief Begins the "Transparent Data Mode".
     *
     * @details When in this mode, the device "hangs" until all
     * data is sent; no event or other commands will be processed.
     *
     * @param handle Nextion context pointer.
     * @param command Command that will start it.
     * @param command_length Command length.
     * @param data_size How many bytes will be written. "(command_length + NEX_DVC_CMD_END_LENGTH + data_size) < NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE"
     *
     * @return NEX_OK if success, otherwise any NEX_DVC_ERR_* value.
     */
    nex_err_t nextion_transparent_data_mode_begin(nextion_handle_t handle,
                                                  const char *command,
                                                  size_t command_length,
                                                  size_t data_size);

    /**
     * @brief Writes a value onto the device serial buffer.
     *
     * @note Use only when in "Transparent Data Mode".
     *
     * @param handle Nextion context pointer.
     * @param byte Value to be written.
     *
     * @return NEX_OK if success, otherwise NEX_FAIL.
     */
    nex_err_t nextion_transparent_data_mode_write(nextion_handle_t handle, uint8_t value);

    /**
     * @brief Ends the "Transparent Data Mode".
     * @param handle Nextion context pointer.
     * @return NEX_OK if success, otherwise NEX_FAIL.
     */
    nex_err_t nextion_transparent_data_mode_end(nextion_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif //__NEXTION_H__