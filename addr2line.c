/*
 *	This file will be used in
 *		1) mkbiz command
 *			Build .debug_line index table and shrink the .debug_line section
 *		2) Stack tracer in target board
 *			Change the pc to filename:line_number pair to ease to debug
 */
#include <string.h>

#undef	CHECK_READELF_OUTPUT
#undef	PRINT

#ifdef	CHECK_READELF_OUTPUT
#define	PRINT(x...)			printf(x);
#else
#define	PRINT(x...)
#endif

#define	tprint0n(x...)		printf(x);

enum		/* DWARF2 Line number opcodes.			*/
{
    DW_LNS_EXTENDED_OP			=  0,
    DW_LNS_COPY					=  1,
    DW_LNS_ADVANCE_PC			=  2,
    DW_LNS_ADVANCE_LINE			=  3,
    DW_LNS_SET_FILE				=  4,
    DW_LNS_SET_COLUMN			=  5,
    DW_LNS_NEGATE_STMT			=  6,
    DW_LNS_SET_BASIC_BLOCK		=  7,
    DW_LNS_CONST_ADD_PC			=  8,
    DW_LNS_FIXED_ADVANCE_PC		=  9,
    /* DWARF 3.  */
    DW_LNS_SET_PROLOGUE_END		= 10,
    DW_LNS_SET_EPILOGUE_BEGIN	= 11,
    DW_LNS_SET_ISA				= 12
} DWARF_LINE_NUMBER_OPS_T;

enum		/* DWARF2 Line number extended opcodes.	*/
{
    DW_LNE_END_SEQUENCE			=  	1,
    DW_LNE_SET_ADDRESS			=  	2,
    DW_LNE_DEFINE_FILE			=  	3,
    DW_LNE_set_discriminator		=	4,
} DWARF_LINE_NUMBER_X_OPS_T;

/*
 *	CHECK_READELF_OUTPUT
 *		- define it to compare the search result with readelf command as followed.
 *		  # readelf -wl -e ucos > out.readelf
 *		  # mkbiz       -a ucos > out.mkbiz
 *		  # diff out.readelf out.mkbiz
 */

#define	DEBUG	0

unsigned int	*dwarfLst = 0;				/* Pointer to dwarf packet			*/
unsigned int	nDwarfLst = 0;				/* Number of dwarf packets			*/
unsigned char 	*pDwarfData = 0;
unsigned int	bFullPath = 1;

#define	NUM_FILES		512
#define	NUM_DIRS		512

static short getShort(char *pSrc)
{
	short data;
	char *pDst = (char *)&data;

	#ifdef	CHECK_ENDIAN
	if (need_swap)
	{
		pDst[0] = pSrc[1];
		pDst[1] = pSrc[0];
	}
	else
	{
	#endif
		pDst[0] = pSrc[0];
		pDst[1] = pSrc[1];
	#ifdef	CHECK_ENDIAN
	}
	#endif
	return(data);
}

static int getLong(char *pSrc)
{
	int	data;
	char *pDst = (char *)&data;

	#ifdef	CHECK_ENDIAN
	if (need_swap)
	{
		pDst[0] = pSrc[3];
		pDst[1] = pSrc[2];
		pDst[2] = pSrc[1];
		pDst[3] = pSrc[0];
	}
	else
	{
	#endif
		pDst[0] = pSrc[0];
		pDst[1] = pSrc[1];
		pDst[2] = pSrc[2];
		pDst[3] = pSrc[3];
	#ifdef	CHECK_ENDIAN
	}
	#endif
	return(data);
}

static unsigned int decodeULEB128 (char *cpp[])
{
	unsigned int  result;
	int           shift;
	unsigned char byte;

	result   = 0;
	shift    = 0;

	do
    {
		byte = *cpp[0]; cpp[0] += 1;
		result |= ((byte & 0x7f) << shift);
		shift += 7;
    }
	while (byte & 0x80);

	return result;
}

static int decodeSLEB128(char *cpp[])
{
	int           result;
	int           shift;
	unsigned char byte;

	result = 0;
	shift = 0;

	do
	{
		byte = *cpp[0]; cpp[0] += 1;
		result |= ((byte & 0x7f) << shift);
		shift += 7;
	}
	while (byte & 0x80);

	if ((shift < 32) && (byte & 0x40))
		result |= -(1 << shift);

	return result;
}

static char * _basename (const char *name)
{
	const char *base = NULL;

	if( name != NULL )
	{
		if (bFullPath)
			return (char *)name;

		for (base = name; *name; name++)
		{
			if ((*name == '/') || (*name == '\\'))
			{
				base = name + 1;
			}
		}
	}

	return (char *) base;
}

int searchLineInfo(char **ppDebugLine, size_t *pSize, unsigned int srchAddr, char **ppFileName)
{
	unsigned long	length;				/* Copy of Dwarf line info header */
	unsigned short	version;			/* Copy of Dwarf line info header */
	unsigned int	prologue_length;	/* Copy of Dwarf line info header */
	unsigned char	insn_min;			/* Copy of Dwarf line info header */
	unsigned char	default_is_stmt;	/* Copy of Dwarf line info header */
	int				line_base;			/* Copy of Dwarf line info header */
	unsigned char	line_range;			/* Copy of Dwarf line info header */
	unsigned char	opcode_base;		/* Copy of Dwarf line info header */
	unsigned int	ptr_size;			/* Size of pointer, fixed to 4 */

	int				i;
	char			*cp;
	char			*dwarfStart;		/* Start of current input dwarf packet */
	char			*pLineEnd;			/* End of current input dwarf packet */
	char			*secEndPtr;			/* End of debug_line section */
	char			*pEncStream;		/* Start of encoded opcode in current input packet */
	unsigned char	opcode;				/* current opcode */
	unsigned int	address = 0;		/* Current address */
	unsigned int	low_pc;				/* lowest address in current packet */
	unsigned int	high_pc;			/* lowest address in current packet */
	unsigned long	curr_offset = 0;	/* Current offset in input dwarf packet */
	unsigned int	lineNo, prevNo = 1;	/* Line number and previous line number */
	unsigned int	fileNo, newFno;		/* File number and new file number */
	int				bEos = 0, nAddedPkt = 0, bNewPc = 0, bAdded = 0;
	int				column, is_stmt, basic_block = 1;
	int				numFiles = 0;
	char			*fileList[NUM_FILES] = { 0,};


	#if (DEBUG > 0)
	tprint0n("searchLineInfo(0x%x, %d, 0x%x, 0x%x)\n", *ppDebugLine, *pSize, srchAddr, ppFileName);
	#endif

	PRINT("\n");
	PRINT("Dump of debug contents of section .debug_line:\n");
	PRINT("\n");

	cp = *ppDebugLine;
	secEndPtr = *ppDebugLine + *pSize;

	/*
	 *	Support 32bit format only
	 */
	while (cp < secEndPtr)
	{
		dwarfStart		= cp;
		ptr_size		= 4;
		length			= getLong(cp);  cp += 4;
		pLineEnd		= cp + length;
		version			= getShort(cp); cp += 2;
		prologue_length	= getLong(cp);  cp += 4;
		insn_min		= (unsigned char)(*cp++);
		default_is_stmt	= (unsigned char)(*cp++);
		line_base		= (signed   char)(*cp++);
		line_range		= (unsigned char)(*cp++);
		opcode_base		= (unsigned char)(*cp++);

		#if	0
		xlibc_hexdump("DwarfPacket", dwarfStart, 0x100);
		xlibc_hexdump("DwarfPacket", pLineEnd,   0x100);
		#endif

		PRINT("  Length:                      %u\n", length);
		PRINT("  DWARF Version:               %u\n", version);
		PRINT("  Prologue Length:             %u\n", prologue_length);
		PRINT("  Minimum Instruction Length:  %u\n", insn_min);
		PRINT("  Initial value of 'is_stmt':  %u\n", default_is_stmt);
		PRINT("  Line Base:                   %d\n", line_base);
		PRINT("  Line Range:                  %u\n", line_range);
		PRINT("  Opcode Base:                 %u\n", opcode_base);
		PRINT("  (Pointer size:               %u)\n", ptr_size);

		PRINT("\n");
		i = numFiles = 0;
		if (cp[0] == 0x00)
		{
			PRINT(" The File Name Table is empty.\n");
		}
		else
		{
			PRINT(" The File Name Table:\n");
			PRINT("  Entry	Dir	Time	Size	Name\n");
			while (cp[0] != 0x00)
			{
				unsigned int ch1 = 0, ch2 = 0, ch3 = 0;
				char *name;

				i++;
				name = cp;
				cp  += strlen(name) + 1;
				if (srchAddr == -1)				/* Removed in packed debug_line */
				{
					ch1  = decodeULEB128(&cp);	/* Dir */
					ch2  = decodeULEB128(&cp);	/* Time */
					ch3  = decodeULEB128(&cp);	/* Size */
				}
				PRINT("  %d	%d	%d	%d	%s\n", i, ch1, ch2, ch3, name);
				if (numFiles < NUM_FILES)
				{
					fileList[numFiles++] = name;
				}
			}
		}
		cp++;

		PRINT("\n");
		PRINT(" Line Number Statements:\n");

		if (cp >= (dwarfStart + length + 4))
		{
			if (cp > (dwarfStart + length + 4))
				PRINT("overrun %x :: %xn\n", cp, dwarfStart + length + 4);
			PRINT("\n");
		}

		nAddedPkt = 0;			/* Number of packets added */
		pEncStream = cp;		/* Start pointer of encoded actual line info stream */

		while (cp < pLineEnd)
		{
			opcode	= 0;
			lineNo	= 1;
			prevNo	= 1;
			fileNo	= 1;
			newFno	= 1;
			column	= 0;
			address	= 0;
			bEos	= 0;
			low_pc  = 0;
			bNewPc	= 1;
			bAdded 	= 0;
			high_pc = 0;
			is_stmt = default_is_stmt;
			basic_block = 1;

			for (i = 0; bEos == 0; i++)
			{
				opcode = *cp++;

				if (opcode >= opcode_base)
				{
					int	addrInc, lineInc;

					/* Mark valid line_info has been added */
					bAdded = 1;

					if (bNewPc || (address < low_pc)) { bNewPc = 0; low_pc = address; }

					/* Line and Address increment */
					opcode	-= opcode_base;

					lineInc  = line_base + (opcode % line_range);
					addrInc	 = ((opcode - lineInc + line_base) / line_range) * insn_min;
					lineNo	+= lineInc;
					address	+= addrInc;
					PRINT("  Special opcode %d: advance Address by %d to 0x%x and Line by %d to %d\n",
								opcode, addrInc, address, lineInc, lineNo);

					if (srchAddr < address)
					{
						#if (DEBUG > 0)
						tprint0n("Found at line case[1] %d, addr = 0x%x\n", prevNo, address);
						#endif
						*ppFileName = _basename(fileList[fileNo-1]);
						return(prevNo);
					}
					fileNo = newFno;

					prevNo	 = lineNo;
					if (address > high_pc) high_pc = address;
				}
				else
				{
					switch (opcode)
					{
					  case DW_LNS_EXTENDED_OP : 	 // 0,
					  {
						opcode = *cp++; /* Ignore length */
//						PRINT("  Length = %x\n", opcode);
						opcode = *cp++;

						switch (opcode)
						{
						  case DW_LNE_END_SEQUENCE : // 1,
						  {
							int  nStuff = 0;

							PRINT("  Extended opcode %d: End of Sequence\n", opcode);
							/*
							 *		Work arround for ARM axf file.
							 *	Norcroft ARM compiler add stuffing bytes to make 4 byte
							 *	align for the next packet.
							 *	It seems that there's a stuffing byte at the end of
							 *	Dwarf packet. Just skip it.
							 */
							if (cp >= pLineEnd)
							{
								for (nStuff = 0; (nStuff < 16) && (cp < secEndPtr); nStuff++, cp++)
								{
									if ((getLong(cp) != 0) && (getShort(cp+4) == 2))
										break;
									#if	0
									PRINT("  %08x %08x\n", getLong(cp), getLong(cp+4));
									#endif
								}
								#if	0
								if (nStuff)
								{
									PRINT("  nStuff=%d, cp =0x%x, pLineEnd=0x%x\n", nStuff, cp, pLineEnd);
								}
								#endif
							}
							else if (cp < pLineEnd)
							{
								PRINT("  More Sequence Presents\n");
							}
							bEos = 1;
							if (bNewPc || (address < low_pc)) { bNewPc = 0; low_pc = address; }
							if (address > high_pc) high_pc = address;
							if (srchAddr < address)
							{
								#if (DEBUG > 0)
								tprint0n("Found at line case[2] %d, addr = 0x%x\n", prevNo, address);
								#endif
								*ppFileName = _basename(fileList[fileNo-1]);
								return(prevNo);
							}
							break;
						  }
						  case DW_LNE_SET_ADDRESS :	 // 2,
						  {
							address = getLong(cp); cp += 4;
							if (bNewPc || (address < low_pc)) { bNewPc = 0; low_pc = address; }
							if (address > high_pc) high_pc = address;
					  		PRINT("  Extended opcode %d: set Address to 0x%x\n", opcode, address);
							break;
						  }
						  case DW_LNE_DEFINE_FILE : // 3
						  {
							unsigned int ch1, ch2, ch3;
							char *name;

							name = cp;
							cp  += strlen(name) + 1;
							ch1  = decodeULEB128(&cp);	/* Dir */
							ch2  = decodeULEB128(&cp);	/* Time */
							ch3  = decodeULEB128(&cp);	/* Size */
							PRINT("  Define file :  %d	%d	%d	%d	%s\n", numFiles, ch1, ch2, ch3, name);
							if (numFiles < NUM_FILES)
								fileList[numFiles] = name;
							break;
						  }
						  case DW_LNE_set_discriminator :
						  {
						  	char * cpcp ;
						  	int len;
							cpcp = cp ;
						  	len = decodeULEB128(&cp);
//							cp += len ;

							PRINT("Unknown extended opcode(%x)\n", opcode);
							break;
						  }
						  default:
						  	break;
						}
						break;
					  }
					  case DW_LNS_COPY : 				// 1,
					  {
						/* Mark valid line_info has been added */
						bAdded = 1;

						prevNo = lineNo;
						PRINT("  Copy\n");
						break;
					  }
					  case DW_LNS_ADVANCE_PC : 			// 2,
					  {
						int	addrInc;

						addrInc	 = decodeULEB128(&cp) * insn_min;
						address	+= addrInc;
						PRINT("  Advance PC by %d to %x\n", addrInc, address);
						if (srchAddr < address)
						{
							#if (DEBUG > 0)
							tprint0n("Found at line case[3] %d, addr = 0x%x\n", prevNo, address);
							#endif
							*ppFileName = _basename(fileList[fileNo-1]);
							return(prevNo);
						}
						fileNo = newFno;

						break;
					  }
					  case DW_LNS_ADVANCE_LINE : 		// 3,
					  {
						int	lineInc;

						lineInc	 = decodeSLEB128(&cp);
						lineNo	+= lineInc;
						PRINT("  Advance Line by %d to %d\n", lineInc, lineNo);
						break;
					  }
					  case DW_LNS_SET_FILE : 			// 4,
					  {
						newFno = decodeULEB128(&cp);
						PRINT("  Set File Name to entry %d in the File Name Table\n", newFno);
						break;
					  }
					  case DW_LNS_SET_COLUMN : 			// 5,
					  {
						column = decodeULEB128(&cp);
						PRINT("  Set column to %u\n", column);
						break;
					  }
					  case DW_LNS_NEGATE_STMT : 		// 6,
					  {
						is_stmt = !is_stmt;
						PRINT("  Set is_stmt to %d\n", is_stmt);
						break;
					  }
					  case DW_LNS_SET_BASIC_BLOCK :	 	// 7,
					  {
						basic_block = 1;
						PRINT("  Set basic to %d\n", basic_block);
						break;
					  }
					  case DW_LNS_CONST_ADD_PC : 		// 8,
					  {
						int	addrInc;

						addrInc	 = insn_min * ((255-opcode_base)/line_range);
						address	+= addrInc;
						PRINT("  Advance PC by constant %d to 0x%x\n", addrInc, address);
						break;
					  }
					  case DW_LNS_FIXED_ADVANCE_PC :	// 9,
					  {
						int	addrInc;

						addrInc	 = (unsigned)getShort(cp); cp += 2;
						address	+= addrInc;
						PRINT("  Command Fixed Advance PC\n");
						break;
					  }
					  case DW_LNS_SET_PROLOGUE_END:
						 break;

					  case DW_LNS_SET_EPILOGUE_BEGIN:
						 break;

					  case DW_LNS_SET_ISA:
					  {
							decodeULEB128(&cp);
					   		break;
					  }
					  default:
					  {

					  	break;
					  }

					}
				}
			}
		}

		if (srchAddr != -1)
		{
			*ppFileName = (char *)"??";
			return(0);
			/* If search address given, decode just 1 dwarf packet */
		}
		curr_offset += cp - dwarfStart;

	}

	return 0;
}

int addr2line(unsigned int addr, char **ppFileName)
{
	int		x, l = 0, r = nDwarfLst-1, matched = 0;
	int		lineNo = 0;

	if (dwarfLst == NULL)
		return 0;

	do
	{
		x = (l + r) / 2;
		if      (addr < dwarfLst[2*x+0]) { matched = 0; r = x - 1; }
		else if (addr < dwarfLst[2*x+2]) { matched = 1; l = x + 1; }
		else                             { matched = 0; l = x + 1; }

	} while ((l <= r) && (matched == 0));

	if (matched)
	{
		char	*pDwarf = pDwarfData + dwarfLst[2*x+1];
		size_t	size	= 4 + getLong(pDwarf);

		#if (DEBUG > 0)
		tprint0n("Address is in dwarf packet %d, [%x .. %x]\n", x, pDwarf, pDwarf+size);
		#endif
		lineNo = searchLineInfo(&pDwarf, &size, addr, ppFileName);
		#if (DEBUG > 0)
		tprint0n("%08x ==> %s:%d\n", addr, *ppFileName, lineNo);
		#endif
	}
	else
	{
		*ppFileName = (char *)"Not Found";
	}
	return lineNo;
}
