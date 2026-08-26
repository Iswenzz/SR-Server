TARGETNAME=cod4x18_dedrun

BUILD_NUMBER=$(shell git rev-list --count HEAD)
BUILD_BRANCH=$(shell git rev-parse --abbrev-ref HEAD)
BUILD_REVISION=$(shell git rev-parse HEAD)

ifeq ($(BUILD_NUMBER), )
BUILD_NUMBER:=0
endif

ifeq ($(BUILD_BRANCH), )
BUILD_BRANCH:=no-branch
endif

ifeq ($(BUILD_REVISION), )
BUILD_REVISION:=no-revision
endif

##################################
# Compilers
CC=gcc
CPP=g++
NASM=nasm

##################################
# Defines
WIN_DEFINES=WINVER=0x501
LINUX_DEFINES=_GNU_SOURCE

##################################
# Flags
COMMON_FLAGS=-m32 -msse2 -mfpmath=sse -Wall -fno-omit-frame-pointer -fmax-errors=15 -Wno-unused-result -Isrc
CFLAGS=$(COMMON_FLAGS) -std=gnu11
CXXFLAGS=$(COMMON_FLAGS) -std=gnu++11
SR_LLIBS=SR CoD4DM1 samplerate speex
WIN_LFLAGS=-m32 -g -Wl,--nxcompat,--stack,0x800000 -static-libgcc -static -lm
WIN_LLIBS=tomcrypt mbedtls mbedcrypto mbedx509 ws2_32 wsock32 iphlpapi gdi32 winmm crypt32 stdc++ dbghelp ole32 $(SR_LLIBS)
LINUX_LFLAGS=-m32 -g -static-libgcc -rdynamic -Wl,-rpath=./
LINUX_LLIBS=tomcrypt mbedtls mbedcrypto mbedx509 dl pthread m stdc++ $(SR_LLIBS)
BSD_LLIBS=tomcrypt mbedtls mbedcrypto mbedx509 pthread m execinfo stdc++ $(SR_LLIBS)
COD4X_DEFINES=BUILD_NUMBER=$(BUILD_NUMBER) BUILD_BRANCH=$(BUILD_BRANCH) BUILD_REVISION=$(BUILD_REVISION)

ifeq ($(DEBUG), true)
DCFLAGS=-fno-pie -O0 -g
else
DCFLAGS=-fno-pie -O1 -DNDEBUG
endif

##################################
# Directories
SRC_DIR=src
PLUGIN_DIR=plugins
BIN_DIR=bin
LIB_DIR=lib
OBJ_DIR=obj
PLUGINS_DIR=plugins
ZLIB_DIR=$(SRC_DIR)/zlib
WIN_DIR=$(SRC_DIR)/win32
LINUX_DIR=$(SRC_DIR)/unix
ASSETS_DIR=$(SRC_DIR)/xassets
EXTERNAL=mbedtls tomcrypt

##################################
# Windows
ifeq ($(OS),Windows_NT)
BIN_EXT=.exe
NASMFLAGS=-f win -dWin32 --prefix _
OS_SOURCES=$(wildcard $(WIN_DIR)/*.c)
OS_OBJ=$(patsubst $(WIN_DIR)/%.c,$(OBJ_DIR)/%.o,$(OS_SOURCES))
C_DEFINES=$(addprefix -D,$(COD4X_DEFINES) $(WIN_DEFINES))
LFLAGS=$(WIN_LFLAGS)
LLIBS=-L$(LIB_DIR)/ $(addprefix -l,$(WIN_LLIBS))
RESOURCE_FILE=src/win32/win_cod4.res
DEF_FILE=$(BIN_DIR)/$(TARGETNAME).def
INTERFACE_LIB=$(PLUGINS_DIR)/libcom_plugin.a
ADDITIONAL_OBJ=$(INTERFACE_LIB)
CLEAN=-del $(subst /,\\,$(OBJ_DIR)/*.o $(LIB_DIR)/*.a $(DEF_FILE) $(INTERFACE_LIB))

##################################
# Linux
else
BIN_EXT=
NASMFLAGS=-f elf
OS_SOURCES=$(wildcard $(LINUX_DIR)/*.c)
OS_OBJ=$(patsubst $(LINUX_DIR)/%.c,$(OBJ_DIR)/%.o,$(OS_SOURCES))
C_DEFINES=$(addprefix -D,$(COD4X_DEFINES) $(LINUX_DEFINES))
LFLAGS=$(LINUX_LFLAGS)

UNAME := $(shell uname)
ifeq ($(UNAME),FreeBSD)
LLIBS=-L./$(LIB_DIR) $(addprefix -l,$(BSD_LLIBS))
else
LLIBS=-L./$(LIB_DIR) $(addprefix -l,$(LINUX_LLIBS))
endif

RESOURCE_FILE=
ADDITIONAL_OBJ=
CLEAN=-rm $(OBJ_DIR)/*.o $(DEF_FILE) $(INTERFACE_LIB)
endif

##################################
# Sources
TARGET=$(addprefix $(BIN_DIR)/,$(TARGETNAME)$(BIN_EXT))
ASM_SOURCES=$(wildcard $(SRC_DIR)/asmsource/*.asm)
C_SOURCES=$(wildcard $(SRC_DIR)/*.c)
CPP_SOURCES=$(wildcard $(SRC_DIR)/*.cpp)
ZLIB_SOURCES=$(wildcard $(ZLIB_DIR)/*.c)
ASSETS_SOURCES=$(wildcard $(ASSETS_DIR)/*.c)

##################################
# Objects
ASM_OBJ=$(patsubst $(SRC_DIR)/asmsource/%.asm,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
C_OBJ=$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SOURCES))
CPP_OBJ=$(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(CPP_SOURCES))
ZLIB_OBJ=$(patsubst $(ZLIB_DIR)/%.c,$(OBJ_DIR)/%.o,$(ZLIB_SOURCES))
ASSETS_OBJ=$(patsubst $(ASSETS_DIR)/%.c,$(OBJ_DIR)/%.o,$(ASSETS_SOURCES))

##################################
# Targets
all: $(EXTERNAL) $(TARGET) $(ADDITIONAL_OBJ)

mbedtls:
	@echo   $(MAKE)  $@
	@$(MAKE) -C $(SRC_DIR)/$@

tomcrypt:
	@echo   $(MAKE)  $@
	@$(MAKE) -C $(SRC_DIR)/$@

clean:
	@echo   clean Server
	@$(CLEAN)
	@echo   clean Mbedtls
	@$(MAKE) -C $(SRC_DIR)/mbedtls clean
	@echo   clean Tomcrypt
	@$(MAKE) -C $(SRC_DIR)/tomcrypt clean

##################################
# Rules
$(TARGET): $(OS_OBJ) $(C_OBJ) $(CPP_OBJ) $(ZLIB_OBJ) $(ASSETS_OBJ) $(ASM_OBJ) $(OBJ_DIR)/version.o
	@echo   $(CXX) $(TARGET)
	@$(CXX) $(LFLAGS) -o $@ $^ $(RESOURCE_FILE) $(LLIBS)

$(OBJ_DIR)/version.o: src/version/version.c
	@echo   $(CC)  $@ $(C_DEFINES)
	@$(CC) -c $(CFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo   $(CC)  $@
	@$(CC) -c $(CFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo   $(CPP)  $@
	@$(CPP) -c $(CXXFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/asmsource/%.asm
	@echo   $(NASM)  $@
	@$(NASM) $(NASMFLAGS) $< -o $@

$(OBJ_DIR)/%.o: $(ZLIB_DIR)/%.c
	@echo   $(CC)  $@
	@$(CC) -c $(CFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(OBJ_DIR)/%.o: $(ASSETS_DIR)/%.c
	@echo   $(CC)  $@
	@$(CC) -c $(CFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(OBJ_DIR)/%.o: $(WIN_DIR)/%.c
	@echo   $(CC)  $@
	@$(CC) -c $(CFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(OBJ_DIR)/%.o: $(LINUX_DIR)/%.c
	@echo   $(CC)  $@
	@$(CC) -c $(CFLAGS) $(DCFLAGS) $(C_DEFINES) -o $@ $<

$(INTERFACE_LIB): $(DEF_FILE) $(TARGET)
	@echo   dlltool  $@
	@dlltool -D $(TARGET) -d $(DEF_FILE) -l $@

$(DEF_FILE): $(TARGET)
	@echo   pexports  $@
	@pexports $^ > $@
