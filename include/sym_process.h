#ifndef _SYM_PROCESS_H_
#define _SYM_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include	"types.h"

extern size_t		load_symbol			(char symName[]);
extern unsigned int	find_sym_byAddr		(unsigned int addr, char **pSymName);
extern char*		find_symbol_name	(unsigned int addr);
extern unsigned int	find_sym_byName		(char *symName);
extern int			addr2line			(unsigned int addr, char **ppFileName);

#ifdef __cplusplus
}
#endif

#endif/*_SYM_PROCESS_H_*/
