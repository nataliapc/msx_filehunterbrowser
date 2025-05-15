.PHONY: all clean

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

ROOTDIR = ..
BINDIR = $(ROOTDIR)/bin
LIBDIR = $(ROOTDIR)/libs
INCDIR = $(ROOTDIR)/includes
DSKDIR = $(ROOTDIR)/dsk
EXTERNALS = $(ROOTDIR)/externals
LIB_GUARD=@mkdir -p $(LIBDIR)

MAKE = make -s --no-print-directory


all: UNAPI_TCPIP


##################################### UNAPI_TCPIP

UNAPI_TCPIP: $(LIBDIR)/unapi_tcpip.lib

$(LIBDIR)/unapi_tcpip.lib:
	@echo "$(COL_YELLOW)######## Creating $@$(COL_RESET)"
	@$(MAKE) -C UNAPI_TCPIP
	@cp UNAPI_TCPIP/libs/unapi_tcpip.lib $(LIBDIR)
	@cp UNAPI_TCPIP/includes/hgetlib.h $(INCDIR)


#####################################

clean: clean_unapi_tcpip

clean_unapi_tcpip:
	@echo "$(COL_ORANGE)#### $@$(COL_RESET)"
	@$(MAKE) -C UNAPI_TCPIP clean
	@rm -f $(LIBDIR)/unapi_tcpip.lib
	@rm -f $(INCDIR)/hgetlib.h
