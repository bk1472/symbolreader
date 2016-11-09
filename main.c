#include	"types.h"
#include	"sym_process.h"
#include	"util.h"

int main (int argc, char **argv)
{
	unsigned int	addr    = 0;
	char			*symbol = NULL;
	char 			*file   = NULL;
	int				lineNo  = 0;

	if (argc != 3)
	{
		printf("Usage: $ %s [symbolname] [addr:symbol name]\n", basename(argv[0]));
		exit(-1);
	}
	load_symbol(argv[1]);

	if (argv[2][0] == '0' && argv[2][1] == 'x')
	{
		addr = strtoll(argv[2], NULL, 16);
	}

	if (addr == 0)
	{
		addr = find_sym_byName(argv[2]);
		symbol = argv[2];
	}
	else
	{
		find_sym_byAddr(addr, &symbol);
	}
	lineNo = addr2line(addr, &file);
	printf("symbol:%s addr:%x, line:%d, file:%s\n", symbol, addr, lineNo, file);

	return 0;
}
