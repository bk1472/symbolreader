#include	"types.h"
#include	"win32_ext.h"
#include	<windows.h>

extern size_t	getpagesize(void);
extern long		_get_osfhandle(int fd);

void *mingw_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	HANDLE handle;

	if (offset % getpagesize() != 0)
	{
		printf("Offset does not match the memory allocation granularity\n");
		exit(1);
	}

	handle = CreateFileMapping((HANDLE)_get_osfhandle(fd), NULL, PAGE_WRITECOPY, 0, 0, NULL);

	if (handle != NULL)
	{
		start = MapViewOfFile(handle, FILE_MAP_COPY, 0, offset, length);
		CloseHandle(handle);
	}

	return start;
}

int mingw_munmap(void *start, size_t length)
{
	UnmapViewOfFile(start);
	return 0;
}
