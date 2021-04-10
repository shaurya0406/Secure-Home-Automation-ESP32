#include "common_infra_test.h"
#include "nextion/nextion.h"

uint8_t *fake_buffer;
uint8_t fake_buffer_counter;

uint8_t touch_page_id;
uint8_t touch_component_id;
nextion_touch_state_t touch_state;

uint16_t touch_coord_x;
uint16_t touch_coord_y;
nextion_touch_state_t touch_coord_state;
bool touch_coord_exited_sleep;

nextion_device_state_t device_state;

static void _test_event_callback_on_touch(nextion_handle_t handle,
                                          uint8_t page_id,
                                          uint8_t component_id,
                                          nextion_touch_state_t state)
{
    touch_page_id = page_id;
    touch_component_id = component_id;
    touch_state = state;
}

static void _test_event_callback_on_touch_coord(nextion_handle_t handle,
                                                uint16_t x,
                                                uint16_t y,
                                                nextion_touch_state_t state,
                                                bool exited_sleep)
{
    touch_coord_x = x;
    touch_coord_y = y;
    touch_coord_state = state;
    touch_coord_exited_sleep = exited_sleep;
}

static void _test_event_callback_on_device(nextion_handle_t handle, nextion_device_state_t state)
{
    device_state = state;
}

static nex_comm_err_t _test_event_comm_read(nextion_comm_t *self, uint8_t *byte)
{
    *byte = fake_buffer[fake_buffer_counter++];

    return NEX_COMM_OK;
}

#define DO_CALLBACK()                          \
    void *original_read_method = vtable->read; \
    vtable->read = &_test_event_comm_read;     \
    nextion_event_process(handle);             \
    vtable->read = (nex_comm_err_t(*)(nextion_comm_t *, uint8_t *))original_read_method;

TEST_GROUP_NEX_DISPLAY(NEX_EVENT);

TEST(NEX_EVENT, Dispatch_event_touch)
{
    uint8_t buffer[7] = {NEX_DVC_EVT_TOUCH_OCCURRED, 0x01U, 0x02U, 0x01U, NEX_DVC_CMD_END_VALUE, NEX_DVC_CMD_END_VALUE, NEX_DVC_CMD_END_VALUE};
    fake_buffer = buffer;
    fake_buffer_counter = 0;

    nextion_event_callback_t callback;

    callback.on_touch = &_test_event_callback_on_touch;

    nextion_event_callback_set(handle, callback);

    DO_CALLBACK()

    LONGS_EQUAL(1, touch_page_id);
    LONGS_EQUAL(2, touch_component_id);
    NEX_TOUCH_STATES_EQUAL(NEXTION_TOUCH_PRESSED, touch_state);
}

TEST(NEX_EVENT, Dispatch_event_touch_coord)
{
    uint8_t buffer[9] = {NEX_DVC_EVT_TOUCH_COORDINATE_AWAKE, 0x00U, 0x7AU, 0x00U, 0x1EU, 0x01U, NEX_DVC_CMD_END_VALUE, NEX_DVC_CMD_END_VALUE, NEX_DVC_CMD_END_VALUE};
    fake_buffer = buffer;
    fake_buffer_counter = 0;

    nextion_event_callback_t callback;

    callback.on_touch_coord = &_test_event_callback_on_touch_coord;

    nextion_event_callback_set(handle, callback);

    DO_CALLBACK()

    LONGS_EQUAL(122, touch_coord_x);
    LONGS_EQUAL(30, touch_coord_y);
    CHECK_FALSE(touch_coord_exited_sleep);
    NEX_TOUCH_STATES_EQUAL(NEXTION_TOUCH_PRESSED, touch_coord_state);
}

TEST(NEX_EVENT, Dispatch_event_device)
{
    uint8_t buffer[4] = {NEX_DVC_EVT_HARDWARE_READY, NEX_DVC_CMD_END_VALUE, NEX_DVC_CMD_END_VALUE, NEX_DVC_CMD_END_VALUE};
    fake_buffer = buffer;
    fake_buffer_counter = 0;

    nextion_event_callback_t callback;

    callback.on_device = &_test_event_callback_on_device;

    nextion_event_callback_set(handle, callback);

    DO_CALLBACK()

    NEX_CODES_EQUAL(NEX_DVC_EVT_HARDWARE_READY, device_state);
}
