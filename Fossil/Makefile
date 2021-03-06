#!/usr/bin/make
#
# This is the top-level makefile for Fossil when the build is occurring
# on a unix platform.  This works out-of-the-box on most unix platforms.
# But you are free to vary some of the definitions if desired.
#
#### The toplevel directory of the source tree.  Fossil can be built
#    in a directory that is separate from the source tree.  Just change
#    the following to point from the build directory to the src/ folder.
#
SRCDIR = ./src

#### The directory into which object code files should be written.
#    Having a "./" prefix in the value of this variable breaks our use of the
#    "makeheaders" tool when running make on the MinGW platform, apparently
#    due to some command line argument manipulation performed automatically
#    by the shell.
#
#
OBJDIR = bld

#### C Compiler and options for use in building executables that
#    will run on the platform that is doing the build.  This is used
#    to compile code-generator programs as part of the build process.
#    See TCC below for the C compiler for building the finished binary.
#
BCC = cc

#### The suffix to add to final executable file.  When cross-compiling
#    to windows, make this ".exe".  Otherwise leave it blank.
#
E = 

TCC = cc

#### Tcl shell for use in running the fossil testsuite.  If you do not
#    care about testing the end result, this can be blank.
#
TCLSH = tclsh

LIB =	   -lz -lssl -lcrypto -ldl
TCC +=	   -g -O2 -DHAVE_AUTOCONFIG_H
INSTALLDIR = $(DESTDIR)/usr/local/bin
USE_SYSTEM_SQLITE = 0
FOSSIL_ENABLE_TCL = @FOSSIL_ENABLE_TCL@
FOSSIL_ENABLE_TCL_STUBS = @FOSSIL_ENABLE_TCL_STUBS@
FOSSIL_ENABLE_TCL_PRIVATE_STUBS = @FOSSIL_ENABLE_TCL_PRIVATE_STUBS@

include $(SRCDIR)/main.mk

distclean: clean
	rm -f autoconfig.h config.log Makefile
