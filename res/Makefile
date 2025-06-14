.PHONY: all clean clean_h

ROOTDIR = ..
BINDIR = $(ROOTDIR)/bin
INCDIR = $(ROOTDIR)/includes
UTILSDIR = $(ROOTDIR)/src/libs
OBJDIR = $(ROOTDIR)/obj
LIBDIR = $(ROOTDIR)/lib
OUTDIR = ./out
OUT_GUARD=@mkdir -p $(OUTDIR)
XXD_GUARD=@which xxd > /dev/null || { echo "Installing xxd..."; sudo apt-get update -qq && sudo apt-get install -qq -y xxd; }

HELP_TXT = help.txt
HELP_BIN = help.bin
HELP_ZX0 = help.bin.zx0
HELP_ZX0_C := utils_help_zx0.c

HELPCMD_TXT = help_cmd.txt
HELPCMD_BIN = help_cmd.bin
HELPCMD_ZX0 = help_cmd.bin.zx0
HELPCMD_ZX0_C := utils_help_cmd_zx0.c


all: $(OUTDIR)/$(HELP_ZX0_C) $(OUTDIR)/$(HELPCMD_ZX0_C)


# =========================================================
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
	@$(XXD_GUARD)
	@xxd -i $< | sed '/_len = /d' | sed 's/unsigned char/const unsigned char/' > $@
	@cp $@ $(UTILSDIR)/


# =========================================================
$(OUTDIR)/$(HELPCMD_BIN): $(HELPCMD_TXT)
	@$(OUT_GUARD)
	@$(XXD_GUARD)
	@cp $< $@
	@echo -n "00" | xxd -r -p >> $@

$(OUTDIR)/$(HELPCMD_ZX0): $(OUTDIR)/$(HELPCMD_BIN)
	@$(OUT_GUARD)
	@$(BINDIR)/zx0 -f $< $@

$(OUTDIR)/$(HELPCMD_ZX0_C): $(OUTDIR)/$(HELPCMD_ZX0)
	@$(XXD_GUARD)
	@xxd -i $< | sed '/_len = /d' | sed 's/unsigned char/#pragma codeseg DISPOSABLE\nconst unsigned char/' > $@
	@cp $@ $(UTILSDIR)/


# =========================================================
clean: clean_h
	@rm -rf $(OUTDIR)

clean_h:
	@rm -f  $(UTILSDIR)/$(HELP_ZX0_C) \
			$(UTILSDIR)/$(HELPCMD_ZX0_C) \
			$(OBJDIR)/$(HELP_ZX0_C)* \
			$(OBJDIR)/$(HELPCMD_ZX0_C)* \
			$(LIBDIR)/utils.lib
