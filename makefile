OSTYPE		 = cygwin
TOP_DIR		 = .

include		$(TOP_DIR)/incs.mk


TGT			 = $(BIN_DIR)/symbolreader

SRCS		 =
SRCS		+= main.c
SRCS		+= sym_process.c
SRCS		+= addr2line.c
ifeq ($(OSTYPE), cygwin)
SRCS		+= win32_ext.c
endif
SRCS		+= util.c

OBJS		 = $(addprefix $(OBJ_DIR)/, $(notdir $(SRCS:.c=.o)))

all : $(TGT)

include		$(TOP_DIR)/rules.mk
