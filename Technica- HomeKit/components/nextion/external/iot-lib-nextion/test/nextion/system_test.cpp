#include "Windows.h"
#include "common_infra_test.h"
#include "nextion/system.h"

TEST_GROUP_NEX_DISPLAY(NEX_OP_SYSTEM);

TEST(NEX_OP_SYSTEM, Get_text_from_text_component)
{
    char text[10];
    size_t length;
    nex_err_t code = nextion_system_get_text(handle, "get t0.txt", 10, text, 10, &length);

    CHECK_NEX_OK(code);
    SIZET_EQUAL(9, length);
    STRCMP_EQUAL("test text", text);
}

TEST(NEX_OP_SYSTEM, Cannot_get_text_from_invalid_text_component)
{
    char text[10];
    size_t length;
    nex_err_t code = nextion_system_get_text(handle, "get t99.txt", 11, text, 10, &length);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_VARIABLE_OR_ATTRIBUTE, code);
}

TEST(NEX_OP_SYSTEM, Get_number_from_number_component)
{
    int32_t number;
    nex_err_t code = nextion_system_get_number(handle, "get n0.val", 10, &number);

    CHECK_NEX_OK(code);
    LONGS_EQUAL(50, number);
}

TEST(NEX_OP_SYSTEM, Cannot_get_number_from_invalid_number_component)
{
    int32_t number;
    nex_err_t code = nextion_system_get_number(handle, "get n99.val", 11, &number);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_VARIABLE_OR_ATTRIBUTE, code);
}

TEST(NEX_OP_SYSTEM, Get_display_brightness)
{
    uint8_t percentage;
    nex_err_t code = nextion_system_get_brightness(handle, false, &percentage);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_SYSTEM, Get_persisted_display_brightness)
{
    uint8_t percentage;
    nex_err_t code = nextion_system_get_brightness(handle, true, &percentage);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_SYSTEM, Cannot_get_display_brightness_when_percentage_null)
{
    nex_err_t code = nextion_system_get_brightness(handle, false, NULL);

    CHECK_NEX_FAIL(code);
}

TEST(NEX_OP_SYSTEM, Set_display_brightness)
{
    uint8_t percentage;

    nextion_system_get_brightness(handle, false, &percentage);

    nex_err_t code = nextion_system_set_brightness(handle, percentage, false);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_SYSTEM, Set_persisted_display_brightness)
{
    uint8_t percentage;

    nextion_system_get_brightness(handle, true, &percentage);

    nex_err_t code = nextion_system_set_brightness(handle, percentage, true);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_SYSTEM, Set_sleep_on_no_touch)
{
    nex_err_t code = nextion_system_set_sleep_no_touch(handle, 60);

    nextion_system_set_sleep_no_touch(handle, 0);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_SYSTEM, Set_wake_up_when_touched)
{
    nex_err_t code = nextion_system_set_wake_on_touch(handle, true);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_SYSTEM, Reset)
{
    nex_err_t code = nextion_system_reset(handle);

    CHECK_NEX_OK(code);

    Sleep(100);

    // As all logic relies on "bkcmd=3" and this configuration
    // is lost on reset, we need to configure it again.
    nextion_init(handle);
}