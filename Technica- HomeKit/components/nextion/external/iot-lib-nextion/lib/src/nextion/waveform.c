#include "common_infra.h"
#include "nextion/nextion.h"
#include "nextion/waveform.h"

#ifdef __cplusplus
extern "C"
{
#endif

    nex_err_t nextion_waveform_start_refesh(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 8, "ref_star");
    }

    nex_err_t nextion_waveform_stop_refesh(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 8, "ref_stop");
    }

    nex_err_t nextion_waveform_add_value(nextion_handle_t handle,
                                         uint8_t waveform_id,
                                         uint8_t channel_id,
                                         uint8_t value)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        size_t command_length = 6 + (3 * INT8_CHAR_LENGTH);
        char command[6 + (3 * INT8_CHAR_LENGTH)];

        command_length = snprintf(command, command_length, "add %d,%d,%d", waveform_id, channel_id, value);

        uint8_t response[NEX_DVC_CMD_ACK_LENGTH];

        // This operation does not respects the "bkcmd" value.
        // Will only return in case of failure.

        size_t bytes_read = 0;

        if (!nextion_command_send_raw(handle,
                                      command,
                                      command_length,
                                      response,
                                      NEX_DVC_CMD_ACK_LENGTH,
                                      NEXTION_RESP_MODE_ACCEPT_TIMEOUT,
                                      &bytes_read))
        {
            return NEX_FAIL;
        }

        if (bytes_read == 0)
            return NEX_OK;

        return response[0];
    }

    nex_err_t nextion_waveform_clear_channel(nextion_handle_t handle,
                                             uint8_t waveform_id,
                                             uint8_t channel_id)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                5 + (2 * INT8_CHAR_LENGTH),
                                "cle %d,%d",
                                waveform_id, channel_id);
    }

    nex_err_t nextion_waveform_clear(nextion_handle_t handle, uint8_t waveform_id)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 8 + INT8_CHAR_LENGTH, "cle %d,255", waveform_id);
    }

    nex_err_t nextion_waveform_stream_begin(nextion_handle_t handle,
                                            uint8_t waveform_id,
                                            uint8_t channel_id,
                                            size_t value_count)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((value_count < (NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE - 20)), "value_count error(>=NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE-20)", NEX_FAIL);

        size_t command_length = 7 + (2 * INT8_CHAR_LENGTH) + SIZET_CHAR_LENGTH;
        char command[7 + (3 * INT8_CHAR_LENGTH) + SIZET_CHAR_LENGTH];

        command_length = snprintf(command, command_length, "addt %d,%d,%zd", waveform_id, channel_id, value_count);

        return nextion_transparent_data_mode_begin(handle, command, command_length, value_count);
    }

    nex_err_t nextion_waveform_stream_write(nextion_handle_t handle, uint8_t value)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        return nextion_transparent_data_mode_write(handle, value);
    }

    nex_err_t nextion_waveform_stream_end(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        return nextion_transparent_data_mode_end(handle);
    }
#ifdef __cplusplus
}
#endif