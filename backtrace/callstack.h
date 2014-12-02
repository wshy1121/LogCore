#ifndef __INCLUDE_C_CALLSTACK_H__
#define __INCLUDE_C_CALLSTACK_H__

#ifdef __cplusplus
extern "C" {
#endif

void callstack_trace_default();
void callstack_trace(unsigned int pc, unsigned int sp, unsigned int bp);

#ifdef __cplusplus
}
#endif

#endif //__INCLUDE_C_CALLSTACK_H__

