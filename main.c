#include	"types.h"
#include	"sym_process.h"
#include	"util.h"

int main (int argc, char **argv)
{
	unsigned int	addr    = 0;
	char			*symbol = NULL;
	char 			*file   = NULL;
	int				lineNo  = 0;

	//hexdump("load_symbol", (void *)(load_symbol), 0x100);
	load_symbol(argv[1]);

	printf("call find_sym_byName\n");
	addr = find_sym_byName(argv[2]);
	lineNo = addr2line(addr, &file);
	printf("addr:%x, line:%d, file:%s\n", addr, lineNo, file);

	printf("Input address:");
	fflush(stdout);
	scanf("%x", &addr);
	fflush(stdin);
	//addr += 0x35;

	printf("call find_sym_byAddr\n");
	find_sym_byAddr(addr, &symbol);
	printf("find symbol = %s\n", symbol);

	lineNo = addr2line(addr, &file);
	printf("addr:%x, line:%d, file:%s\n", addr, lineNo, file);
	return 0;
}
