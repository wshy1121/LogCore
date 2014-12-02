#ifndef __INCLUDE_C_PROC_PARSER_H__
#define __INCLUDE_C_PROC_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 库映射地址空间，及代码/数据/堆栈段地址空间 */
typedef struct libinfo{
	char startaddress[16];		/* 起始虚拟地址 */
	char endaddress[16];		/* 结束虚拟地址 */
	char perms[16];			/* 权限 */
	char offset[16];		/* 偏移量 */
	char dev[16];
	char inode[16];
	char pathname[64];
	struct libinfo *pnext;
} libmap_t;

// 获取进程名
int get_proc_name(int pid, char *name, int len);
// 获取线程名
int get_thread_name(int tid, char *name, int len);
// 获取段结束地址
unsigned int get_proc_sp_end(int pid);
// 获取段映射表
libmap_t *get_lib_maps(int pid);
// 根据pc获取段映射
libmap_t *get_libmap_by_pc(unsigned int pc);

#ifdef __cplusplus
}
#endif

#endif //__INCLUDE_C_PROC_PARSER_H__

