#include "common_infra_test.h"
#include "nextion/waveform.h"

TEST_GROUP_NEX_DISPLAY(NEX_OP_WAVEFORM);

#define TEST_WAVEFORM_ID 7U

TEST(NEX_OP_WAVEFORM, Start_waveform_refresh)
{
    nex_err_t code = nextion_waveform_start_refesh(handle);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_WAVEFORM, Stop_waveform_refresh)
{
    nex_err_t code = nextion_waveform_stop_refesh(handle);

    nextion_waveform_start_refesh(handle);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_WAVEFORM, Add_value_to_waveform)
{
    nex_err_t code = nextion_waveform_add_value(handle, TEST_WAVEFORM_ID, 0, 50);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_WAVEFORM, Cannot_add_value_to_invalid_waveform)
{
    nex_err_t code = nextion_waveform_add_value(handle, 50, 0, 50);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Cannot_add_value_to_invalid_waveform_channel)
{
    nex_err_t code = nextion_waveform_add_value(handle, TEST_WAVEFORM_ID, 5, 50);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Clear_waveform_channel)
{
    nex_err_t code = nextion_waveform_clear_channel(handle, TEST_WAVEFORM_ID, 0);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_WAVEFORM, Cannot_clear_waveform_channel_of_invalid_waveform)
{
    nex_err_t code = nextion_waveform_clear_channel(handle, 50, 0);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Cannot_clear_invalid_waveform_channel)
{
    nex_err_t code = nextion_waveform_clear_channel(handle, TEST_WAVEFORM_ID, 5);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Clear_waveform)
{
    nex_err_t code = nextion_waveform_clear(handle, TEST_WAVEFORM_ID);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_WAVEFORM, Cannot_clear_invalid_waveform)
{
    nex_err_t code = nextion_waveform_clear(handle, 50);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Stream_works)
{
    if (nextion_waveform_stream_begin(handle, TEST_WAVEFORM_ID, 0, 50) != NEX_OK)
    {
        FAIL_TEST("Could not start streaming");
    }

    for (int i = 0; i < 50; i++)
    {
        if (nextion_waveform_stream_write(handle, i) == NEX_FAIL)
        {
            nextion_waveform_stream_end(handle);

            FAIL_TEST("Could not write value to stream");
        }
    }

    nextion_waveform_stream_end(handle);
}

TEST(NEX_OP_WAVEFORM, Cannot_start_stream_with_invalid_waveform)
{
    nex_err_t code = nextion_waveform_stream_begin(handle, 50, 0, 50);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Cannot_start_stream_with_invalid_waveform_channel)
{
    nex_err_t code = nextion_waveform_stream_begin(handle, 50, 5, 50);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_WAVEFORM, code);
}

TEST(NEX_OP_WAVEFORM, Cannot_start_stream_with_invalid_value_count)
{
    nex_err_t code = nextion_waveform_stream_begin(handle, TEST_WAVEFORM_ID, 0, NEX_DVC_TRANSPARENT_DATA_MAX_DATA_SIZE);

    CHECK_NEX_FAIL(code);
}

TEST(NEX_OP_WAVEFORM, Cannot_write_to_unstarted_stream)
{
    nex_err_t code = nextion_waveform_stream_write(handle, 100);

    CHECK_NEX_FAIL(code);
}

TEST(NEX_OP_WAVEFORM, Cannot_end_unstarted_stream)
{
    nex_err_t code = nextion_waveform_stream_end(handle);

    CHECK_NEX_FAIL(code);
}