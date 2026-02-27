#include "vfs.h"

#include <core/arch/i686/VGATextDevice.h>
#include <core/arch/i686/e9.h>
#include <IO.h>

int VFS_Write(fd_t file, uint8_t *data, size_t size)
{
    switch (file)
    {
    case VFS_FD_STDIN:
        return 0;

    case VFS_FD_STDOUT:
    case VFS_FD_STDERR:
        for (size_t i = 0; i < size; i++)
        {
            putChar(data[i]);
        }
        return size;
    case VFS_FD_DEBUG:
        for (size_t i = 0; i < size; i++)
        {
            e9_putc(data[i]);
        }
        return size;

    default:
        return -1;
    }
}

int VFS_Read(fd_t file, uint8_t *data, size_t size)
{
    return 0;
}