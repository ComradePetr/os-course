#include "filesystem.h"
#include "kmem_cache.h"
#include "stdio.h"
#include "string.h"
#include "locking.h"

static LIST_HEAD(filesystem);
static DEFINE_SPINLOCK(fs_lock);

static char* find_end(char *s){
	while(*s && *s!='/')
		++s;
	return s;
}

static fsnode* go(listh *cur, char* a, char *b){
	const char c = *b;
	*b=0;

	const listh * const end = cur;
	listh *pos = cur->next;

	for (; pos != end; pos = pos->next) {
		fsnode *node = LIST_ENTRY(pos, fsnode, link);
		if(!strcmp(node->name, a)){
			*b=c;
			return node;
		}
	}
	
	*b=c;
	return NULL;
}

int mkdir(char *path){
	const bool enabled = spin_lock_irqsave(&fs_lock);

	listh* cur = &filesystem;
	do{
		char* end = find_end(path);
		fsnode* t = go(cur, path, end);
		if(!t){
			if(*end){
				spin_unlock_irqrestore(&fs_lock, enabled);
				return -1; //path does not exist
			}

			fsnode *new = kmem_alloc(sizeof(fsnode));
			DBG_ASSERT(new);

			new->name = strdup(path);
			new->type = FOLDER;
			new->size = 0;
			list_init(&new->content);
			list_init(&new->link);

			list_add_tail(&new->link, cur);
			
			spin_unlock_irqrestore(&fs_lock, enabled);
			return 0;
		}else{
			if(!*end || t->type!=FOLDER){
				spin_unlock_irqrestore(&fs_lock, enabled);
				return -1;
			}
			cur = &t->content;
			path = end + 1;
		}
	}while(1);
	spin_unlock_irqrestore(&fs_lock, enabled);
	return -1;
}

static const int BLOCK_SIZE = 4096;
typedef struct{
	int pos;
	char *mem;
	listh link;
} block;
static block* add_block(fsnode *file){
	DBG_ASSERT(file->type==FILE);
	char *mem = kmem_alloc(BLOCK_SIZE);
	block *b = kmem_alloc(sizeof(block));
	b->pos = 0;
	b->mem = mem;
	list_init(&b->link);
	list_add_tail(&b->link, &file->content);
	return b;
}

static LIST_HEAD(descriptors);
typedef struct{
	int id;
	fsnode *file;
	listh link;
	block *curblock;
	int pos;
} descriptor;



static int new_descriptor(fsnode *file){
	DBG_ASSERT(file->type == FILE);
	static int max_descriptor_id = 0;
	
	++max_descriptor_id;
	descriptor *new = kmem_alloc(sizeof(descriptor));
	new->id = max_descriptor_id;
	new->file = file;
	DBG_ASSERT(file->content.next != &(file->content));
	new->curblock = LIST_ENTRY(file->content.next, block, link);
	new->pos = 0;
	list_init(&new->link);
	list_add_tail(&new->link, &descriptors);
	
	return new->id;
}

static descriptor* get_desc_by_fd(int fd){
	const listh * const end = &descriptors;
	listh *pos = end->next;

	for (; pos != end; pos = pos->next) {
		descriptor *d = LIST_ENTRY(pos, descriptor, link);
		if(d->id==fd)
			return d;
	}
	return NULL;
}

int close(int fd){
	const bool enabled = spin_lock_irqsave(&fs_lock);
	descriptor *d=get_desc_by_fd(fd);
	if(!d){
		spin_unlock_irqrestore(&fs_lock, enabled);
		return -1;
	}
	list_del(&d->link);
	kmem_free(d);
	spin_unlock_irqrestore(&fs_lock, enabled);
	return 0;
}


int open(char *path){
	const bool enabled = spin_lock_irqsave(&fs_lock);
	listh* cur = &filesystem;
	do{
		char* end = find_end(path);
		fsnode* t = go(cur, path, end);
		if(!t){
			if(*end){
				spin_unlock_irqrestore(&fs_lock, enabled);
				return -1; //path does not exist
			}

			fsnode *new = kmem_alloc(sizeof(fsnode));
			DBG_ASSERT(new);

			new->name = strdup(path);
			new->type = FILE;
			new->size = 0;
			list_init(&new->content);
			list_init(&new->link);
			add_block(new);

			list_add_tail(&new->link, cur);

			int d=new_descriptor(new);
			spin_unlock_irqrestore(&fs_lock, enabled);
			return d;
		}else{
			if(*end){
				if(t->type!=FOLDER){
					spin_unlock_irqrestore(&fs_lock, enabled);
					return -1;
				}
				cur = &t->content;
				path = end + 1;
			}else{
				if(t->type!=FILE){
					spin_unlock_irqrestore(&fs_lock, enabled);
					return -1;
				}
				int d=new_descriptor(t);
				spin_unlock_irqrestore(&fs_lock, enabled);
				return d;
			}
		}
	}while(1);
	spin_unlock_irqrestore(&fs_lock, enabled);
	return -1;
}

static void write_char(descriptor *d, char c){
	block *b = d->curblock;
	fsnode *f = d->file;
	DBG_ASSERT(d->pos<=b->pos);
	if(d->pos==BLOCK_SIZE){
		listh *p=&b->link, *head = &(f->content);
		if(p->next==head)
			add_block(f);
		DBG_ASSERT(p->next!=head);
		d->curblock = b = LIST_ENTRY(p->next, block, link);
		d->pos = 0;
	}

	DBG_ASSERT(d->pos<=b->pos);
	DBG_ASSERT(d->pos<BLOCK_SIZE);
	b->mem[d->pos]=c;
	if(d->pos==b->pos)
		++b->pos, ++f->size;
	++d->pos;
}

int64_t write(int fd, const char *buf, size_t count){
	const bool enabled = spin_lock_irqsave(&fs_lock);
	descriptor *d=get_desc_by_fd(fd);
	if(!d){
		spin_unlock_irqrestore(&fs_lock, enabled);
		return -1;
	}
	int64_t res;
	for(;count;--count)
		write_char(d, *(buf++)), ++res;
	spin_unlock_irqrestore(&fs_lock, enabled);
	return res;
}

static int read_char(descriptor *d){
	block *b = d->curblock;
	DBG_ASSERT(d->pos<=b->pos);
	if(d->pos==b->pos){
		listh *p=&b->link, *head = &(d->file->content);
		if(p->next==head)
			return -1;
		d->curblock = b = LIST_ENTRY(p->next, block, link);
		d->pos = 0;
	}
	if(d->pos==b->pos)
		return -1;

	return b->mem[d->pos++];
}

int64_t read(int fd, char *buf, size_t count){
	const bool enabled = spin_lock_irqsave(&fs_lock);
	descriptor *d=get_desc_by_fd(fd);
	if(!d){
		spin_unlock_irqrestore(&fs_lock, enabled);
		return -1;
	}
	int64_t res;
	for(;count;--count){
		int c = read_char(d);
		if(c==-1)
			break;
		*(buf++) = c, ++res;
	}
	spin_unlock_irqrestore(&fs_lock, enabled);
	return res;
}

fsnode* readdir(char *path){
	const bool enabled = spin_lock_irqsave(&fs_lock);
	listh* cur = &filesystem;
	do{
		char* end = find_end(path);
		fsnode* t = go(cur, path, end);
		if(!t || t->type!=FOLDER){
			spin_unlock_irqrestore(&fs_lock, enabled);
			return NULL;
		}
		cur = &t->content;
		if(!*end)
			break;
		path = end + 1;
	}while(1);

	fsnode *r=LIST_ENTRY(cur, fsnode, content);
	spin_unlock_irqrestore(&fs_lock, enabled);
	return r;
}

size_t size(int fd){
	const bool enabled = spin_lock_irqsave(&fs_lock);
	descriptor *d=get_desc_by_fd(fd);
	if(!d){
		spin_unlock_irqrestore(&fs_lock, enabled);
		return 0;
	}
	size_t res = d->file->size;
	spin_unlock_irqrestore(&fs_lock, enabled);
	return res;
}