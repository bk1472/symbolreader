OSTYPE		 = cygwin
TOP_DIR		 = .

include		$(TOP_DIR)/incs.mk


APP_NAME	 = readsym
TGT			 = $(BIN_DIR)/$(APP_NAME)

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
	@cp $(BIN_DIR)/$(APP_NAME) $(RELEASE_DIR)/$(APP_NAME)

include		$(TOP_DIR)/rules.mk
