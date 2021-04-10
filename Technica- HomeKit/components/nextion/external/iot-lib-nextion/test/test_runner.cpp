#include "Windows.h"
#include "CppUTest/CommandLineTestRunner.h"
#include "nextion/nextion.h"
#include "common_infra_test.h"

nex_comm_err_t test_runner_comm_init(nextion_comm_t *self);
nex_comm_err_t test_runner_comm_free(nextion_comm_t *self);
nex_comm_err_t test_runner_comm_read(nextion_comm_t *self, uint8_t *byte);
nex_comm_err_t test_runner_comm_write(nextion_comm_t *self, const uint8_t *buffer, size_t buffer_length);

nextion_handle_t TEST_NEXTION_HANDLE;
nextion_comm_vtable_t *TEST_NEXTION_VTABLE;

typedef struct
{
    nextion_comm_t base;
    LPCSTR comm_port_name;
    HANDLE comm_port_handle;
} nextion_windows_comm_t;

static nextion_comm_vtable_t vtable = {
    test_runner_comm_init,
    test_runner_comm_free,
    test_runner_comm_read,
    test_runner_comm_write};

int main(int ac, char **av)
{
    nextion_windows_comm_t *comm = (nextion_windows_comm_t *)malloc(sizeof(nextion_windows_comm_t));
    comm->base.vtable = &vtable;
    comm->comm_port_name = "COM4";

    nextion_handle_t nextion_handle = nextion_create((nextion_comm_t *)comm);

    if (nextion_handle == NULL)
    {
        printf("Could not create Nextion context.");

        return -1;
    }

    if (nextion_init(nextion_handle) == NEX_FAIL)
    {
        printf("Could not initialize the Nextion display.");

        return -1;
    }

    TEST_NEXTION_HANDLE = nextion_handle;
    TEST_NEXTION_VTABLE = &vtable;

    int test_result = RUN_ALL_TESTS(ac, av);

    nextion_free(nextion_handle);

    return test_result;
}

static nex_comm_err_t test_runner_comm_init(nextion_comm_t *self)
{
    nextion_windows_comm_t *super = (nextion_windows_comm_t *)self;

    HANDLE handle = CreateFile(super->comm_port_name,
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               0,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               0);

    if (handle == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();

        if (error == ERROR_FILE_NOT_FOUND)
        {
            printf("Serial port '%s' not found.", super->comm_port_name);
        }
        else
        {
            printf("Error on opening port: %d", error);
        }

        return NEX_COMM_FAIL;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(handle, &dcbSerialParams))
    {
        printf("Could not get port state.");

        return NEX_COMM_FAIL;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(handle, &dcbSerialParams))
    {
        printf("Could not set port state.");

        return NEX_COMM_FAIL;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(handle, &timeouts))
    {
        printf("Could not set port timeouts.");

        return NEX_COMM_FAIL;
    }

    super->comm_port_handle = handle;

    return NEX_COMM_OK;
}

static nex_comm_err_t test_runner_comm_free(nextion_comm_t *self)
{
    nextion_windows_comm_t *super = (nextion_windows_comm_t *)self;

    CloseHandle(super->comm_port_handle);

    super->comm_port_handle = NULL;

    return NEX_COMM_OK;
}

static nex_comm_err_t test_runner_comm_read(nextion_comm_t *self, uint8_t *byte)
{
    DWORD bytes_read_serial;

    bool result = ReadFile(((nextion_windows_comm_t *)self)->comm_port_handle, byte, 1, &bytes_read_serial, NULL);

    // How ReadFile signals it has timed out?
    // https://stackoverflow.com/questions/38363301/how-do-i-find-out-that-readfile-function-timed-out
    //
    // If a timeout occurs, the read is complete (ReadFile() returns TRUE), and the number
    // of bytes that had been buffered before the timeout elapsed is reflected in your dwRead variable.
    // So, you will know if a timeout occurred if dwRead is less than the number of bytes you
    // asked ReadFile() to read. If there was no buffered data, dwRead will be 0. If dwRead is equal
    // to the number of bytes you asked for, there was no timeout since ReadFile() exited when the last
    // requested byte was read.

    if (result)
    {
        if (bytes_read_serial == 0)
            return NEX_COMM_TIMEOUT;

        return NEX_COMM_OK;
    }

    return NEX_COMM_FAIL;
}

static nex_comm_err_t test_runner_comm_write(nextion_comm_t *self, const uint8_t *buffer, size_t buffer_length)
{
    HANDLE handle = ((nextion_windows_comm_t *)self)->comm_port_handle;

    if (!WriteFile(handle, buffer, (DWORD)buffer_length, NULL, NULL))
        return NEX_COMM_FAIL;

    if (!FlushFileBuffers(handle))
        return NEX_COMM_FAIL;

    return NEX_COMM_OK;
}