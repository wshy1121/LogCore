#include "DahuaBacktrace.h"
#include "proc_parser.h"
#include "elf_parser.h"
#include "callstack.h"
#include "common.h"

char g_proc_name[256] = {0};
FILE *file = NULL;

void u_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	if(file)
	{
		vfprintf(file, fmt, ap);
	}
	va_end(ap);

	return;
}

static void parse_register(const void *_ct, unsigned int *bp, unsigned int *sp, unsigned int *pc)
{
	struct ucontext* ct = (struct ucontext*)_ct;    
	
#if ARCH == PLATFORM_X86

	unsigned int *regs = (unsigned int*)ct->uc_mcontext.gregs;
	*bp = (unsigned int)regs[6];
	*sp = (unsigned int)regs[7];
	*pc = (unsigned int)regs[14];


#elif ARCH == PLATFORM_ARM

	struct sigcontext* sc  = &ct->uc_mcontext;
	*bp = (unsigned int)sc->arm_fp;
	*sp = (unsigned int)sc->arm_sp;
	*pc = (unsigned int)sc->arm_pc;

#elif ARCH == PLATFORM_SH4

	unsigned int *regs = (unsigned int*)ct->uc_mcontext.gregs;

//	int i;
//	for (i = 0; i < 32; i++)
//		printf("regs[%d]=%08x\n", i, regs[i]);
	
	*bp = (unsigned int)regs[19];
	*sp = (unsigned int)regs[15];
	*pc = (unsigned int)regs[16];

#elif ARCH == PLATFORM_ARM_V5T_LE

	struct sigcontext* sc  = &ct->uc_mcontext;
	*bp = (unsigned int)sc->arm_fp;
	*sp = (unsigned int)sc->arm_sp;
	*pc = (unsigned int)sc->arm_pc;
	
#elif ARCH == PLATFORM_POWERPC
	///< 需要确定bp、sp、pc对应的寄存器位置
	unsigned int *regs = (unsigned int*)ct->uc_mcontext.uc_regs->gregs;
	
	*bp = (unsigned int)regs[19];
	*sp = (unsigned int)regs[15];
	*pc = (unsigned int)regs[16];
#elif ARCH == PLATFORM_CSKY
	struct sigcontext* sc  = &ct->uc_mcontext;
	*bp = (unsigned int)0;
	*sp = (unsigned int)sc->sc_usp;
	*pc = (unsigned int)sc->sc_pc;
#endif

	u_printf("=====================  CPU registers  ======================\n");
	u_printf("PC: 0x%08x\n", *pc);
	u_printf("EBP: 0x%08x, ESP: 0x%08x\n", *bp, *sp);
}

static void u_printf_time()
{
	struct tm *tm_ptr;
	time_t local_time;
	char fmt_time[64];

	local_time = time(NULL);
	tm_ptr = localtime(&local_time);
	strftime(fmt_time, sizeof(fmt_time), "%F %H:%M:%S", tm_ptr);
      	u_printf("%s", fmt_time);
}

#include <ucontext.h>
static void exception_action(int signo, siginfo_t* info, void* ct)
{
	if (signal(signo,SIG_DFL ) == SIG_ERR)
	{
		printf("error  %d SIG_DFL  error！\n", signo);
	}
	unsigned int bp, sp, pc;
	char thread_name[256] = {0};
	int pid = getpid();
	int tid = gettid();

	get_proc_name(pid, g_proc_name, sizeof(g_proc_name));
	get_thread_name(tid, thread_name, sizeof(thread_name));

	u_printf("===================== exception occur ======================222\n");
	u_printf("pid: %d [%s], tid: %d [%s]\n", pid, g_proc_name, tid, thread_name);
	u_printf("signal num: %d\n", signo);
	//u_printf("exception time: ");
	//u_printf_time();
	u_printf("\n");

	parse_register(ct, &bp, &sp, &pc);

	callstack_trace(pc, sp, bp);

	u_printf("\n");
	if(file)
	{
		fclose(file);
	}

	//kill(pid, signo);
	//exit(-1);
}

int dahua_backtrace_init(char *path)
{
	struct sigaction sigact;
	
	sigemptyset (&sigact.sa_mask);
	memset(&sigact, 0, sizeof (struct sigaction));
	sigact.sa_flags = SA_ONESHOT | SA_SIGINFO;
	sigact.sa_sigaction = exception_action;
	sigaction(SIGHUP, &sigact, NULL);
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
	sigaction(SIGILL, &sigact, NULL);
	sigaction(SIGTRAP, &sigact, NULL);
	sigaction(SIGABRT, &sigact, NULL);
	sigaction(SIGBUS, &sigact, NULL);
	sigaction(SIGFPE, &sigact, NULL);
	sigaction(SIGSEGV, &sigact, NULL);
	sigaction(SIGPWR, &sigact, NULL);
	sigaction(SIGSYS, &sigact, NULL);
	if(!file && path)
	{
		file = fopen(path, "a+b");
		if(file)
		{
			unsigned int len = 0;
			fseek(file, 0L, SEEK_END);
			len = ftell(file);
			
			if(len > 32 * 1024)
			{
					fclose(file);
					file = fopen(path, "wr+");
			}
	   }
		else
		{
			printf("fopen path %s filed!\n", path);
		}
	}
	return 0;
}

void cxx_except_hook()
{
    return; 
}

