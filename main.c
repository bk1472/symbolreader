#include	"types.h"
#include	"sym_process.h"
#include	"util.h"

int main (int argc, char **argv)
{
	unsigned int	addr    = 0;
	char			*symbol = NULL;
	char 			*file   = NULL;
	int				lineNo  = 0;
	char			*symBuf = NULL;

	if (argc != 3)
	{
		printf("Usage: $ %s [symbolname] [addr:symbol name]\n", basename(argv[0]));
		exit(-1);
	}
	load_symbol(argv[1]);

	if ((addr=strtoll(argv[2], NULL, 16)) == 0) // input string is bot number
	{
		int len = strlen(argv[2]);

		symBuf = (char*)malloc(len + 2);

		symBuf[0] = '_';
		strncpy(&symBuf[1], argv[2], len);

		addr = find_sym_byName(symBuf);
		symbol = symBuf;
	}
	else
	{
		find_sym_byAddr(addr, &symbol);
	}

	lineNo = addr2line(addr, &file);
	if (symbol[0] == '_')
		symbol = &symbol[1];

	fprintf(stdout, "[pc:0x%08x, %s()@%s:%d]\n", addr, symbol, file, lineNo);

	free(symBuf);
	return 0;
}
