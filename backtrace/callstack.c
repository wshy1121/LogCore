#include <time.h>
#include "callstack.h"
#include "elf_parser.h"
#include "proc_parser.h"
#include "common.h"

extern char g_proc_name[256];
extern void u_printf(const char *fmt, ...);

// ʹ��execinfo.h�ķ�ʽ, N6�²�֧��
void callstack_trace_default()
{
#if 0
#include <execinfo.h>

	void *bt[100];
	char **strings;
	int i;
	int sz = backtrace(bt, 20);
	strings = backtrace_symbols(bt, sz);

	if (!strings)
		return;

	u_printf("===================== callstack trace ======================\n");

	for (i = 0; i < sz; ++i)
	{
		u_printf("%s\n", strings[i]);
	}
#endif
}

static char *strip_path(char *pathname)
{
	int len = strlen(pathname);
	int i;
	for (i = len - 1; i >= 0; i--)
		if (pathname[i] == '/')
			break;

	return pathname + i + 1;
}

static int is_inside(char *pathname)
{
	if (0 == strcmp(strip_path(pathname), strip_path(g_proc_name)))
		return 1;

	return 0;
}

/*
 * �����main������ʱ�򷵻�0
 */
static int printf_symbl_name(int pid, unsigned int pc, int inside)
{
	char funcname[64] = "";
	libmap_t *lm = get_libmap_by_pc(pc);
	unsigned int func_addr;
	int ret = 0;

	if (!lm)
		return -1;

	if (!is_inside(lm->pathname))
		return 1;
	
	ret = get_symbol_by_pid(pid, pc, funcname, &func_addr);
	if (ret != 0)
		func_addr = pc;

	u_printf("- 0x%08x <%s>[+0x%x] (%s)\n", pc, funcname, pc - func_addr, lm->pathname);

	if (0 == strcmp("main", funcname))
		return 0;
	if (0 == strcmp("clone", funcname))
		return 0;

	return 1;
}

/*
	x86�������ù���
	1�����ú���ʱ��eip�Զ���ջ
	2��ebpѹջ
	3����ebp = esp
*/
static void cst_x86(unsigned int pc, unsigned int sp, unsigned int bp)
{
#if ARCH == PLATFORM_X86
	int pid = getpid();
	unsigned int sp_end = get_proc_sp_end(pid);
	unsigned int current_bp = bp;
	unsigned int current_pc;
	int ret;
	int depth = 20;
	
	while(1)
	{
		current_pc = *((unsigned int*)current_bp + 1);
		current_bp = *((unsigned int*)current_bp);

		// ��ӡ
		ret = printf_symbl_name(pid, current_pc, 0);
		if (ret == 0)
			break;
		if (0 == --depth)
			break;

		// ����ջ��Χ���˳�
		if (current_bp < sp || current_bp >= sp_end)
			break;
	}
#endif
}

/*
	��׼arm�������ù���
	1�����浱ǰsp����ip = sp
	2��pc��lr ��ip ��fp ������ջ
	3��fp = ip - 4 (old_sp - 4)
	��ʱջ�ṹ:
			<- old_sp
	pc		<- now_fp (old_sp - 4)
	lr		<- �������ص�ַ��Ҳ���ǵ����ߵ�pc
	ip(old_sp)
	old_fp	<- now_sp
*/
static void cst_arm_standard(unsigned int pc, unsigned int sp, unsigned int bp)
{
#if ARCH == PLATFORM_ARM
	int pid = getpid();
	unsigned int sp_end = get_proc_sp_end(pid);
	unsigned int current_bp = bp;
	unsigned int current_pc;
	int ret;
	int depth = 20;
	
	while(1)
	{
		current_pc = *((unsigned int*)current_bp - 1);
		current_bp = *((unsigned int*)current_bp - 3);

		// ��ӡ
		ret = printf_symbl_name(pid, current_pc, 0);
		if (ret == 0)
			break;
		if (0 == --depth)
			break;

		// ����ջ��Χ���˳�
		if (current_bp < sp || current_bp >= sp_end)
			break;
	}
#endif
}


/*
	û�취ʱ�����
*/
static void cst_common(unsigned int pc, unsigned int sp, int hop_step)
{
	int pid = getpid();
	unsigned int sp_end = get_proc_sp_end(pid);
	unsigned int current_sp = sp;
	unsigned int current_pc;
	int ret;
	int depth = 20;
	
	while(1)
	{
		// ����ջ��Χ���˳�
		if (current_sp < sp || current_sp >= sp_end)
			break;

		current_pc = *((unsigned int*)current_sp);

		// ��ӡ
		ret = printf_symbl_name(pid, current_pc, 1);
		if (ret == 0)
			break;
		if (0 == --depth)
			break;

		current_sp += (4 * hop_step);
	}
}

/*
	arm_v5t_le�������ù���
	1��push lr
*/
static void cst_arm_v5t_le(unsigned int pc, unsigned int sp, unsigned int bp)
{
#if ARCH == PLATFORM_ARM_V5T_LE
	cst_common(pc, sp, 1);
#endif
}

/*
	sh4ƽ̨�������ù���
*/
static void cst_sh4(unsigned int pc, unsigned int sp, unsigned int bp)
{
#if ARCH == PLATFORM_SH4
	cst_common(pc, sp, 1);
#endif
}


// ʹ�ý���elf���ű�ķ�ʽ
void callstack_trace(unsigned int pc, unsigned int sp, unsigned int bp)
{
	int pid = getpid();
	int have_symbol = check_have_symbol_by_pid(pid);
	
	time_t Temptime;
	time(&Temptime);

	get_lib_maps(pid);
	
	u_printf("============Time========= callstack trace:%s ======================\n", ctime(&Temptime));

	if (have_symbol)
		u_printf(" (warn: elf has no symbol table)\n");
		
	// ��ӡpcָ�����ں���
	printf_symbl_name(pid, pc, 0);

	cst_x86(pc, sp, bp);
	cst_arm_standard(pc, sp, bp);
	cst_sh4(pc, sp, bp);
	cst_arm_v5t_le(pc, sp, bp);

}

