#include "common_infra_test.h"
#include "rgb565/rgb565.h"

TEST_GROUP(RGB565){};

// Just need to test the primary colors conversion.

TEST(RGB565, Convert_from_888_red)
{
    RGB565_EQUAL(RGB565_COLOR_RED, rgb565_convert_from_888(255, 0, 0));
}

TEST(RGB565, Convert_from_888_green)
{
    RGB565_EQUAL(RGB565_COLOR_GREEN, rgb565_convert_from_888(0, 255, 0));
}

TEST(RGB565, Convert_from_888_blue)
{
    RGB565_EQUAL(RGB565_COLOR_BLUE, rgb565_convert_from_888(0, 0, 255));
}
