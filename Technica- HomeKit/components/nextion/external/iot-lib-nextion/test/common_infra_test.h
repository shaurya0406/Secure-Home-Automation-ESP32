#ifndef __COMMON_INFRA_TEST_H__
#define __COMMON_INFRA_TEST_H__

#include "CppUTest/TestHarness.h"
#include "nextion/nextion.h"

// Checks

#define CHECK_NEX_OK(code) LONGS_EQUAL(NEX_OK, code)
#define CHECK_NEX_FAIL(code) LONGS_EQUAL(NEX_FAIL, code)
#define CHECK_NULL(pointer) CHECK_EQUAL(NULL, pointer)
#define CHECK_NOT_NULL(pointer) CHECK_TRUE(pointer != NULL)

// Equals

#define SIZET_EQUAL(a, b) UNSIGNED_LONGS_EQUAL(a, b)
#define RGB565_EQUAL(a, b) UNSIGNED_LONGS_EQUAL(a, b)
#define NEX_CODES_EQUAL(a, b) LONGS_EQUAL(a, b)
#define NEX_TOUCH_STATES_EQUAL(a, b) LONGS_EQUAL(a, b)

#define TEST_GROUP_NEX_DISPLAY(testGroup)                    \
    TEST_GROUP(testGroup)                                    \
    {                                                        \
        nextion_handle_t handle = TEST_NEXTION_HANDLE;       \
        nextion_comm_vtable_t *vtable = TEST_NEXTION_VTABLE; \
    }

/**
 * @brief Handle used by the tests.
 * @details Created by the test runner.
 */
extern nextion_handle_t TEST_NEXTION_HANDLE;

/**
 * @brief Virtual method table used by the test communication port.
 * @details Created by the test runner.
 */
extern nextion_comm_vtable_t *TEST_NEXTION_VTABLE;

static nex_comm_err_t dummy_comm_init(nextion_comm_t *self);
static nex_comm_err_t dummy_comm_free(nextion_comm_t *self);
static nex_comm_err_t dummy_comm_read(nextion_comm_t *self, uint8_t *byte);
static nex_comm_err_t dummy_comm_write(nextion_comm_t *self, const uint8_t *buffer, size_t buffer_length);

static nextion_comm_vtable_t DUMMY_COMM_VTABLE{dummy_comm_init,
                                               dummy_comm_free,
                                               dummy_comm_read,
                                               dummy_comm_write};
static nextion_comm_t DUMMY_COMM{&DUMMY_COMM_VTABLE};

static nex_comm_err_t dummy_comm_init(nextion_comm_t *self)
{
    return NEX_COMM_OK;
}

static nex_comm_err_t dummy_comm_free(nextion_comm_t *self)
{
    return NEX_COMM_OK;
}

static nex_comm_err_t dummy_comm_read(nextion_comm_t *self, uint8_t *byte)
{
    return NEX_COMM_OK;
}

static nex_comm_err_t dummy_comm_write(nextion_comm_t *self, const uint8_t *buffer, size_t buffer_length)
{
    return NEX_COMM_OK;
}

#endif //__COMMON_INFRA_TEST_H__