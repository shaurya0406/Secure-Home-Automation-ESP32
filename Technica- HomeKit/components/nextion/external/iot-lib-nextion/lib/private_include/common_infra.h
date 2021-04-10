#ifndef __COMMON_INFRA_H__
#define __COMMON_INFRA_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define BOOL_CHAR_LENGTH 1
#define INT8_CHAR_LENGTH 3
#define INT16_CHAR_LENGTH 5
#define INT32_CHAR_LENGTH 10
#define SIZET_CHAR_LENGTH 10

#ifdef DEBUG
/**
 * @brief Logs an error.
 * @param format Text to be logged. Supports formats.
 * @param ... Values used on the format.
 */
#define NEX_LOGE(format, ...) printf(format, ##__VA_ARGS__)

/**
 * @brief Logs a warning.
 * @param format Text to be logged. Supports formats.
 * @param ... Values used on the format.
 */
#define NEX_LOGW(format, ...) printf(format, ##__VA_ARGS__)

/**
 * @brief Logs an information.
 * @param format Text to be logged. Supports formats.
 * @param ... Values used on the format.
 */
#define NEX_LOGI(format, ...) printf(format, ##__VA_ARGS__)

/**
 * @brief Logs for debug.
 * @param format Text to be logged. Supports formats.
 * @param ... Values used on the format.
 */
#define NEX_LOGD(format, ...) printf(format, ##__VA_ARGS__)

/**
 * @brief Logs for verbose.
 * @param format Text to be logged. Supports formats.
 * @param ... Values used on the format.
 */
#define NEX_LOGV(format, ...) printf(format, ##__VA_ARGS__)

/**
 * @brief Macro that forces a method to return a value
 * based on a failed condition.
 *
 * @note When building with DEBUG, the message will be printed.
 *
 * @param condition Condition to evaluate.
 * @param message Message to be written if the condition fails. Only for DEBUG builds.
 * @param return_value Value to be returned if the condition fails.
 *
 * @return If the condition fails the method will return what was passed on
 * "return_value" parameter, otherwise the method will continue.
 */
#define NEX_CHECK(condition, message, return_value)              \
    if (!(condition))                                            \
    {                                                            \
        NEX_LOGE("%s(%d): %s", __FUNCTION__, __LINE__, message); \
        return (return_value);                                   \
    }
#else

#define NEX_LOGE(format, ...)
#define NEX_LOGW(format, ...)
#define NEX_LOGI(format, ...)
#define NEX_LOGD(format, ...)
#define NEX_LOGV(format, ...)

#define NEX_CHECK(condition, message, return_value) \
    if (!(condition))                               \
    {                                               \
        return (return_value);                      \
    }
#endif

/**
 * @brief Checks if a Nextion handle is valid.
 * @param handle Nextion handle.
 * @param return_value Value to be returned if the condition fails.
 * @return If the condition fails the method will return what was passed on
 * "return_value" parameter, otherwise the method will continue.
 */
#define NEX_CHECK_HANDLE(handle, return_value) NEX_CHECK((handle != NULL), "handle error(NULL)", return_value)

/**
 * @brief Sends a command that waits for a simple response (ACK).
 *
 * @details We need it because I wan´t to allocate eveything
 * possible on the stack. That's one of the reasons we have
 * "command_length". Another reason is to avoid "strlen" as
 * it is a O(N) function.
 *
 * @details The parameter "command_length" comes first
 * because it's easier to reason with "command_format" followed
 * by the values ("..." ) used to format it. Otherwise the
 * "command_length" could be mistakenly read as one of the format
 * values.
 *
 * @param result Where the result will be stored.
 * @param handle Nextion handle pointer.
 * @param command_length Length of the command.
 * @param command_format Format where values will be applied.
 * @param ... Values to apply on the format.
 *
 * @return NEX_OK if success, otherwise any NEX_DVC_ERR_* value.
 */
#define NEX_SEND_COMMAND(result, handle, command_length, command_format, ...)                 \
    char buffer[command_length + 1];                                                          \
    size_t real_length = snprintf(buffer, command_length + 1, command_format, ##__VA_ARGS__); \
    result = nextion_command_send(handle, buffer, real_length);

/**
 * @brief Sends a command that waits for a simple response (ACK) and
 * returns a nex_err_t result.
 *
 * @param handle Nextion handle pointer.
 * @param command_length Length of the command.
 * @param command_format Format where values will be applied.
 * @param ... Values to apply on the format.
 *
 * @return NEX_OK if success, otherwise any NEX_DVC_ERR_* value.
 */
#define NEX_RETURN_SEND_COMMAND(handle, command_length, command_format, ...)         \
    nex_err_t result;                                                                \
    NEX_SEND_COMMAND(result, handle, command_length, command_format, ##__VA_ARGS__); \
    return result;

#ifdef __cplusplus
}
#endif

#endif //__COMMON_INFRA_H__