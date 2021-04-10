#include "common_infra.h"
#include "nextion/nextion.h"
#include "nextion/page.h"

#ifdef __cplusplus
extern "C"
{
#endif

    nex_err_t nextion_page_get(nextion_handle_t handle, uint8_t *page_id)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);
        NEX_CHECK((page_id != NULL), "page_id error(NULL)", NEX_FAIL);

        uint8_t response[5];
        size_t bytes_read = 0;

        if (!nextion_command_send_raw(handle,
                                      "sendme",
                                      6,
                                      response,
                                      5,
                                      NEXTION_RESP_MODE_NORMAL,
                                      &bytes_read))
        {
            return NEX_FAIL;
        }

        if (bytes_read == 5 && response[0] == NEX_DVC_RSP_SENDME_RESULT)
        {
            *page_id = response[1];

            return NEX_OK;
        }
        else if (bytes_read == NEX_DVC_CMD_ACK_LENGTH)
        {
            return response[0];
        }
        else
            return NEX_FAIL;
    }

    nex_err_t nextion_page_set(nextion_handle_t handle, const char *page_name_or_id)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 5 + NEX_DVC_PAGE_MAX_NAME_LENGTH, "page %s", page_name_or_id);
    }

    nex_err_t nextion_page_refresh(nextion_handle_t handle)
    {
        NEX_CHECK_HANDLE(handle, NEX_FAIL);

        NEX_RETURN_SEND_COMMAND(handle, 5, "ref 0");
    }

#ifdef __cplusplus
}
#endif