#include	<fcntl.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<string.h>
#include	<errno.h>
#include	<time.h>
#include	<unistd.h>
#include	"types.h"
#ifdef	CYGWIN
#include	"win32_ext.h"
#else
#include	<ar.h>
#include	<sys/mman.h>		//	For mmap()
#endif

typedef struct {
	unsigned int addr;
	unsigned int end;
	unsigned int ptr;
} symEntry_t;

extern unsigned int		*dwarfLst;
extern unsigned int		nDwarfLst;
extern unsigned char	*pDwarfData;

unsigned int			nTxtSyms       = 0;
unsigned int			*pSymTabBase   = NULL;
char					*pSymStrBase   = NULL;
unsigned int			*pSymHashBase  = NULL;

size_t load_symbol(char symName[])
{
	uint_t*			pSymStorage = NULL;
	struct stat		sb;
	int				fd     = 0;
	int				len    = 0;
	int				nBytes = 0;

	printf("Checking & Importing symbols from file %s\n", symName);

	fd = open(symName, O_RDONLY, 0777);


	if ((fd > 0) && (fstat(fd, &sb) == 0) )
		len = sb.st_size;

	if (0 == len)
		return 0;

	nBytes = len;
	pSymStorage = (void*)mmap((void*) 0, len, PROT_READ, MAP_SHARED, fd, 0);

	if (((void *)pSymStorage != NULL) && (pSymStorage[0] == 0xB12791EE))
	{
		unsigned int	val1 = pSymStorage[2];	/* Total data size				*/
		unsigned int	val2 = pSymStorage[3];	/* Number of symbols			*/
		unsigned int	val3 = pSymStorage[4];	/* Size of symbol strings table */
		unsigned int	val4 = 3*sizeof(int)*val2 + val3;
		unsigned int	val9;

		val9 = val1 + 5 * 4;
		if ( (val9== nBytes) && (val1 == val4) )
		{
			nTxtSyms    = val2;
			pSymTabBase = pSymStorage + 5;
			pSymStrBase = (char *)(pSymTabBase + 3 * val2);

			if (*(unsigned int *)pSymStrBase == 0)
			{
				printf("16bit symbol Hash Mode\n");
//				pSymHashBase = (unsigned short *)((unsigned int)pSymStrBase + 4);
				pSymStrBase  = (char   *)(pSymHashBase + ((nTxtSyms + 1) & ~1));
			}
			else if (*(unsigned int *)pSymStrBase == 2)
			{
				printf("32bit symbol Hash Mode\n");
				pSymHashBase = (unsigned int *)((unsigned int)pSymStrBase + 4);
				pSymStrBase  = (char   *)(pSymHashBase + ((nTxtSyms + 1) & ~1));
			}
			if (*(unsigned int *)pSymStrBase == 1)
			{
				nDwarfLst	= ((unsigned int *)pSymStrBase)[1];
				val2		= ((unsigned int *)pSymStrBase)[2];
				dwarfLst	= (unsigned int *)(pSymStrBase + 12);
				pDwarfData	= (char *)(dwarfLst + 2 * nDwarfLst);
				pSymStrBase	= pDwarfData + val2;;
			}

			printf("nTxtSyms     = %d\n", nTxtSyms);
			printf("pSymTabBase  = [0x%06x..0x%06x)\n", pSymTabBase,  pSymTabBase + 3 * nTxtSyms);
			printf("pSymHashBase = [0x%06x..0x%06x)\n", pSymHashBase, pSymHashBase + nTxtSyms);
			printf("pSymStrBase  = [0x%06x..0x%06x)\n", pSymStrBase,  val9);

			printf("nDwarfLst    = %d\n", nDwarfLst);
			printf("pDwarfLst    = [0x%06x..0x%06x)\n", dwarfLst, dwarfLst + 2 * nDwarfLst);
			printf("pDwarfData   = [0x%06x..0x%06x)\n", pDwarfData, pDwarfData + val2);
		}
	}

	if (0)
	{
		#include	"util.h"
		unsigned int	i;
		symEntry_t		*pSyms = (symEntry_t *)pSymTabBase;

		hexdump("StrBase", (void*)pSymStrBase+pSyms[pSymHashBase[0]].ptr, 0x100);

		for (i = 0; i < nTxtSyms; i++)
			printf("[%6d] %s\n", i, &pSymStrBase[pSyms[pSymHashBase[i]].ptr]);

	}

	return nBytes;
}

unsigned int find_sym_byAddr(unsigned int addr, char **pSymName)
{
	int			x, l = 0, r = nTxtSyms-1, matched = 0;
	symEntry_t	*pSyms = (symEntry_t *)pSymTabBase;

	if (pSymName == NULL)
		return 0;

	if (pSyms == NULL)
	{
		*pSymName = (char *)"No symbol table";
		return 0;
	}

	do
	{
		x = (l + r) / 2;
		if      (addr < pSyms[x].addr) { matched = 0; r = x - 1; }
		else if (addr < pSyms[x].end ) { matched = 1; l = x + 1; }
		else                           { matched = 0; l = x + 1; }

	} while ((l <= r) && (matched == 0));

	if (matched)
	{
		*pSymName = &pSymStrBase[pSyms[x].ptr];
		return(pSyms[x].addr);
	}
	else
	{
		*pSymName = (char *)"Not Found";
		return 0;
	}
}

char *find_symbol_name(unsigned int addr)
{
	char		*pSymName;
	static char nameBuf[12];

	if (find_sym_byAddr(addr, &pSymName) != 0)
		return pSymName;

	snprintf(nameBuf, 12, "%#x", addr);
	return(nameBuf);
}

unsigned int find_sym_byName(char *symName)
{
	int			x, l = 0, r = nTxtSyms-1, matched = 0, rc;
	unsigned int		addr = 0;
	symEntry_t	*pSyms = (symEntry_t *)pSymTabBase;

	do
	{
		x = (l + r) / 2;
		rc = strcmp(symName, &pSymStrBase[pSyms[pSymHashBase[x]].ptr]);
		if      (rc < 0) { matched = 0; r = x - 1; }
		else if (rc > 0) { matched = 0; l = x + 1; }
		else             { matched = 1;            }

	} while ((l <= r) && (matched == 0));

	if (matched)
	{
		addr = pSyms[pSymHashBase[x]].addr;
	}

	return(addr);
}


