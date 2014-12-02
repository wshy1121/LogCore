#include <elf.h>

#include "elf_parser.h"
#include "common.h"

static int get_link_name_by_pid(int pid, char *linkname, int name_size)
{
	char filename[32] = "";
	char *p = NULL;

	sprintf(filename, "/proc/%d/exe", pid);
	if (0 != access(filename, R_OK))
		return -1;

	if (-1 == readlink(filename, linkname, name_size))
		return -2;

	p = strstr(linkname, " (deleted)");
	if (NULL != p)
		*p = 0x00;

	return 0;
}

static int check_have_symbol(const char *filename)
{
	FILE *file;
	Elf32_Ehdr elfhead;
	Elf32_Shdr elfsection;
	int section_num;
	int section_size;
	int i;
	
	int ret = -3;

	/*
	dprintf("检测elf文件是否含有符号表。\n");
	*/
	if(NULL == (file = fopen(filename, "rb"))){
		ret = -4;
		goto out;
	}
	
	if(1 != fread(&elfhead, sizeof(Elf32_Ehdr), 1, file)){
		ret = -5;
		goto out;
	}
	section_num = elfhead.e_shnum;
	section_size = elfhead.e_shentsize;
	if(0 != fseek(file, elfhead.e_shoff, SEEK_SET)){
		ret = -6;
		goto out;
	}
	
	for(i = 0; i < section_num; i++){
		if(1 != fread(&elfsection, sizeof(Elf32_Shdr), 1, file)){
			ret = -7;
			goto out;
		}
		if(SHT_SYMTAB == elfsection.sh_type){
			ret = 0;
			goto out;
		}
	}

out:
	if(NULL != file)
		fclose(file);
	return ret;
}

int check_have_symbol_by_pid(int pid)
{
	int ret = 0;
	char linkname[64] = "";

	ret = get_link_name_by_pid(pid, linkname, sizeof(linkname));
	if (ret != 0)
		return ret;

	return check_have_symbol(linkname);
}

static int elf_read(const char *filename, int offset, int size, void *buf)
{
	FILE *fp;

	if (NULL == (fp = fopen(filename, "r")))
		return -1;

	fseek(fp, offset, SEEK_SET);
	size = fread(buf, 1, size, fp);
	fclose(fp);

	return size;
}


static int get_symbol(const char *elf_file, unsigned int addr, char *symbol_name, unsigned int *func_addr)
{
	Elf32_Ehdr elf_header;			/* elf头 */
	Elf32_Shdr *section_table = NULL;	/* elf节区表 */
	int section_size;					/* elf节区表大小 */
	int i, j;

	Elf32_Shdr *pshdr_sym;		/* 符号表 */
	Elf32_Shdr *pshdr_str;		/* 字符串表 */
	void *psymboltable = NULL;	/* 符号表内容 */
	void *pstringtable = NULL;		/* 字符串表内容 */
	int symsize;					/* 符号表大小 */
	int strsize;					/* 字符串表大小 */
	Elf32_Sym *psym;			/* 一个符号表项 */

	int found = 0;

	// read elf header
	if (sizeof(Elf32_Ehdr) != elf_read(elf_file, 0, sizeof(Elf32_Ehdr), &elf_header))
		return -1;

	// count section table size and malloc section_table
	section_size = elf_header.e_shentsize * elf_header.e_shnum;
	if (NULL == (section_table = (Elf32_Shdr*)malloc(section_size)))
		return -1;

	// read section table
	if (elf_read(elf_file, elf_header.e_shoff, section_size, section_table) != section_size)
		return -1;

	/* 遍历所有节表, find synbol table */
	pshdr_sym = section_table;
	for (i = 0; i < elf_header.e_shnum; i++, pshdr_sym++)
	{
		if (SHT_SYMTAB == pshdr_sym->sh_type)
		{
			/* 读取符号表内容 */
			psymboltable = (void*)malloc(pshdr_sym->sh_size);
			if (NULL == psymboltable)
				break;

			symsize = elf_read(elf_file, pshdr_sym->sh_offset, pshdr_sym->sh_size, psymboltable);
			if (symsize != pshdr_sym->sh_size)
				break;

			/* 取得符号表对应的字符串表 */
			pshdr_str = section_table + pshdr_sym->sh_link;
			pstringtable = (void*)malloc(pshdr_str->sh_size);
			if (NULL == pstringtable)
				break;

			strsize = elf_read(elf_file, pshdr_str->sh_offset, pshdr_str->sh_size, pstringtable);
			if (strsize != pshdr_str->sh_size)
				break;

			/* 遍历符号表 */
			psym = (Elf32_Sym*)psymboltable;	/* 第一个符号项 */
			for (j = 0; j < symsize / sizeof(Elf32_Sym) ; j++, psym++)
			{
				if (0 == psym->st_shndx || STT_FUNC != ELF32_ST_TYPE(psym->st_info))
					continue;
				if (!psym->st_value)
					continue;

				if (psym->st_value <= addr && addr < (psym->st_value + psym->st_size))
				{
					if (func_addr)
						*func_addr = psym->st_value;
					strcpy(symbol_name, pstringtable + psym->st_name);
					found = 1;
					break;
				}
			}

			if (psymboltable)
				free(psymboltable), psymboltable = NULL;

			if (pstringtable)
				free(pstringtable), pstringtable = NULL;

			if (found)
				break;
		}
	}

	if (section_table)
		free(section_table);

	if (psymboltable)
		free(psymboltable);
		
	if (pstringtable)
		free(pstringtable);

	if (found == 0)
		return -100;

	return 0;
}

int get_symbol_by_pid(int pid, unsigned int addr, char *symbol_name, unsigned int *func_addr)
{
	int ret = 0;
	char linkname[64] = "";

	ret = get_link_name_by_pid(pid, linkname, sizeof(linkname));
	if (ret != 0)
		return ret;

	return get_symbol(linkname, addr, symbol_name, func_addr);
}

