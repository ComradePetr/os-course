#include "elf.h"
#include "stdio.h"
#include "paging.h"
#include "string.h"
#include "memory.h"
#include "threads.h"
#include "kmem_cache.h"
#include "thread_regs.h"

void handle_pheader(char* file, struct elf_phdr pheader, pte_t* pml4, phys_t pml4_paddr){
	int pages = (pheader.p_memsz + PAGE_SIZE - 1) / PAGE_SIZE;
	uint64_t start = pheader.p_vaddr;
	start-=start%PAGE_SIZE;
	uint64_t end = start + pages * PAGE_SIZE;
	int rc = pt_populate_range(pml4, start, end);
	DBG_ASSERT(!rc);

	struct pt_iter iter;
	for_each_slot_in_range(pml4, start, end, iter){
		int level = iter.level;
		DBG_ASSERT(!level);
		iter.pt[level][iter.idx[level]] = page_paddr(alloc_pages(0)) | PTE_PT_FLAGS | PTE_PRESENT;
		store_pml4(pml4_paddr);
	}

	memcpy((void*)pheader.p_vaddr, file + pheader.p_offset, pheader.p_filesz);
}

int run_elf(void* file0){
	char* file = file0;

	struct elf_hdr header;
	memcpy(&header, file, sizeof(header));

	DBG_ASSERT(!memcmp(header.e_ident,"\x7f""ELF",4));
	DBG_ASSERT(header.e_ident[4]==ELF_CLASS64 && header.e_ident[5]==ELF_DATA2LSB);
	DBG_ASSERT(header.e_type==ELF_EXEC);
	DBG_ASSERT(sizeof(struct elf_phdr) == header.e_phentsize);

	phys_t pml4_paddr = load_pml4();
	pte_t *pml4 = va(pml4_paddr);

	char* pos = file + sizeof(header);
	for(int i=0;i<header.e_phnum;++i,pos+=header.e_phentsize){
		struct elf_phdr pheader;
		memcpy(&pheader, pos, sizeof(pheader));
		if(pheader.p_type == PT_LOAD)
			handle_pheader(file, pheader, pml4, pml4_paddr);
	}

	struct page* stack = alloc_pages(0);
	DBG_ASSERT(stack);

	struct pt_iter iter;
	pt_iter_set(&iter, pml4, 0);
	int level = iter.level;
	DBG_ASSERT(!level);
	iter.pt[level][iter.idx[level]] = page_paddr(stack) | PTE_PT_FLAGS | PTE_PRESENT;
	store_pml4(pml4_paddr);

	struct thread_regs* regs = thread_regs(current());
	regs->ss = (uint64_t)USER_DATA;
	regs->cs = (uint64_t)USER_CODE;
	regs->rsp = (uint64_t)((char*)iter.addr + PAGE_SIZE);
	regs->rip = (uint64_t)header.e_entry;

	puts("run_elf just prepared memory and is going to run user application");
	return 0;
}

pid_t execute(char *file){
	return create_kthread(&run_elf, file);
}
