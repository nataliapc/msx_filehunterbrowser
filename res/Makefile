.PHONY: all clean clean_h

ROOTDIR = ..
BINDIR = $(ROOTDIR)/bin
INCDIR = $(ROOTDIR)/includes
UTILSDIR = $(ROOTDIR)/src/libs
OBJDIR = $(ROOTDIR)/obj
LIBDIR = $(ROOTDIR)/lib
OUTDIR = ./out
OUT_GUARD=@mkdir -p $(OUTDIR)

HELP_TXT = help.txt
HELP_BIN = help.bin
HELP_ZX0 = help.bin.zx0
HELP_ZX0_C := utils_help_zx0.c


all: $(OUTDIR)/$(HELP_ZX0_C)

$(OUTDIR)/$(HELP_BIN): $(HELP_TXT)
	@$(OUT_GUARD)
	# remove \n\r from the txt and create with it the binary file, maintaining the trailing spaces
	@sed 's/\r//g' $< | sed 's/$$/ /' | tr -d '\n' | tr -d '\r' | tr -d '\t' | tr -d '\f' > $@


$(OUTDIR)/$(HELP_ZX0): $(OUTDIR)/$(HELP_BIN)
	@$(BINDIR)/zx0 -f $< $@
	# add 2 bytes at start of HELP_ZX0 binary with the size of HELP_BIN binary file on little-endian format
	@SIZE=$$(stat -c %s $(OUTDIR)/$(HELP_BIN)); \
		LOW=$$(printf "%02x" $$(( $$SIZE & 0xFF ))); \
		HIGH=$$(printf "%02x" $$(( ( $$SIZE >> 8 ) & 0xFF ))); \
		echo -n "$$LOW$$HIGH" | xxd -r -p | cat - $@ > $@.tmp
	@mv $@.tmp $@
	

$(OUTDIR)/$(HELP_ZX0_C): $(OUTDIR)/$(HELP_ZX0)
	@which xxd > /dev/null || { echo "Installing xxd..."; sudo apt-get update -qq && sudo apt-get install -qq -y xxd; }
	@xxd -i $< | sed '/_len = /d' | sed 's/unsigned char/const unsigned char/' > $@
	@cp $@ $(UTILSDIR)/

clean: clean_h
	@rm -rf $(OUTDIR)

clean_h:
	@rm -f  $(UTILSDIR)/$(HELP_ZX0_C) \
			$(OBJDIR)/$(HELP_ZX0_C)* \
			$(LIBDIR)/utils.lib
