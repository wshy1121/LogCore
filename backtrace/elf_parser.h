#ifndef __INCLUDE_C_ELF_PARSER_H__
#define __INCLUDE_C_ELF_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

// 检查是否存在符号表，0表示存在
int check_have_symbol_by_pid(int pid);
// 获取addr对应的symbol_name
int get_symbol_by_pid(int pid, unsigned int addr, char *symbol_name, unsigned int *func_addr);

#ifdef __cplusplus
}
#endif

#endif //__INCLUDE_C_ELF_PARSER_H__

