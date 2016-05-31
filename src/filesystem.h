#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "list.h"
#include "stdint.h"

enum fsnode_type {FILE, FOLDER};
typedef struct{
	char* name;
	size_t size;
	enum fsnode_type type;
	struct list_head link;
	struct list_head content;
} fsnode;

int mkdir(char *path);
int open(char *path);
fsnode* readdir(char *path);
int close(int fildes);
int64_t read(int fd, char *buf, size_t count);
int64_t write(int fd, const char *buf, size_t count);
size_t size(int fd);

#endif /*__FILESYSTEM_H__*/
