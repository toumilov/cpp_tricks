#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <execinfo.h>
#include <initializer_list>


static void sig_handler(int sig, siginfo_t *info, void *ucontext)
{
	// Only signal safe function shall be used here

	auto print = [](const char* s){
		write(STDERR_FILENO, s, strlen(s));
	};
	auto to_int = [](int n){
		static char buf[10];
		bool negative = n < 0;
		n = abs(n);
		int digits = (n == 0) ? 1 : floor(log10(n)) + 1;
		if (negative)
		{
			digits++;
		}
		buf[digits] = 0;
		int x = std::pow(10, digits);
		digits--;
		for(; digits >= 0; digits--)
		{
			buf[digits] = '0' + n % 10;
			n /= 10;
		}
		if (negative)
		{
			buf[0] = '-';
		}
		return buf;
	};
	auto to_hex_uint = [](uint64_t n){
		const char digits[] = "0123456789abcdef";
		static char buf[18];
		char *b = (char*)&n;
		int v, i, j = 0;
		for(i = 7; i >= 0; i--)
		{
			v = (b[i] >> 4) & 0x0F;
			buf[j++] = digits[v];
			v = b[i] & 0x0F;
			buf[j++] = digits[v];
		}
		buf[16] = 0;
		return buf;
	};

	// Signal info
	switch(sig)
	{
		case SIGINT:  print("SIGINT"); break;
		case SIGTERM: print("SIGTERM"); break;
		case SIGABRT: print("SIGABRT"); break;
		case SIGSEGV: print("SIGSEGV"); break;
		case SIGBUS:  print("SIGBUS"); break;
		default:      print(to_int(sig)); break;
	}
	print("\n");
	print("errno: ");
	print(to_int(info->si_errno));
	print("\n");
	print("code: ");
	print(to_int(info->si_code));
	print("\n");
	print("address: ");
	print(to_hex_uint(reinterpret_cast<uint64_t>((int*)info->si_addr)));
	print("\n");

	// Backtrace
	void *trace[10];
	int trace_size = backtrace(trace, 10);
	backtrace_symbols_fd(trace, trace_size, STDERR_FILENO);

	// ucontext_t* uc = (ucontext_t*)ucontext;
	// trace[1] = (void*)uc->uc_mcontext.gregs[REG_RIP];
	// char **messages = backtrace_symbols(trace, trace_size);
	// for(int i = 1; i < trace_size; ++i)
	// {
	// 	printf("[bt] %s\n", messages[i]);
	// }

	exit(EXIT_FAILURE);
}

void sig_setup()
{
	struct sigaction action;
	action.sa_sigaction = sig_handler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = SA_SIGINFO;
	static int signallist[]= {SIGINT, SIGHUP, SIGSEGV, SIGABRT};
	for(const auto& s : {SIGINT, SIGHUP, SIGSEGV, SIGABRT})
	{
		sigaction(s, &action, NULL);
	}
}

void foo()
{
	int* ptr = (int*)0x42;
	*ptr = 123;
}

int main()
{
	sig_setup();
	foo();
	return 0;
}