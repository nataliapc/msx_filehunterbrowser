.PHONY: clean contrib test release resview dsk rom

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
BINDIR = $(ROOTDIR)/bin
SRCDIR = $(ROOTDIR)/src
SRCLIB = $(SRCDIR)/libs
LIBDIR = $(ROOTDIR)/libs
INCDIR = $(ROOTDIR)/includes
RESDIR = $(ROOTDIR)/res
OBJDIR = $(ROOTDIR)/obj
DSKDIR = $(ROOTDIR)/dsk
CONTRIB = $(ROOTDIR)/contrib
EXTERNALS = $(ROOTDIR)/externals
DIR_GUARD=@mkdir -p $(OBJDIR)
LIB_GUARD=@mkdir -p $(LIBDIR)

AS = $(DOCKER_RUN) sdasz80
AR = $(DOCKER_RUN) sdar
CC = $(DOCKER_RUN) sdcc
HEX2BIN = hex2bin
MAKE = make -s --no-print-directory
JAVA = java
DSKTOOL = $(BINDIR)/dsktool
OPENMSX = openmsx

EMUEXT = -ext debugdevice
EMUEXT1 = $(EMUEXT) -ext Mitsubishi_ML-30DC_ML-30FD
EMUEXT2 = $(EMUEXT) -ext msxdos2
EMUEXT2P = $(EMUEXT) -ext msxdos2 -ext ram512k
EMUSCRIPTS = -script $(ROOTDIR)/emulation/boot.tcl


DEFINES := -D_DOSLIB_
#DEBUG := -D_DEBUG_
#FULLOPT :=  --max-allocs-per-node 200000
LDFLAGS = -rc
OPFLAGS = --std-sdcc2x --less-pedantic --opt-code-size -pragma-define:CRT_ENABLE_STDIO=0
WRFLAGS = --disable-warning 196 --disable-warning 84
CCFLAGS = --code-loc 0x07c0 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx $(OPFLAGS) $(WRFLAGS) $(DEFINES) $(DEBUG)


LIBS = unapi_tcpip.lib dos.lib conio.lib utils.lib vdp.lib
REL_LIBS = 	$(addprefix $(LIBDIR)/, $(LIBS)) \
			$(addprefix $(OBJDIR)/, \
				crt0msx_msxdos_advanced.rel \
				fh.rel \
				heap.rel \
				mod_downloadFiles.rel \
				mod_searchString.rel \
				mod_help.rel \
			)

PROGRAM = fh
DSKNAME = $(PROGRAM).dsk


all: contrib $(OBJDIR)/$(PROGRAM).com release

contrib:
	@$(MAKE) -C $(CONTRIB) all SDCC_VER=$(SDCC_VER)

$(LIBDIR)/conio.lib:
	@$(MAKE) -j -C $(EXTERNALS)/sdcc_msxconio all SDCC_VER=$(SDCC_VER) DEFINES=-DXXXXX
	@$(LIB_GUARD)
	@cp $(EXTERNALS)/sdcc_msxconio/lib/conio.lib $@
	@cp $(EXTERNALS)/sdcc_msxconio/include/conio.h $(INCDIR)
	@cp $(EXTERNALS)/sdcc_msxconio/include/conio_aux.h $(INCDIR)

$(LIBDIR)/dos.lib:
	@$(MAKE) -j -C $(EXTERNALS)/sdcc_msxdos all SDCC_VER=$(SDCC_VER) DEFINES=-DDISABLE_CONIO
	@$(LIB_GUARD)
	@cp $(EXTERNALS)/sdcc_msxdos/lib/dos.lib $@
	@cp $(EXTERNALS)/sdcc_msxdos/include/dos.h $(INCDIR)
	@$(AR) -d $@ dos_putchar.c.rel dos_cputs.c.rel dos_kbhit.c.rel dos_cprintf.c.rel ;

#$(LIBDIR)/unapi_net.lib: $(patsubst $(SRCLIB)/%, $(OBJDIR)/%.rel, $(wildcard $(SRCLIB)/unapinet_*))
#	@echo "$(COL_WHITE)######## Creating $@$(COL_RESET)"
#	@$(LIB_GUARD)
#	@$(AR) $(LDFLAGS) $@ $^ ;

$(LIBDIR)/utils.lib: $(patsubst $(SRCLIB)/%, $(OBJDIR)/%.rel, $(wildcard $(SRCLIB)/utils_*))
	@echo "$(COL_WHITE)######## Creating $@$(COL_RESET)"
	@$(LIB_GUARD)
	@$(AR) $(LDFLAGS) $@ $^ ;

$(LIBDIR)/vdp.lib: $(patsubst $(SRCLIB)/%, $(OBJDIR)/%.rel, $(wildcard $(SRCLIB)/vdp_*))
	@echo "$(COL_WHITE)######## Creating $@$(COL_RESET)"
	@$(LIB_GUARD)
	@$(AR) $(LDFLAGS) $@ $^ ;

$(OBJDIR)/%.rel: $(SRCDIR)/%.s
	@echo "$(COL_BLUE)#### ASM $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(AS) -go $@ $^ ;

$(OBJDIR)/%.rel: $(SRCDIR)/%.c
	@echo "$(COL_BLUE)#### CC $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -c -o $@ $^ ;

$(OBJDIR)/%.c.rel: $(SRCLIB)/%.c
	@echo "$(COL_BLUE)#### CC $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -c -o $@ $^ ;

$(OBJDIR)/%.s.rel: $(SRCLIB)/%.s
	@echo "$(COL_BLUE)#### ASM $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(AS) -go $@ $^ ;

$(OBJDIR)/$(PROGRAM).rel: $(SRCDIR)/$(PROGRAM).c $(wildcard $(INCDIR)/*.h)
	@echo "$(COL_BLUE)#### CC $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -c -o $@ $< ;

$(OBJDIR)/$(PROGRAM).com: $(REL_LIBS) $(wildcard $(INCDIR)/*.h)
	@echo "$(COL_YELLOW)######## Compiling $@$(COL_RESET)"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) $(FULLOPT) -I$(INCDIR) -L$(LIBDIR) $(REL_LIBS) -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@)


release: $(OBJDIR)/$(PROGRAM).com
	@echo "$(COL_WHITE)**** Copying $^ file to $(DSKDIR)$(COL_RESET)"
	@cp $(OBJDIR)/$(PROGRAM).com $(DSKDIR)

$(DSKNAME): all
	@echo "$(COL_WHITE)**** $(DSKNAME) generating ****$(COL_RESET)"
	@rm -f $(DSKNAME)
	@$(DSKTOOL) c 360 $(DSKNAME) > /dev/null
	@cd dsk ; ../$(DSKTOOL) a ../$(DSKNAME) \
		MSXDOS.SYS COMMAND.COM AUTOEXEC.BAT \
		$(PROGRAM).com > /dev/null

dsk: $(DSKNAME)

###################################################################################################

clean: cleanobj cleanlibs
	@rm -f $(OBJDIR)/$(PROGRAM).com $(DSKDIR)/$(PROGRAM).com \
	       $(DSKNAME)

cleanprogram:
	@echo "$(COL_ORANGE)#### Cleaning program files$(COL_RESET)"
	@rm -f $(REL_LIBS)

cleanobj:
	@echo "$(COL_ORANGE)#### Cleaning obj$(COL_RESET)"
	@rm -f $(DSKDIR)/$(PROGRAM).com
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -rf $(OBJDIR)

cleanlibs:
	@echo "$(COL_ORANGE)#### Cleaning libs$(COL_RESET)"
	@rm -f $(addprefix $(LIBDIR)/, $(LIBS))
	@$(MAKE) -C $(EXTERNALS)/sdcc_msxdos clean
	@$(MAKE) -C $(EXTERNALS)/sdcc_msxconio clean
	@$(MAKE) -C $(CONTRIB) clean


###################################################################################################

test: all
	@$(BINDIR)/create_sym_debug.py $(OBJDIR)/$(PROGRAM)
	@mv $(OBJDIR)/$(PROGRAM)_opmdeb.sym $(OBJDIR)/program.sym
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
#		$(OPENMSX) -machine msx2plus $(EMUEXT2P) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPENMSX) -machine Sony_HB-F1XD $(EMUEXT2) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPENMSX) -machine Panasonic_FS-A1WSX $(EMUEXT2) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPENMSX) -machine Sony_HB-F700S $(EMUEXT2) -diska $(DSKDIR) $(EMUSCRIPTS) \
#		$(OPENMSX) -machine Toshiba_HX-10 $(EMUEXT1) -diska $(DSKDIR) $(EMUSCRIPTS) \
		$(OPENMSX) -machine turbor $(EMUEXT) -diska $(DSKDIR) $(EMUSCRIPTS) \
	; fi'
