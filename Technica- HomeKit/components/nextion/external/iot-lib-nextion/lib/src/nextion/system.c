#include <malloc.h>
#include <string.h>
#include "common_infra.h"
#include "nextion/nextion.h"
#include "nextion/system.h"

#ifdef __cplusplus
extern "C"
{
#endif

    nex_err_t nextion_system_get_text(nextion_handle_t handle,
                                      const char *command,
                                      size_t command_length,
                                      char *text,
                                      size_t expected_text_length,
                                      size_t *text_length)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((command != NULL), "command error(NULL)", NEX_FAIL);
        NEX_CHECK((text != NULL), "text error(NULL)", NEX_FAIL);

        expected_text_length += NEX_DVC_CMD_ACK_LENGTH;

        uint8_t *response = (uint8_t *)malloc(expected_text_length);
        size_t bytes_read = 0;

        if (!nextion_command_send_raw(handle,
                                      command,
                                      command_length,
                                      response,
                                      expected_text_length,
                                      NEXTION_RESP_MODE_NORMAL,
                                      &bytes_read))
        {
            return NEX_FAIL;
        }

        nex_err_t result;

        if (bytes_read > NEX_DVC_CMD_ACK_LENGTH && response[0] == NEX_DVC_RSP_GET_STRING)
        {
            const size_t length = bytes_read - NEX_DVC_CMD_ACK_LENGTH;

            text[length] = '\0';

            memcpy(text, response + NEX_DVC_CMD_START_LENGTH, length);

            if (text_length != NULL)
            {
                *text_length = length;
            }

            result = NEX_OK;
        }
        else if (bytes_read == NEX_DVC_CMD_ACK_LENGTH)
        {
            // In case of error it will send the basic ACK response.

            result = response[0];
        }
        else
        {
            result = NEX_FAIL;
        }

        free(response);

        return result;
    }

    nex_err_t nextion_system_get_number(nextion_handle_t handle,
                                        const char *command,
                                        size_t command_length,
                                        int32_t *number)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((command != NULL), "command error(NULL)", NEX_FAIL);
        NEX_CHECK((number != NULL), "number error(NULL)", NEX_FAIL);

        uint8_t response[8];
        size_t bytes_read = 0;

        if (!nextion_command_send_raw(handle,
                                      command,
                                      command_length,
                                      response,
                                      8,
                                      NEXTION_RESP_MODE_NORMAL,
                                      &bytes_read))
        {
            return NEX_FAIL;
        }

        if (bytes_read == 8 && response[0] == NEX_DVC_RSP_GET_NUMBER)
        {
            // Number: 4 bytes and signed = int32_t.
            // Sent in little endian format.
            *number = (int32_t)(((uint32_t)response[4] << 24) | ((uint32_t)response[3] << 16) | ((uint32_t)response[2] << 8) | (uint32_t)response[1]);

            return NEX_OK;
        }
        else if (bytes_read == NEX_DVC_CMD_ACK_LENGTH)
        {
            // In case of error it will send the basic ACK response.

            return response[0];
        }
        else
        {
            return NEX_FAIL;
        }
    }

    nex_err_t nextion_system_reset(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        uint8_t response[NEX_DVC_CMD_ACK_LENGTH];

        if (!nextion_command_send_raw(handle,
                                      "rest",
                                      4,
                                      response,
                                      NEX_DVC_CMD_ACK_LENGTH,
                                      NEXTION_RESP_MODE_ACCEPT_TIMEOUT,
                                      NULL))
        {
            return response[0];
        }

        return NEX_OK;
    }

    nex_err_t nextion_system_get_brightness(nextion_handle_t handle, bool persisted, uint8_t *percentage)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((percentage != NULL), "percentage error(NULL)", NEX_FAIL);

        int32_t value = 0;
        nex_err_t result;

        if (persisted)
        {
            result = nextion_system_get_number(handle, "get dims", 8, &value);
        }
        else
        {
            result = nextion_system_get_number(handle, "get dim", 7, &value);
        }

        *percentage = (uint8_t)value;

        return result;
    }

    nex_err_t nextion_system_set_brightness(nextion_handle_t handle, uint8_t percentage, bool persist)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        if (percentage > 100)
        {
            percentage = 100;
        }

        nex_err_t result;

        if (persist)
        {
            NEX_SEND_COMMAND(result, handle, 5 + INT8_CHAR_LENGTH, "dims=%d", percentage);
        }
        else
        {
            NEX_SEND_COMMAND(result, handle, 4 + INT8_CHAR_LENGTH, "dim=%d", percentage);
        }

        return result;
    }

    nex_err_t nextion_system_set_sleep_no_touch(nextion_handle_t handle, uint16_t seconds)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 5 + INT16_CHAR_LENGTH, "thsp=%d", seconds);
    }

    nex_err_t nextion_system_set_wake_on_touch(nextion_handle_t handle, bool wake_on_touch)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 5 + BOOL_CHAR_LENGTH, "thup=%d", wake_on_touch);
    }

#ifdef __cplusplus
}
#endif
