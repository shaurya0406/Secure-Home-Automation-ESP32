#include "common_infra_test.h"
#include "nextion/nextion.h"

TEST_GROUP_NEX_DISPLAY(NEX_CORE);

TEST(NEX_CORE, Create_handle)
{
    nextion_handle_t handle = nextion_create(&DUMMY_COMM);

    CHECK_NOT_NULL(handle);

    nextion_free(handle);
}

TEST(NEX_CORE, Cannot_create_null_comm)
{
    nextion_handle_t handle = nextion_create(NULL);

    CHECK_NULL(handle);
}

TEST(NEX_CORE, Free_handle)
{
    bool result = nextion_free(nextion_create(&DUMMY_COMM));

    CHECK_TRUE(result);
}

TEST(NEX_CORE, Cannot_free_null_handle)
{
    bool result = nextion_free(NULL);

    CHECK_FALSE(result);
}

TEST(NEX_CORE, Get_comm_object)
{
    nextion_handle_t handle = nextion_create(&DUMMY_COMM);
    nextion_comm_t *retrieved = nextion_comm_get(handle);

    CHECK_NOT_NULL(retrieved);
    CHECK_EQUAL(&DUMMY_COMM, retrieved);

    nextion_free(handle);
}

TEST(NEX_CORE, Cannot_get_comm_object_null_context)
{
    nextion_comm_t *comm = nextion_comm_get(NULL);

    CHECK_NULL(comm);
}

TEST(NEX_CORE, Cannot_init_null_context)
{
    nex_err_t result = nextion_init(NULL);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Send_command)
{
    nex_err_t result = nextion_command_send(handle, "page 0", 6);

    CHECK_NEX_OK(result);
}

TEST(NEX_CORE, Cannot_send_command_with_null_handle)
{
    nex_err_t result = nextion_command_send(NULL, "", 0);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Cannot_send_null_command)
{
    nex_err_t result = nextion_command_send(handle, NULL, 0);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Send_command_raw)
{
    uint8_t response[NEX_DVC_CMD_ACK_LENGTH];
    size_t bytes_read;

    bool result = nextion_command_send_raw(handle,
                                           "page 0",
                                           6,
                                           response,
                                           NEX_DVC_CMD_ACK_LENGTH,
                                           NEXTION_RESP_MODE_NORMAL,
                                           &bytes_read);

    CHECK_TRUE(result);
    SIZET_EQUAL(NEX_DVC_CMD_ACK_LENGTH, bytes_read);
}

TEST(NEX_CORE, Cannot_send_raw_command_with_null_handle)
{
    uint8_t response[1];
    size_t bytes_read;

    bool result = nextion_command_send_raw(NULL, "", 0, response, 0, NEXTION_RESP_MODE_NORMAL, &bytes_read);

    CHECK_FALSE(result);
}

TEST(NEX_CORE, Cannot_send_raw_null_command)
{
    uint8_t response[1];
    size_t bytes_read;

    bool result = nextion_command_send_raw(handle, NULL, 0, response, 0, NEXTION_RESP_MODE_NORMAL, &bytes_read);

    CHECK_FALSE(result);
}

TEST(NEX_CORE, Cannot_send_raw_command_with_null_response_buffer)
{
    size_t bytes_read;

    bool result = nextion_command_send_raw(handle, "", 0, NULL, 0, NEXTION_RESP_MODE_NORMAL, &bytes_read);

    CHECK_FALSE(result);
}

TEST(NEX_CORE, Cannot_send_raw_command_with_insufficient_response_buffer_size)
{
    uint8_t response[1];
    size_t bytes_read;

    bool result = nextion_command_send_raw(handle, "", 0, response, (NEX_DVC_CMD_ACK_LENGTH - 1), NEXTION_RESP_MODE_NORMAL, &bytes_read);

    CHECK_FALSE(result);
}

TEST(NEX_CORE, Transparent_data_mode_begin)
{
    nex_err_t result = nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, 1);

    nextion_transparent_data_mode_write(handle, 0);

    nextion_transparent_data_mode_end(handle);

    CHECK_NEX_OK(result);
}

TEST(NEX_CORE, Cannot_begin_transparent_data_mode_with_too_much_data)
{
    nex_err_t result = nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Transparent_data_mode_write)
{
    nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, 1);

    nex_err_t result = nextion_transparent_data_mode_write(handle, 0);

    nextion_transparent_data_mode_end(handle);

    CHECK_NEX_OK(result);
}

TEST(NEX_CORE, Cannot_write_on_unstarted_transparent_data_mode)
{
    nex_err_t result = nextion_transparent_data_mode_write(handle, 0);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Transparent_data_mode_end)
{
    nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, 1);

    nextion_transparent_data_mode_write(handle, 0);

    nex_err_t result = nextion_transparent_data_mode_end(handle);

    CHECK_NEX_OK(result);
}

TEST(NEX_CORE, Cannot_end_unstarted_transparent_data_mode)
{
    nex_err_t result = nextion_transparent_data_mode_end(handle);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Cannot_send_command_on_transparent_mode)
{
    nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, 1);

    nex_err_t result = nextion_command_send(handle, "page 1", 6);

    nextion_transparent_data_mode_write(handle, 0);
    nextion_transparent_data_mode_end(handle);

    CHECK_NEX_FAIL(result);
}

TEST(NEX_CORE, Cannot_send_raw_command_on_transparent_mode)
{
    nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, 1);

    uint8_t buffer[NEX_DVC_CMD_ACK_LENGTH];

    bool result = nextion_command_send_raw(handle,
                                           "page 1",
                                           6,
                                           buffer,
                                           NEX_DVC_CMD_ACK_LENGTH,
                                           NEXTION_RESP_MODE_NORMAL,
                                           NULL);

    nextion_transparent_data_mode_write(handle, 0);
    nextion_transparent_data_mode_end(handle);

    CHECK_FALSE(result);
}

TEST(NEX_CORE, Cannot_process_events_on_transparent_mode)
{
    nextion_transparent_data_mode_begin(handle, "wept 0,1", 8, 1);

    bool result = nextion_event_process(handle);

    nextion_transparent_data_mode_write(handle, 0);
    nextion_transparent_data_mode_end(handle);

    CHECK_FALSE(result);
}
