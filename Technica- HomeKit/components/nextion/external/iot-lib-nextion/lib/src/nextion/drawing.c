#include <malloc.h>
#include "common_infra.h"
#include "nextion/nextion.h"
#include "nextion/drawing.h"

#ifdef __cplusplus
extern "C"
{
#endif

    nex_err_t nextion_draw_fill_screen(nextion_handle_t handle, rgb565_t color)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 4 + INT16_CHAR_LENGTH, "cls %d", color);
    }

    nex_err_t nextion_draw_fill_area(nextion_handle_t handle,
                                     area_t area,
                                     rgb565_t color)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                9 + (5 * INT16_CHAR_LENGTH),
                                "fill %d,%d,%d,%d,%d",
                                area.upper_left.x,
                                area.upper_left.y,
                                area.bottom_right.x - area.upper_left.x,
                                area.bottom_right.y - area.upper_left.y,
                                color);
    }

    nex_err_t nextion_draw_fill_circle(nextion_handle_t handle,
                                       point_t center,
                                       uint16_t radius,
                                       rgb565_t color)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                8 + (4 * INT16_CHAR_LENGTH),
                                "cirs %d,%d,%d,%d",
                                center.x, center.y, radius, color);
    }

    nex_err_t nextion_draw_line(nextion_handle_t handle,
                                area_t area,
                                rgb565_t color)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                9 + (5 * INT16_CHAR_LENGTH),
                                "line %d,%d,%d,%d,%d",
                                area.upper_left.x,
                                area.upper_left.y,
                                area.bottom_right.x,
                                area.bottom_right.y,
                                color);
    }

    nex_err_t nextion_draw_rectangle(nextion_handle_t handle,
                                     area_t area,
                                     rgb565_t color)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                9 + (5 * INT16_CHAR_LENGTH),
                                "draw %d,%d,%d,%d,%d",
                                area.upper_left.x,
                                area.upper_left.y,
                                area.bottom_right.x,
                                area.bottom_right.y,
                                color);
    }

    nex_err_t nextion_draw_circle(nextion_handle_t handle,
                                  point_t center,
                                  uint16_t radius,
                                  rgb565_t color)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                7 + (4 * INT16_CHAR_LENGTH),
                                "cir %d,%d,%d,%d",
                                center.x, center.y, radius, color);
    }

    nex_err_t nextion_draw_picture(nextion_handle_t handle,
                                   uint8_t picture_id,
                                   point_t origin)

    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                6 + INT8_CHAR_LENGTH + (2 * INT16_CHAR_LENGTH),
                                "pic %d,%d,%d",
                                origin.x, origin.y, picture_id);
    }

    nex_err_t nextion_draw_crop_picture(nextion_handle_t handle,
                                        uint8_t picture_id,
                                        area_t crop_area,
                                        point_t destination)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle,
                                11 + INT8_CHAR_LENGTH + (6 * INT16_CHAR_LENGTH),
                                "xpic %d,%d,%d,%d,%d,%d,%d",
                                crop_area.upper_left.x,
                                crop_area.upper_left.y,
                                crop_area.bottom_right.x - crop_area.upper_left.x,
                                crop_area.bottom_right.y - crop_area.upper_left.y,
                                destination.x,
                                destination.y,
                                picture_id);
    }

    nex_err_t nextion_draw_text(nextion_handle_t handle,
                                area_t area,
                                font_t font,
                                background_t background,
                                text_alignment_t alignment,
                                const char *text,
                                size_t text_length)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((text != NULL), "text error(NULL)", NEX_FAIL);

        // It's not a problem having a background value if the fill mode is "none".
        uint16_t background_value = background.picture_id;

        if (background.fill_mode == BACKG_FILL_COLOR)
        {
            background_value = background.color;
        }

        size_t command_length = 17 + (6 * INT16_CHAR_LENGTH) + (2 * INT8_CHAR_LENGTH) + 3 + text_length;
        char *command = (char *)malloc(command_length);

        command_length = snprintf(command,
                                  command_length,
                                  "xstr %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\"",
                                  area.upper_left.x,
                                  area.upper_left.y,
                                  area.bottom_right.x - area.upper_left.x,
                                  area.bottom_right.y - area.upper_left.y,
                                  font.id,
                                  font.color,
                                  background_value,
                                  alignment.horizontal,
                                  alignment.vertical,
                                  background.fill_mode,
                                  text);

        const nex_err_t response = nextion_command_send(handle, command, command_length);

        free(command);

        return response;
    }

#ifdef __cplusplus
}
#endif