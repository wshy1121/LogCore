#ifndef __INCLUDE_C_ELF_PARSER_H__
#define __INCLUDE_C_ELF_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

// ����Ƿ���ڷ��ű�0��ʾ����
int check_have_symbol_by_pid(int pid);
// ��ȡaddr��Ӧ��symbol_name
int get_symbol_by_pid(int pid, unsigned int addr, char *symbol_name, unsigned int *func_addr);

#ifdef __cplusplus
}
#endif

#endif //__INCLUDE_C_ELF_PARSER_H__

