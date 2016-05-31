#include "initramfs.h"
#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "kmem_cache.h"
#include "ctype.h"
#include "filesystem.h"
#include "list.h"

static inline char* ALIGN_UP(char* x0, uintmax_t a)
{
	uintmax_t x = (uintmax_t)((void*)x0);
	uintptr_t t=(a - 1);
	if(x & t)
		return (char*)((x|t)+1);
	else
		return x0;
}

static inline unsigned long read_num(char* s){
	static const char digits[] = "0123456789abcdef";
	unsigned long n=0;
	for(int i=0;i<8;++i)
		n=n*16+(strchr(digits, tolower(*s++))-digits);
	return n;
}

void initfs(char* start, char* end){
	while(start + sizeof(struct cpio_header) <= end) {
		start = ALIGN_UP(start, 4);
		struct cpio_header *header = (struct cpio_header *)start;
		DBG_ASSERT(!memcmp(header->magic, CPIO_HEADER_MAGIC, sizeof(header->magic)));
		int mode = read_num(header->mode), sz = read_num(header->filesize), nsz = read_num(header->namesize);

		start += sizeof(struct cpio_header);
		char* name = kmem_alloc(nsz + 1);
		memcpy(name, start, nsz);
		name[nsz] = 0;
		DBG_INFO("Read from archive %s", name);
		if(!strcmp(name, END_OF_ARCHIVE))
			break;

		start += nsz;
		start = ALIGN_UP(start, 4);
		char* content = start;
		start += sz;

		if(S_ISREG(mode)){
			int fd = open(name);
			DBG_ASSERT(fd != -1);
			write(fd, content, sz);
			close(fd);
		}else if(S_ISDIR(mode)){
			int res = mkdir(name);
			DBG_ASSERT(res == 0);
		}
		
		kmem_free(name);
	}
}