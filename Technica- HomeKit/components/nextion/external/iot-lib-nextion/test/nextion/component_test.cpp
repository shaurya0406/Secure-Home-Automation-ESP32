#include "common_infra_test.h"
#include "nextion/component.h"

TEST_GROUP_NEX_DISPLAY(NEX_OP_COMPONENT);

TEST(NEX_OP_COMPONENT, Refresh_component)
{
    nex_err_t code = nextion_component_refresh(handle, "n0");

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Cannot_refresh_invalid_component)
{
    nex_err_t code = nextion_component_refresh(handle, "n99");

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_COMPONENT, code);
}

TEST(NEX_OP_COMPONENT, Set_component_visibility)
{
    nex_err_t code = nextion_component_set_visibility(handle, "n0", false);

    nextion_component_set_visibility(handle, "n0", true);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Cannot_set_invalid_component_visibility)
{
    nex_err_t code = nextion_component_set_visibility(handle, "n99", false);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_COMPONENT, code);
}

TEST(NEX_OP_COMPONENT, Set_all_components_visibility)
{
    nex_err_t code = nextion_component_set_visibility_all(handle, false);

    nextion_component_set_visibility_all(handle, true);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Set_component_touchability)
{
    nex_err_t code = nextion_component_set_touchable(handle, "t0", false);

    nextion_component_set_touchable(handle, "t0", true);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Cannot_set_invalid_component_touchability)
{
    nex_err_t code = nextion_component_set_touchable(handle, "t99", false);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_COMPONENT, code);
}

TEST(NEX_OP_COMPONENT, Set_all_components_touchability)
{
    nex_err_t code = nextion_component_set_touchable_all(handle, false);

    nextion_component_set_touchable_all(handle, true);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Get_component_text)
{
    char text[10];
    size_t length;
    nex_err_t code = nextion_component_get_text(handle, "t0", text, 10, &length);

    CHECK_NEX_OK(code);
    SIZET_EQUAL(9, length);
    STRCMP_EQUAL("test text", text);
}

TEST(NEX_OP_COMPONENT, Get_component_number)
{
    int number;
    nex_err_t code = nextion_component_get_number(handle, "n0", &number);

    CHECK_NEX_OK(code);
    LONGS_EQUAL(50, number);
}

TEST(NEX_OP_COMPONENT, Get_component_boolean)
{
    bool value;
    nex_err_t code = nextion_component_get_boolean(handle, "c0", &value);

    CHECK_NEX_OK(code);
    CHECK_TRUE(value);
}

TEST(NEX_OP_COMPONENT, Set_component_text)
{
    nex_err_t code = nextion_component_set_text(handle, "b0", "Button!", 7);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Set_component_number)
{
    nex_err_t code = nextion_component_set_number(handle, "x0", 100);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_COMPONENT, Set_component_boolean)
{
    nex_err_t code = nextion_component_set_boolean(handle, "r0", false);

    CHECK_NEX_OK(code);
}
