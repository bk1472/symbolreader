#include	"types.h"
#include	"sym_process.h"
#include	"util.h"

char tmpBuf[1024];
int main (int argc, char **argv)
{
	unsigned int	addr    = 0;
	char			*symbol = NULL;
	char 			*file   = NULL;
	int				lineNo  = 0;
	char			*symBuf = NULL;
	char			mod;
	char			*chk;

	if (argc != 3)
	{
		printf("Usage: $ %s [symbolname] [addr:symbol name]\n", basename(argv[0]));
		exit(-1);
	}
	load_symbol(argv[1]);

	/*Digit check*/
	chk = argv[2];
	if (chk[0] == '0' &&(chk[1] == 'x' || chk[1] == 'X'))
		mod = 'a';
	else
	{
		mod = 'a';
		for(;*chk != '\0'; chk++)
		{
			if (*chk >= '0' && *chk <= '9')
				continue;
			if (*chk >= 'a' && *chk <= 'f')
				continue;
			if (*chk >= 'A' && *chk <= 'F')
				continue;
			mod = 's';
			break;
		}
	}


	if (mod == 's')
	{
		int len = strlen(argv[2]);

		symBuf = (char*)malloc(len + 2);

		symBuf[0] = '_';
		strncpy(&symBuf[1], argv[2], len);
		symBuf[len+1] = '\0';
		addr = find_sym_byName(symBuf);
		symbol = &symBuf[1];
	}
	else
	{
		int i = 0;
		addr=strtoul(argv[2], NULL, 16);
		find_sym_byAddr(addr, &symbol);
		if (symbol[0] == '_')
			symbol = &symbol[1];
		strcpy(&tmpBuf[0], symbol);
		symbol = &tmpBuf[0];
		while(symbol[i] != '\0')
		{
			if (symbol[i] == '.') {
				symbol[i] = '\0';
				break;
			}
			i++;
		}

	}

	lineNo = addr2line(addr, &file);

	fprintf(stdout, "%c[pc:0x%08x, %s()@%s:%d]\n", mod, addr, symbol, file, lineNo);

	if(symBuf)
		free(symBuf);

	return 0;
}
