#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "util.h"

#define	GHD_BUFSZ			80

static	int	err_code = 0;

static void hexdump_fp(FILE *fp, const char *name, void *vcp, int size)
{
	static UCHAR n2h[] = "0123456789abcdef";
	int		i, hpos, cpos;
	char	buf[GHD_BUFSZ+1] = {0};
	UCHAR	*cp, uc;
	volatile ULONG	*lp, word_buff[4];
	ULONG	crc_val = 0;

	snprintf(buf, GHD_BUFSZ, "%s(Size=0x%x, CRC32=0x%08x)", name, size, crc_val);
	buf[GHD_BUFSZ] = 0;
	fprintf(fp,"%s\n",buf);

	if (size == 0) return;

	memset(buf, ' ', GHD_BUFSZ);

	cp = (UCHAR*)vcp;
	hpos = cpos = 0;
	for (i=0; i < size; ) {

		if ((i % 16) == 0) {
			snprintf(buf, GHD_BUFSZ, "\t0x%08x(%04x):", (int)vcp+i, i);
			hpos = 18;
		}

		if ((i % 4)  == 0) buf[hpos++] = ' ';

		if ((i+0)<size) {uc=cp[i+0]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}
		if ((i+1)<size) {uc=cp[i+1]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}
		if ((i+2)<size) {uc=cp[i+2]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}
		if ((i+3)<size) {uc=cp[i+3]; buf[hpos++]=n2h[(uc&0xF0)>>4];buf[hpos++]=n2h[uc&15];}

		cpos = (i%16) + 56;

		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}
		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}
		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}
		if (i<size) {buf[cpos++] = (isprint(cp[i]) ? cp[i] : '.'); i++;}

		if ((i%16) == 0) {
			buf[cpos] = 0x00;
			fprintf(fp,"%s\n", buf);
		}
	}
	buf[cpos] = 0x00;
	if ((i%16) != 0) {
		for ( ; hpos < 56; hpos++)
			buf[hpos] = ' ';
		fprintf(fp,"%s\n", buf);
	}
}


void hexdump(const char *name, void *vcp, int size)
{
	hexdump_fp(stdout, name, vcp, size);
}


void err_print(char *file, char *string)
{
    (void) fflush(stdout);
   	(void) fprintf(stderr, "getsym: %s: %s\n", file, string);
	errcase();
    return;
}

void errcase(void)
{
	err_code++;
}

int exitcode(void)
{
	return err_code;
}
