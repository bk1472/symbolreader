#ifndef __WIN32_EXT_H__
#define __WIN32_EXT_H__

#ifdef __cplusplus
extern "C" {
#endif
extern void *mingw_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
#define mmap mingw_mmap

extern int mingw_munmap(void *start, size_t length);
#define munmap mingw_munmap

#define MAP_FAILED 	((void*)-1)
#define MAP_SHARED	0x01
#define MAP_PRIVATE	0x02

#define PROT_READ	0x01
#define PROT_WRITE	0x02
#define PROT_EXEC	0x04
#define PROT_SEM	0x08
#define PROT_NONE	0x00
#ifdef __cplusplus
}
#endif

#endif/*__WIN32_EXT_H__*/
