#include "proc_parser.h"
#include "common.h"

int get_proc_name(int pid, char *name, int len)
{
	char proc[20];
	sprintf(proc, "/proc/%d/cmdline", pid);
	FILE *fp = fopen(proc, "r");
	int ret = 0;

	if (!fp)
		return 0;

	ret = fread(name, 1, len, fp);

	fclose(fp);

	return ret;
}

int get_thread_name(int tid, char *name, int len)
{
	char proc[20];
	sprintf(proc, "/proc/%d/stat", gettid());
	FILE *fp = fopen(proc, "r");
	int ret = 0;
	char buf[256];

	if (!fp)
		return 0;

	ret = fread(buf, 1, sizeof(buf), fp);

	fclose(fp);

	/* 线程名 */
	strncpy(name, strchr(buf, '(') + 1, strcspn(strchr(buf, '(') + 1, ")"));
	
	return strlen(name);
}

/*
 * 在字符串中解析第 num_alterspace 个空格开始的子字符串
 */
static void parse_str_alterspace(const char *buf, int num_alterspace, char *str)
{
	char *pcur = (char*)buf;

	while(num_alterspace--){
		/* patch: thread name contain space char */
		if('(' == pcur[0]){
			pcur = strchr(pcur, ')');
		}

		pcur = strchr(pcur, ' ');
		pcur++;
	}

	while(NULL != pcur && ' ' != *pcur && 0x00 != *pcur)
		*str++ = *pcur++;

	*str = 0x00;
}

unsigned int get_proc_sp_end(int pid)
{
	char proc[20];
	sprintf(proc, "/proc/%d/stat", pid);
	FILE *fp = fopen(proc, "r");
	char buf[256];
	int read = 0;
	char temp[32];
	unsigned int sp_end = 0;

	if (!fp)
		return 0;

	read = fread(buf, 1, sizeof(buf), fp);
	if (read <= 0)
		return 0;
	
	/* 堆栈顶 */
	parse_str_alterspace(buf, 27, temp);
	sp_end = (unsigned int)strtoul(temp, NULL, 10);

	return sp_end;
}

// 获取段映射表
static libmap_t *g_libmap_head = NULL;
static void insert_libmap(libmap_t * lm)
{
	libmap_t *newlib;

	newlib = (libmap_t*)malloc(sizeof(libmap_t));
	if (NULL == newlib)
		return;

	memcpy(newlib, lm, sizeof(libmap_t));
	newlib->pnext = NULL;
	
	if (NULL == g_libmap_head) {
		g_libmap_head = newlib;
	} else {
		libmap_t *p = g_libmap_head;
		while(p->pnext)
			p = p->pnext;
		p->pnext = newlib;
	}
}

libmap_t *get_lib_maps(int pid)
{
	char filename[32];
	int read = 0;
	char *line = NULL;
	size_t len = 0;
	FILE *fp;
	char temp[32];
	libmap_t lm;
	char startaddress[16];		/* 一个maps项 */
	char endaddress[16];
	char perms[16];
	char offset[16];
	char dev[16];
	char inode[16];
	char pathname[64];
	char *p;

	/* 段映射表 */
	sprintf(filename, "/proc/%d/maps", pid);
	if (NULL == (fp = fopen(filename, "r")))
		return NULL;
	
	while ((read = getline(&line, &len, fp)) != -1)
	{
		sscanf(line, "%s %s %s %s %s %s\n", temp, perms, 
				offset, dev, inode, pathname);
		p = strchr(temp, '-');
		*p++ = 0x00;
		strcpy(startaddress, temp);
		strcpy(endaddress, p);

		strcpy(lm.startaddress, startaddress);
		strcpy(lm.endaddress, endaddress);
		strcpy(lm.perms, perms);
		strcpy(lm.offset, offset);
		strcpy(lm.dev, dev);
		strcpy(lm.inode, inode);
		strcpy(lm.pathname, pathname);

		insert_libmap(&lm);
	}
	
	if(line)
		free(line);
	
	fclose(fp);

	// print
	if (0)
	{
		libmap_t *start = g_libmap_head;
		while (start)
		{
			printf("[%s] - [%s] <%s>\n", start->startaddress, start->endaddress, start->pathname);
			start = start->pnext;
		}
	}

	return g_libmap_head;
}

// 根据pc获取段映射
libmap_t *get_libmap_by_pc(unsigned int pc)
{
	libmap_t *lm = g_libmap_head;

	if (pc > (unsigned int)0xc0000000)
		return NULL;

	while (lm)
	{
		unsigned int start = (unsigned int)strtoul(lm->startaddress, NULL, 16);
		unsigned int end = (unsigned int)strtoul(lm->endaddress, NULL, 16);

//		printf("pc=%08x, [%08x] - [%08x]\n", pc, start, end);

		if (start <= pc && pc < end)
			return lm;
		
		lm = lm->pnext;
	}

	return NULL;
}

