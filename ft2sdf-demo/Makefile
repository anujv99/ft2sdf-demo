all: exes


####################################################################
#
# The `space' variable is used to avoid trailing spaces in defining
# the `T' variable later.
#
empty :=
space := $(empty) $(empty)


####################################################################
#
# TOP_DIR is the directory where the main FreeType source is found,
# as well as the `config.mk' file.
#
# TOP_DIR_2 is the directory containing the top of the demonstration
# programs directory.
#
# OBJ_DIR gives the objects directory of the FreeType library.
#
# IF DEVEL_DIR is set, do a development build (i.e., use development
# versions of the FreeType configuration header files `ft2build.h'
# and `ftoption.h' given in this directory).
#
TOP_DIR   ?= ../freetype2
TOP_DIR_2 ?= .
OBJ_DIR   ?= $(TOP_DIR)/objs


######################################################################
#
# CONFIG_MK points to the current `config.mk' to use.  It is defined
# by default as $(TOP_DIR)/config.mk.
#
ifndef CONFIG_MK
  PROJECT   := freetype
  CONFIG_MK := $(TOP_DIR)/config.mk
endif


######################################################################
#
# MODULES_CFG points to the current `modules.cfg' to use.  It is defined
# by default as $(TOP_DIR)/modules.cfg.
#
MODULES_CFG ?= $(TOP_DIR)/modules.cfg

ifeq ($(wildcard $(MODULES_CFG)),)
  no_modules_cfg := 1
endif


####################################################################
#
# Check that we have a working `config.mk' in the above directory.
# Otherwise issue a warning message and stop.
#
ifeq ($(wildcard $(CONFIG_MK)),)
  no_config_mk := 1
endif

ifdef no_config_mk

  exes:
	  $(info Please compile the library before the demo programs!)
  clean distclean:
	  $(info I need a path to FreeType 2's `config.mk' to do that!)
	  $(info Set the `TOP_DIR' variable to the correct value.)

else

  ####################################################################
  #
  # Good, now include `config.mk' in order to know how to build
  # object files from sources, as well as other things (compiler
  # flags).
  #
  include $(CONFIG_MK)

  ifndef no_modules_cfg
    include $(MODULES_CFG)
  endif

  have_makefile := $(strip $(wildcard Makefile))

  ifeq ($(PLATFORM),unix)
    ifdef DEVEL_DIR
      PLATFORM := unixdev
    endif
  endif


  ####################################################################
  #
  # Define a few important variables now.
  #
  ifeq ($(PLATFORM),unix)
    # without absolute paths libtool fails
    TOP_DIR   := $(shell cd $(TOP_DIR); pwd)
    TOP_DIR_2 := $(shell cd $(TOP_DIR_2); pwd)
    ifneq ($(have_makefile),)
      BIN_DIR_2 ?= $(TOP_DIR_2)/bin
      OBJ_DIR_2 ?= $(TOP_DIR_2)/objs
    else
      BIN_DIR_2 ?= .
      OBJ_DIR_2 ?= .
    endif
  else
    ifneq ($(have_makefile),)
      BIN_DIR_2 ?= bin
      OBJ_DIR_2 ?= objs
    else
      BIN_DIR_2 ?= .
      OBJ_DIR_2 ?= .
    endif
  endif

  GRAPH_DIR := $(TOP_DIR_2)/graph

  ifeq ($(TOP_DIR),..)
    SRC_DIR := src
  else
    SRC_DIR := $(TOP_DIR_2)/src
  endif

  FT_INCLUDES := $(OBJ_BUILD) \
                 $(DEVEL_DIR) \
                 $(TOP_DIR)/include \
                 $(SRC_DIR)

  COMPILE = $(CC) $(ANSIFLAGS) \
                  $(INCLUDES:%=$I%) \
                  $(CPPFLAGS) \
                  $(CFLAGS)

  # Enable C99 for gcc to avoid warnings.
  # Note that clang++ aborts with an error if we use `-std=C99',
  # so check for `++' in $(CC) also.
  ifneq ($(findstring -pedantic,$(COMPILE)),)
    ifeq ($(findstring ++,$(CC)),)
      COMPILE += -std=c99
    endif
  endif

  FTLIB := $(LIB_DIR)/$(LIBRARY).$A

  # `-lm' is required to compile on some Unix systems.
  #
  ifeq ($(PLATFORM),unix)
    MATH := -lm
  endif

  ifeq ($(PLATFORM),unixdev)
    MATH := -lm
  endif

  # The default variables used to link the executables.  These can
  # be redefined for platform-specific stuff.
  #
  # The first token of LINK_ITEMS must be the executable.
  #
  LINK_ITEMS = $T$(subst /,$(COMPILER_SEP),$@ $<)

  ifeq ($(PLATFORM),unix)
    override CC = $(CCraw)
    LINK_CMD    = $(subst /,$(SEP),$(OBJ_BUILD)/libtool) \
                  --mode=link $(CC) \
                  $(subst /,$(COMPILER_SEP),$(LDFLAGS))
    LINK_LIBS   = $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE)) $(LIB_CLOCK_GETTIME)
  else
    LINK_CMD = $(CC) $(subst /,$(COMPILER_SEP),$(LDFLAGS))
    ifeq ($(PLATFORM),unixdev)
      LINK_LIBS := $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE)) -lm -lrt -lz -lbz2
      LINK_LIBS += $(shell pkg-config --libs libpng)
      LINK_LIBS += $(shell pkg-config --libs harfbuzz)
      LINK_LIBS += $(shell pkg-config --libs libbrotlidec)
    else
      LINK_LIBS = $(subst /,$(COMPILER_SEP),$(FTLIB) $(EFENCE))
    endif
  endif

  LINK        = $(LINK_CMD) \
                $(LINK_ITEMS) \
                $(LINK_LIBS)
  LINK_COMMON = $(LINK_CMD) \
                $(LINK_ITEMS) $(subst /,$(COMPILER_SEP),$(COMMON_OBJ)) \
                $(LINK_LIBS)
  LINK_GRAPH  = $(LINK_COMMON) $(subst /,$(COMPILER_SEP),$(GRAPH_LIB)) \
                $(GRAPH_LINK) $(MATH)
  LINK_NEW    = $(LINK_CMD) \
                $(LINK_ITEMS) $(subst /,$(COMPILER_SEP),$(COMMON_OBJ) \
                                        $(FTCOMMON_OBJ)) \
                $(LINK_LIBS) $(subst /,$(COMPILER_SEP),$(GRAPH_LIB)) \
                $(GRAPH_LINK) $(MATH)

  .PHONY: exes clean distclean


  ###################################################################
  #
  # Include the rules needed to compile the graphics sub-system.
  # This will also select which graphics driver to compile to the
  # sub-system.
  #
  include $(GRAPH_DIR)/rules.mk


  ####################################################################
  #
  # Detect DOS-like platforms, currently DOS, Win 3.1, Win32 & OS/2.
  #
  ifneq ($(findstring $(PLATFORM),os2 win16 win32 dos),)
    DOSLIKE := 1
  endif


  ###################################################################
  #
  # Clean-up rules.  Because the `del' command on DOS-like platforms
  # cannot take a long list of arguments, we simply erase the directory
  # contents.
  #
  ifdef DOSLIKE

    clean_demo:
	    -del objs\*.$(SO) 2> nul
	    -del $(subst /,\,$(TOP_DIR_2)/src/*.bak) 2> nul

    distclean_demo: clean_demo
	    -del objs\*.lib 2> nul
	    -del bin\*.exe 2> nul

  else

    clean_demo:
	    -$(DELETE) $(subst /,$(SEP),$(OBJ_DIR_2)/*.$(SO) $(OBJ_DIR_2)/*.$(O))
	    -$(DELETE) $(subst /,$(SEP),$(OBJ_DIR_2)/*.$(SA) $(OBJ_DIR_2)/*.$(A))
	    -$(DELETE) $(subst /,$(SEP),$(OBJ_DIR_2)/.libs/*)
	    -$(DELETE) $(subst /,$(SEP),$(SRC_DIR)/*.bak graph/*.bak)
	    -$(DELETE) $(subst /,$(SEP),$(SRC_DIR)/*~ graph/*~)

    distclean_demo: clean_demo
	    -$(DELETE) $(subst /,$(SEP),$(EXES:%=$(BIN_DIR_2)/%$E))
	    -$(DELETE) $(subst /,$(SEP),$(GRAPH_LIB))
    ifeq ($(PLATFORM),unix)
	      -$(DELETE) $(BIN_DIR_2)/.libs/*
	      -$(DELDIR) $(BIN_DIR_2)/.libs
    endif

  endif

  clean:     clean_demo
  distclean: distclean_demo


  ####################################################################
  #
  # Compute the executable suffix to use, and put it in `E'.
  # It is ".exe" on DOS-ish platforms, and nothing otherwise.
  #
  ifdef DOSLIKE
    E := .exe
  else
    E :=
  endif


  ####################################################################
  #
  # POSIX TERMIOS: Do not define if you use OLD U*ix like 4.2BSD.
  #
  ifeq ($(PLATFORM),unix)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
  endif

  ifeq ($(PLATFORM),unixdev)
    EXTRAFLAGS = $DUNIX $DHAVE_POSIX_TERMIOS
  endif


  ###################################################################
  #
  # The list of demonstration programs to build.
  #
  # Note that ttdebug only works if the FreeType's `truetype' driver has
  # been compiled with TT_CONFIG_OPTION_BYTECODE_INTERPRETER defined.
  #

  # Comment out the next line if you don't have a graphics subsystem.
  EXES += ftsdf

  exes: $(EXES:%=$(BIN_DIR_2)/%$E)


  INCLUDES := $(subst /,$(COMPILER_SEP),$(FT_INCLUDES))


  # generic rule
  $(OBJ_DIR_2)/%.$(SO): $(SRC_DIR)/%.c $(FTLIB)
	  $(COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


  ####################################################################
  #
  # Rules for compiling object files for text-only demos.
  #
  $(OBJ_DIR_2)/common.$(SO): $(SRC_DIR)/common.c
  $(OBJ_DIR_2)/output.$(SO): $(SRC_DIR)/output.c
  $(OBJ_DIR_2)/mlgetopt.$(SO): $(SRC_DIR)/mlgetopt.c
  COMMON_OBJ := $(OBJ_DIR_2)/common.$(SO) \
                $(OBJ_DIR_2)/output.$(SO) \
                $(OBJ_DIR_2)/mlgetopt.$(SO)


  FTCOMMON_OBJ := $(OBJ_DIR_2)/ftcommon.$(SO)
  $(FTCOMMON_OBJ): $(SRC_DIR)/ftcommon.c $(SRC_DIR)/ftcommon.h
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) \
                     $T$(subst /,$(COMPILER_SEP),$@ $<)

#  $(OBJ_DIR_2)/ftsbit.$(SO): $(SRC_DIR)/ftsbit.c
#	  $(COMPILE) $T$(subst /,$(COMPILER_SEP),$@ $<)


  # We simplify the dependencies on the graphics library by using
  # $(GRAPH_LIB) directly.

  $(OBJ_DIR_2)/ftsdf.$(SO): $(SRC_DIR)/ftsdf.c \
                               $(SRC_DIR)/ftcommon.h \
                               $(GRAPH_LIB)
	  $(COMPILE) $(GRAPH_INCLUDES:%=$I%) \
                     $T$(subst /,$(COMPILER_SEP),$@ $<)

  ####################################################################
  #
  # Rules used to link the executables.  Note that they could be
  # overridden by system-specific things.
  #

  $(BIN_DIR_2)/ftsdf$E: $(OBJ_DIR_2)/ftsdf.$(SO) $(FTLIB) \
                           $(GRAPH_LIB) $(COMMON_OBJ) $(FTCOMMON_OBJ)
	  $(LINK_NEW)


endif


# This target builds the tarballs.
#
# Not to be run by a normal user -- there are no attempts to make it
# generic.

# we check for `dist', not `distclean'
ifneq ($(findstring distx,$(MAKECMDGOALS)x),)
  FT_H := ../freetype2/include/freetype/freetype.h

  major := $(shell sed -n 's/.*FREETYPE_MAJOR[^0-9]*\([0-9]\+\)/\1/p' < $(FT_H))
  minor := $(shell sed -n 's/.*FREETYPE_MINOR[^0-9]*\([0-9]\+\)/\1/p' < $(FT_H))
  patch := $(shell sed -n 's/.*FREETYPE_PATCH[^0-9]*\([0-9]\+\)/\1/p' < $(FT_H))

#  ifneq ($(findstring x0x,x$(patch)x),)
#    version := $(major).$(minor)
#    winversion := $(major)$(minor)
#  else
    version := $(major).$(minor).$(patch)
    winversion := $(major)$(minor)$(patch)
#  endif
endif

dist:
	-rm -rf tmp
	rm -f ft2demos-$(version).tar.gz
	rm -f ft2demos-$(version).tar.xz
	rm -f ftdmo$(winversion).zip

	for d in `find . -wholename '*/.git' -prune \
	                 -o -type f \
	                 -o -print` ; do \
	  mkdir -p tmp/$$d ; \
	done ;

	currdir=`pwd` ; \
	for f in `find . -wholename '*/.git' -prune \
	                 -o -name .gitignore \
	                 -o -name .mailmap \
	                 -o -type d \
	                 -o -print` ; do \
	  ln -s $$currdir/$$f tmp/$$f ; \
	done

	cd tmp ; \
	$(MAKE) distclean

	mv tmp ft2demos-$(version)

	tar -H ustar -chf - ft2demos-$(version) \
	| gzip -c > ft2demos-$(version).tar.gz
	tar -H ustar -chf - ft2demos-$(version) \
	| xz -c > ft2demos-$(version).tar.xz

	@# Use CR/LF for zip files.
	zip -lr ftdmo$(winversion).zip ft2demos-$(version)

	rm -fr ft2demos-$(version)

# EOF
