#include "common_infra_test.h"
#include "nextion/drawing.h"

TEST_GROUP_NEX_DISPLAY(NEX_OP_DRAW);

TEST(NEX_OP_DRAW, Fill_screen)
{
    nex_err_t code = nextion_draw_fill_screen(handle, RGB565_COLOR_GREEN);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Fill_area)
{
    nex_err_t code = nextion_draw_fill_area(handle, {0, 0, 100, 100}, RGB565_COLOR_GREEN);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Fill_circle)
{
    nex_err_t code = nextion_draw_fill_circle(handle, {100, 100}, 20, RGB565_COLOR_GREEN);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Draw_line)
{
    nex_err_t code = nextion_draw_line(handle, {30, 30, 150, 100}, RGB565_COLOR_GREEN);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Draw_circle)
{
    nex_err_t code = nextion_draw_circle(handle, {100, 100}, 20, RGB565_COLOR_GREEN);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Draw_rectangle)
{
    nex_err_t code = nextion_draw_rectangle(handle, {20, 20, 100, 100}, RGB565_COLOR_GREEN);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Draw_picture)
{
    nex_err_t code = nextion_draw_picture(handle, 0, {100, 100});

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Cannot_draw_invalid_picture)
{
    nex_err_t code = nextion_draw_picture(handle, 50, {100, 100});

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_PICTURE, code);
}

TEST(NEX_OP_DRAW, Draw_cropped_picture)
{
    nex_err_t code = nextion_draw_crop_picture(handle, 0, {0, 0, 100, 100}, {0, 0});

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Cannot_draw_invalid_cropped_picture)
{
    nex_err_t code = nextion_draw_crop_picture(handle, 50, {0, 0, 100, 100}, {0, 0});

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_PICTURE, code);
}

TEST(NEX_OP_DRAW, Draw_text)
{
    nex_err_t code = nextion_draw_text(handle,
                                       {0, 0, 100, 100},
                                       {0, RGB565_COLOR_RED},
                                       {BACKG_FILL_COLOR, 0, RGB565_COLOR_BLACK},
                                       {HORZ_ALIGN_CENTER, VERT_ALIGN_CENTER},
                                       "text",
                                       4);

    CHECK_NEX_OK(code);
}

TEST(NEX_OP_DRAW, Cannot_draw_text_with_invalid_font)
{
    nex_err_t code = nextion_draw_text(handle,
                                       {0, 0, 100, 100},
                                       {5, RGB565_COLOR_RED},
                                       {BACKG_FILL_COLOR, 0, RGB565_COLOR_BLACK},
                                       {HORZ_ALIGN_CENTER, VERT_ALIGN_CENTER},
                                       "text",
                                       4);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_FONT, code);
}

TEST(NEX_OP_DRAW, Cannot_draw_text_with_invalid_background_picture)
{
    nex_err_t code = nextion_draw_text(handle,
                                       {0, 0, 100, 100},
                                       {0, RGB565_COLOR_RED},
                                       {BACKG_FILL_IMAGE, 50, RGB565_COLOR_BLACK},
                                       {HORZ_ALIGN_CENTER, VERT_ALIGN_CENTER},
                                       "text",
                                       4);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_PICTURE, code);
}

TEST(NEX_OP_DRAW, Cannot_draw_text_with_invalid_background_crop_picture)
{
    nex_err_t code = nextion_draw_text(handle,
                                       {0, 0, 100, 100},
                                       {0, RGB565_COLOR_RED},
                                       {BACKG_FILL_CROP_IMAGE, 50, RGB565_COLOR_BLACK},
                                       {HORZ_ALIGN_CENTER, VERT_ALIGN_CENTER},
                                       "text",
                                       4);

    NEX_CODES_EQUAL(NEX_DVC_ERR_INVALID_PICTURE, code);
}
