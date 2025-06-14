.PHONY: clean

SDCC_VER := 4.2.0
DOCKER_IMG = nataliapc/sdcc:$(SDCC_VER)
DOCKER_RUN = docker run -i --rm -u $(shell id -u):$(shell id -g) -v .:/src -w /src $(DOCKER_IMG)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	COL_RED = \e[1;31m
	COL_YELLOW = \e[1;33m
	COL_ORANGE = \e[1;38:5:208m
	COL_BLUE = \e[1;34m
	COL_GRAY = \e[1;30m
	COL_WHITE = \e[1;37m
	COL_RESET = \e[0m
endif

ROOTDIR = .
SRCDIR = $(ROOTDIR)/src
LIBDIR = $(ROOTDIR)/libs
INCDIR = $(ROOTDIR)/includes
OBJDIR = $(ROOTDIR)/obj
DIR_GUARD=@mkdir -p $(OBJDIR)
LIB_GUARD=@mkdir -p $(LIBDIR)

AS = $(DOCKER_RUN) sdasz80
AR = $(DOCKER_RUN) sdar
CC = $(DOCKER_RUN) sdcc

#DEFINES := -DUSE_TLS
#DEBUG := -D_DEBUG_
FULLOPT :=  --max-allocs-per-node 200000
LDFLAGS = -rc
OPFLAGS = --std-sdcc2x --less-pedantic --opt-code-size -pragma-define:CRT_ENABLE_STDIO=0
WRFLAGS = --disable-warning 196 --disable-warning 84 --disable-warning 85
CCFLAGS = --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx $(OPFLAGS) $(WRFLAGS) $(DEFINES) $(DEBUG)


REL_LIBS = 	$(addprefix $(OBJDIR)/, \
				AsmCall.rel \
				BiosCall.rel \
				DosCall.rel \
				UnapiCall.rel \
				UnapiGetCount.rel \
				UnapiGetRamHelper.rel \
				UnapiRead.rel \
				UnapiBuildCodeBlock.rel \
				UnapiParseCodeBlock.rel \
				 \
				base64.rel \
				 \
				hget.rel \
				net_getDriverInfo.rel \
				net_waitConnected.rel \
			)
TARGET = $(LIBDIR)/unapi_tcpip.lib

all: $(TARGET)

$(TARGET): $(REL_LIBS)
	@$(LIB_GUARD)
	@$(AR) $(LDFLAGS) $@ $^ ;
	@echo "$(COL_WHITE)######## Created $@$(COL_RESET)"

$(OBJDIR)/%.rel: $(SRCDIR)/%.s
	@echo "$(COL_BLUE)#### ASM $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(AS) -go $@ $^ ;

$(OBJDIR)/%.rel: $(SRCDIR)/%.asm
	@echo "$(COL_BLUE)#### ASM $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(AS) -go $@ $^ ;

$(OBJDIR)/%.rel: $(SRCDIR)/%.c
	@echo "$(COL_BLUE)#### CC $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -c -o $@ $^ ;


###################################################################################################

clean: cleanobj cleanlibs

cleanobj:
	@echo "$(COL_ORANGE)#### Cleaning obj$(COL_RESET)"
	@rm -rf $(OBJDIR)

cleanlibs:
	@echo "$(COL_ORANGE)#### Cleaning libs$(COL_RESET)"
	@rm -f $(LIBDIR)/*.lib
