#include "common_infra_test.h"
#include "nextion/page.h"

TEST_GROUP_NEX_DISPLAY(NEX_OP_PAGE);

TEST(NEX_OP_PAGE, Get_current_page)
{
    nextion_page_set(handle, "1");

    uint8_t page_id;
    nex_err_t code = nextion_page_get(handle, &page_id);

    nextion_page_set(handle, 0);

    CHECK_NEX_OK(code);
    LONGS_EQUAL(1, page_id);
}

TEST(NEX_OP_PAGE, Change_page)
{
    nex_err_t code = nextion_page_set(handle, "1");

    nextion_page_set(handle, 0);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_PAGE, Cannot_change_to_page_with_invalid_id)
{
    nex_err_t code = nextion_page_set(handle, "nonecziste");

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_PAGE, code);
}

TEST(NEX_OP_PAGE, Refresh_page)
{
    nex_err_t code = nextion_page_refresh(handle);

    CHECK_NEX_OK(code);
}