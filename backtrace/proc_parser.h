#ifndef __INCLUDE_C_PROC_PARSER_H__
#define __INCLUDE_C_PROC_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ��ӳ���ַ�ռ䣬������/����/��ջ�ε�ַ�ռ� */
typedef struct libinfo{
	char startaddress[16];		/* ��ʼ�����ַ */
	char endaddress[16];		/* ���������ַ */
	char perms[16];			/* Ȩ�� */
	char offset[16];		/* ƫ���� */
	char dev[16];
	char inode[16];
	char pathname[64];
	struct libinfo *pnext;
} libmap_t;

// ��ȡ������
int get_proc_name(int pid, char *name, int len);
// ��ȡ�߳���
int get_thread_name(int tid, char *name, int len);
// ��ȡ�ν�����ַ
unsigned int get_proc_sp_end(int pid);
// ��ȡ��ӳ���
libmap_t *get_lib_maps(int pid);
// ����pc��ȡ��ӳ��
libmap_t *get_libmap_by_pc(unsigned int pc);

#ifdef __cplusplus
}
#endif

#endif //__INCLUDE_C_PROC_PARSER_H__

