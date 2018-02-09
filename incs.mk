.PHONY : tags

BUILD_DIR	 = $(TOP_DIR)/build
OBJ_DIR		 = $(BUILD_DIR)/objs
BIN_DIR		 = $(BUILD_DIR)/bin
RELEASE_DIR	 = $(TOP_DIR)/..

INC_DIR		 = $(TOP_DIR)/include
SCRPT_DIR	 = $(TOP_DIR)/script

BUILD_MACH	?= cygwin64

ifeq ($(BUILD_MACH),cygwin64)
CC			 = x86_64-w64-mingw32-gcc # cygwin64
else
CC			 = gcc                    # cygwin32 / linux
endif

CFLAGS		 =
CFLAGS		+= -O2
CFLAGS		+= -I$(INC_DIR) -I.
CFLAGS		+= -Wno-format
CFLAGS		+= -Wno-unused-result
CFLAGS		+= -Wno-pointer-to-int-cast
CFLAGS		+= -Wno-int-to-pointer-cast

ifeq ($(OSTYPE),cygwin)
CFLAGS		+= -DCYGWIN
endif
CFLAGS		+= -DBIT_WIDTH=32

all :

tags :
	@ctags -R
