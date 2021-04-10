#ifndef __NEXTION_TYPES_H__
#define __NEXTION_TYPES_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @typedef nextion_comm_vtable_t
     * @brief Virtual table for communication ports.
     */
    typedef struct nextion_comm_vtable_t nextion_comm_vtable_t;

    /**
     * @typedef nextion_t
     * @brief Nextion context.
     */
    typedef struct nextion_t nextion_t;

    /**
     * @typedef nextion_handle_t
     * @brief Pointer to a Nextion context.
     */
    typedef nextion_t *nextion_handle_t;

    /**
     * @typedef nextion_response_mode_type_t
     * @brief Response mode for "nextion_command_send_raw".
     */
    typedef enum
    {
        NEXTION_RESP_MODE_NORMAL,         /*!< It must get a parseable message and timeout is not tolerated. */
        NEXTION_RESP_MODE_ACCEPT_TIMEOUT, /*!< Like NORMAL mode but timeout means success and no data is returned. */
        NEXTION_RESP_MODE_STRICT_SIZE     /*!< It must fill the response buffer and timeout is not tolerated. */
    } nextion_response_mode_type_t;

    /**
     * @typedef nextion_baud_rate_t
     * @brief Supported baud rates.
     */
    typedef enum
    {
        NEXTION_BAUD_RATE_2400 = 2400U,
        NEXTION_BAUD_RATE_4800 = 4800U,
        NEXTION_BAUD_RATE_9600 = 9600U,
        NEXTION_BAUD_RATE_19200 = 19200U,
        NEXTION_BAUD_RATE_31250 = 31250U,
        NEXTION_BAUD_RATE_38400 = 38400U,
        NEXTION_BAUD_RATE_57600 = 57600U,
        NEXTION_BAUD_RATE_115200 = 115200U,
        NEXTION_BAUD_RATE_230400 = 230400U,
        NEXTION_BAUD_RATE_250000 = 250000U,
        NEXTION_BAUD_RATE_256000 = 256000U,
        NEXTION_BAUD_RATE_512000 = 512000U,
        NEXTION_BAUD_RATE_921600 = 921600U
    } nextion_baud_rate_t;

    /**
     * @typedef nextion_comm_t
     * @brief Base structure for a communication port.
     */
    typedef struct
    {
        const nextion_comm_vtable_t *vtable;
    } nextion_comm_t;

    struct nextion_comm_vtable_t
    {
        /**
         * @brief Initialize a communication port.
         * @param self Pointer to a nextion_comm_t.
         * @return Any NEX_COMM_* value.
         */
        nex_comm_err_t (*init)(nextion_comm_t *self);

        /**
         * @brief Free a communication port.
         * @param self Pointer to a nextion_comm_t.
         * @return Any NEX_COMM_* value.
         */
        nex_comm_err_t (*free)(nextion_comm_t *self);

        /**
         * @brief Reads data from a communication port.
         *
         * @param self Pointer to a nextion_comm_t.
         * @param byte Pointer to a byte location where the read value will be stored.
         * @return Any NEX_COMM_* value.
         */
        nex_comm_err_t (*read)(nextion_comm_t *self, uint8_t *byte);

        /**
         * @brief Writes data to a communication port.
         *
         * @param self Pointer to a nextion_comm_t.
         * @param buffer Buffer with the data to be written.
         * @param buffer_length Bytes count.
         *
         * @return Any NEX_COMM_* value.
         */
        nex_comm_err_t (*write)(nextion_comm_t *self, const uint8_t *buffer, size_t buffer_length);
    };

#ifdef __cplusplus
}
#endif

#endif //__NEXTION_TYPES_H__