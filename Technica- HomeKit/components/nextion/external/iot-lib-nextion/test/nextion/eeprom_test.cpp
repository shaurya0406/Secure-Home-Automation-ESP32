#include "common_infra_test.h"
#include "nextion/eeprom.h"

TEST_GROUP_NEX_DISPLAY(NEX_OP_EEPROM);

TEST(NEX_OP_EEPROM, Write_text)
{
    nex_err_t result = nextion_eeprom_write_text(handle, 0, "text", 4);

    CHECK_NEX_OK(result);
}

TEST(NEX_OP_EEPROM, Write_number)
{
    nex_err_t result = nextion_eeprom_write_number(handle, 20, 18);

    CHECK_NEX_OK(result);
}

TEST(NEX_OP_EEPROM, Read_text)
{
    const char *text = "sample text";
    char returned_text[12];

    nextion_eeprom_write_text(handle, 40, text, 11);

    nex_err_t result = nextion_eeprom_read_text(handle, 40, returned_text, 11);

    CHECK_NEX_OK(result);
    STRCMP_EQUAL(text, returned_text);
}

TEST(NEX_OP_EEPROM, Read_number)
{
    const int32_t number = 1988;
    int32_t returned_number = 0;

    nextion_eeprom_write_number(handle, 60, number);

    nex_err_t result = nextion_eeprom_read_number(handle, 60, &returned_number);

    CHECK_NEX_OK(result);
    LONGS_EQUAL(number, returned_number);
}

TEST(NEX_OP_EEPROM, Stream_works)
{
    if (nextion_eeprom_stream_begin(handle, 0, 50) != NEX_OK)
    {
        FAIL_TEST("Could not start streaming");
    }

    for (int i = 0; i < 50; i++)
    {
        if (nextion_eeprom_stream_write(handle, i) == NEX_FAIL)
        {
            nextion_eeprom_stream_end(handle);

            FAIL_TEST("Could not write value to stream");
        }
    }

    nextion_eeprom_stream_end(handle);
}

TEST(NEX_OP_EEPROM, Cannot_start_stream_with_invalid_address)
{
    nex_err_t code = nextion_eeprom_stream_begin(handle, NEX_DVC_EEPROM_MAX_ADDRESS + 1, 0);

    CHECK_NEX_FAIL(code);
}

TEST(NEX_OP_EEPROM, Cannot_start_stream_with_invalid_end_address)
{
    nex_err_t code = nextion_eeprom_stream_begin(handle, 0, NEX_DVC_EEPROM_MAX_ADDRESS + 1);

    CHECK_NEX_FAIL(code);
}

TEST(NEX_OP_EEPROM, Cannot_write_to_unstarted_stream)
{
    nex_err_t code = nextion_eeprom_stream_write(handle, 100);

    CHECK_NEX_FAIL(code);
}

TEST(NEX_OP_EEPROM, Cannot_end_unstarted_stream)
{
    nex_err_t code = nextion_eeprom_stream_end(handle);

    CHECK_NEX_FAIL(code);
}