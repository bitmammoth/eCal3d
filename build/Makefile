#
# This makefile is to build the eGenesis version of the Cal3D animation library
#
# $Date: 2004/09/09 02:16:14 $
# $Id: makefile-eCal3d,v 1.1 2004/09/09 02:16:14 cvsroot Exp $
# $Revision: 1.1 $
#

DISTROOT=../DIST
OSNAME=$(shell uname -s | tr "[:upper:]" "[:lower:]")
ifeq ($(OSNAME),darwin)
  OSPROC=$(shell uname -p)
else
  OSPROC=$(shell uname -m)
endif
OSID=$(OSNAME)-$(OSPROC)

WFLAGS=-Wall -Wno-deprecated
# Warning Flags for non-C++ code
CCWFLAGS=-Wall #-pedantic

ENGINEDIRS=-I.. -I../cal3d -I../andy

# Default build processor type
ifeq ($(OSNAME),darwin)
  CP           =/usr/bin/ditto --rsrc
  OPT          =-O3 -falign-functions=32 -falign-labels=32 -falign-loops=32 -falign-jumps=32 -funroll-loops -fexpensive-optimizations # -fcoalesce-rtti
  MARCH        = -mcpu=G4 -mtune=G5 -mlongcall -mpowerpc-gpopt
  CCOPTS      += -I/usr/X11R6/include
  DYNLDFLAGS   =
endif

ifeq ($(OSNAME),freebsd)
  CP           = /bin/cp
  MARCH        = -march=i686 -mmmx # General Client Distro
  OPT          = -O3 -funroll-loops -falign-functions=32 -fexpensive-optimizations
  CCOPTS      += -pthread -I/usr/X11R6/include
  DYNLDFLAGS   =-Wl,-soname 
endif

ifeq ($(OSNAME),linux)
  CP           = /bin/cp
  OPT          = -O3 -funroll-loops -falign-functions=32 -fexpensive-optimizations
  MARCH        =-march=i686 -mmmx # General Client Distro
  DYNLDFLAGS   =-Wl,-soname 
endif

COPTNOOPTIM=$(WFLAGS) $(CCOPTS) $(SPECIALDEFINE) $(MARCH) -pipe -I. $(LIBINC) $(ENGINEDIRS) 
COPTS    = $(OPT) $(COPTNOOPTIM)

COMPILE=g++ $(COPTS) -c 
CCCOMPILE=gcc $(CCWFLAGS) $(COPTS) -c 
PLAINCC=gcc $(PROFILING) $(CCWFLAGS) 
LINK=g++ -Wall -ggdb $(PROFILING)
LIBTOOL=libtool

CAL3DVER = 0.0.0
ifeq ($(OSNAME),win32)
  CAL3DLIBNAME = libeCal3D.dll
endif

ifeq ($(OSNAME),linux)
  CAL3DLIBNAME = libeCal3D.so.$(CAL3DVER)
endif

ifeq ($(OSNAME),darwin)
  CAL3DLIBNAME = libeCal3D.$(CAL3DVER).dylib
endif

CAL3DHEADERS=\
	../cal3d/buffersource.h \
	../cal3d/calbone.h \
	../cal3d/cal3d.h \
	../cal3d/calcoreanim.h \
	../cal3d/calcorebone.h \
	../cal3d/calcorekey.h \
	../cal3d/calcoremodel.h \
	../cal3d/calcoresub.h \
	../cal3d/calcoretrack.h \
	../cal3d/caldatasource.h \
	../cal3d/calerror.h \
	../cal3d/calglobal.h \
	../cal3d/calloader.h \
	../cal3d/calmatrix.h \
	../cal3d/calmodel.h \
	../cal3d/calplatform.h \
	../cal3d/calquat.h \
	../cal3d/calsaver.h \
	../cal3d/calsub.h \
	../cal3d/calvector.h \
	../cal3d/streamsource.h

CAL3DOBJECTS=\
	cal-buffersource.o \
	cal-calbone.o \
	cal-calcoreanim.o \
	cal-calcorebone.o \
	cal-calcorekey.o \
	cal-calcoremodel.o \
	cal-calcoresub.o \
	cal-calcoretrack.o \
	cal-calerror.o \
	cal-calglobal.o \
	cal-calloader.o \
	cal-calmatrix.o \
	cal-calmodel.o \
	cal-calplatform.o \
	cal-calquat.o \
	cal-calsaver.o \
	cal-calsub.o \
	cal-calvector.o \
	cal-streamsource.o \

CALVIEWOBJECTS=\
	cv-viewer.o \
	cv-main.o \
	cv-tick.o \
	cv-buffersource.o \
	cv-calbone.o \
	cv-calcoreanim.o \
	cv-calcorebone.o \
	cv-calcorekey.o \
	cv-calcoremodel.o \
	cv-calcoresub.o \
	cv-calcoretrack.o \
	cv-calerror.o \
	cv-calglobal.o \
	cv-calloader.o \
	cv-calmatrix.o \
	cv-calmodel.o \
	cv-calplatform.o \
	cv-calquat.o \
	cv-calsaver.o \
	cv-calsub.o \
	cv-calvector.o \
	cv-streamsource.o \

CALVIEWHEADERS=\
	../calview/cv-global.h \
	../calview/cv-tick.h \
	../calview/cv-viewer.h

###############################################################
#
# Control rules
#
###############################################################

all: libeCal3D.so.$(CAL3DVER) calview

clean:
	$(RM) TAGS #*# gmon.out *~ core *.pdb
	$(RM) libeCal3D.a $(CAL3DLIBNAME) eCal3d.tar.gz eCal3d.tar.bz2
	$(RM) calview *.o

####################################################################
#
# Cal3D objects for use in the engine.
#
####################################################################

eCal3d.tar.gz eCal3d.tar.bz2: $(CAL3DOBJECTS) ../makefiles/Makefile-unix ../makefiles/ecal3d.txt
	$(RM) -r eCal3d
	$(RM) eCal3d.tar.gz eCal3d.tar
	mkdir -p eCal3d/cal3d
	mkdir -p eCal3d/calexp
	mkdir -p eCal3d/calview
	mkdir -p eCal3d/andy
	mkdir -p eCal3d/maxsdk3
	mkdir -p eCal3d/maxsdk4
	mkdir -p eCal3d/maxsdk5
	mkdir -p eCal3d/maxsdk6
	$(CP) ../cal3d/cal* eCal3d/cal3d
	$(CP) ../calexp/cx-* eCal3d/calexp
	$(CP) ../calview/cv-* eCal3d/calview
	$(CP) ../andy/caluserdata.h eCal3d/andy
	$(CP) ../makefiles/Makefile-unix eCal3d/build/Makefile
	$(CP) ../makefiles/ecal3d.txt eCal3d/README
	tar czf eCal3d.tar.gz ./eCal3d
	tar cjf eCal3d.tar.bz2 ./eCal3d


libeCal3D.$(CAL3DVER).dylib: $(CAL3DOBJECTS)
	$(LIBTOOL) -dynamic -install_name libeCal3D.$(CAL3DVER).dylib -flat_namespace -undefined suppress -o libeCal3D.$(CAL3DVER).dylib $(CAL3DOBJECTS)

libeCal3D.so.$(CAL3DVER): $(CAL3DOBJECTS)
	$(PLAINCC) -shared -o $@ $(DYNLDFLAGS) -Wl,$@ $(CAL3DOBJECTS) -lgcc
	strip -x $@

libeCal3D.a: $(CAL3DOBJECTS)
	$(RM) $@
	/usr/bin/ar cr $@ $(CAL3DOBJECTS)
	ranlib $@

cal-buffersource.o : ../cal3d/buffersource.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-buffersource.o ../cal3d/buffersource.cpp

cal-calbone.o : ../cal3d/calbone.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calbone.o ../cal3d/calbone.cpp

cal-calcoreanim.o : ../cal3d/calcoreanim.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calcoreanim.o ../cal3d/calcoreanim.cpp

cal-calcorebone.o : ../cal3d/calcorebone.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calcorebone.o ../cal3d/calcorebone.cpp

cal-calcorekey.o : ../cal3d/calcorekey.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calcorekey.o ../cal3d/calcorekey.cpp

cal-calcoremodel.o : ../cal3d/calcoremodel.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calcoremodel.o ../cal3d/calcoremodel.cpp

cal-calcoresub.o : ../cal3d/calcoresub.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calcoresub.o ../cal3d/calcoresub.cpp

cal-calcoretrack.o : ../cal3d/calcoretrack.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calcoretrack.o ../cal3d/calcoretrack.cpp

cal-calerror.o : ../cal3d/calerror.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calerror.o ../cal3d/calerror.cpp

cal-calglobal.o : ../cal3d/calglobal.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calglobal.o ../cal3d/calglobal.cpp

cal-calloader.o : ../cal3d/calloader.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calloader.o ../cal3d/calloader.cpp

cal-calmatrix.o : ../cal3d/calmatrix.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calmatrix.o ../cal3d/calmatrix.cpp

cal-calmodel.o : ../cal3d/calmodel.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calmodel.o ../cal3d/calmodel.cpp

cal-calplatform.o : ../cal3d/calplatform.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calplatform.o ../cal3d/calplatform.cpp

cal-calquat.o : ../cal3d/calquat.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calquat.o ../cal3d/calquat.cpp

cal-calsaver.o : ../cal3d/calsaver.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calsaver.o ../cal3d/calsaver.cpp

cal-calsub.o : ../cal3d/calsub.cpp $(CAL3DHEADERS) ../andy/caluserdata.h ../cal3d/calphysop.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calsub.o ../cal3d/calsub.cpp

cal-calvector.o : ../cal3d/calvector.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-calvector.o ../cal3d/calvector.cpp

cal-streamsource.o : ../cal3d/streamsource.cpp $(CAL3DHEADERS) ../andy/caluserdata.h
	$(COMPILE) -DCALUSERDATA="<caluserdata.h>" -o cal-streamsource.o ../cal3d/streamsource.cpp

####################################################################
#
# The Cal3D miniviewer, calview.exe
#
####################################################################
 
calview: $(CALVIEWOBJECTS)
	$(LINK) $(CALVIEWOBJECTS) -lglut -o calview
 
cv-viewer.o : ../calview/cv-viewer.cpp $(CALVIEWHEADERS) $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-viewer.o ../calview/cv-viewer.cpp

cv-tick.o : ../calview/cv-tick.cpp $(CALVIEWHEADERS) $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-tick.o ../calview/cv-tick.cpp

cv-main.o : ../calview/cv-main.cpp $(CALVIEWHEADERS) $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-main.o ../calview/cv-main.cpp

cv-buffersource.o : ../cal3d/buffersource.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-buffersource.o ../cal3d/buffersource.cpp

cv-calbone.o : ../cal3d/calbone.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calbone.o ../cal3d/calbone.cpp

cv-calcoreanim.o : ../cal3d/calcoreanim.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calcoreanim.o ../cal3d/calcoreanim.cpp

cv-calcorebone.o : ../cal3d/calcorebone.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calcorebone.o ../cal3d/calcorebone.cpp

cv-calcorekey.o : ../cal3d/calcorekey.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calcorekey.o ../cal3d/calcorekey.cpp

cv-calcoremodel.o : ../cal3d/calcoremodel.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calcoremodel.o ../cal3d/calcoremodel.cpp

cv-calcoresub.o : ../cal3d/calcoresub.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calcoresub.o ../cal3d/calcoresub.cpp

cv-calcoretrack.o : ../cal3d/calcoretrack.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calcoretrack.o ../cal3d/calcoretrack.cpp

cv-calerror.o : ../cal3d/calerror.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calerror.o ../cal3d/calerror.cpp

cv-calglobal.o : ../cal3d/calglobal.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calglobal.o ../cal3d/calglobal.cpp

cv-calloader.o : ../cal3d/calloader.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calloader.o ../cal3d/calloader.cpp

cv-calmatrix.o : ../cal3d/calmatrix.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calmatrix.o ../cal3d/calmatrix.cpp

cv-calmodel.o : ../cal3d/calmodel.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calmodel.o ../cal3d/calmodel.cpp

cv-calplatform.o : ../cal3d/calplatform.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calplatform.o ../cal3d/calplatform.cpp

cv-calquat.o : ../cal3d/calquat.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calquat.o ../cal3d/calquat.cpp

cv-calsaver.o : ../cal3d/calsaver.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calsaver.o ../cal3d/calsaver.cpp

cv-calsub.o : ../cal3d/calsub.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h ../cal3d/calphysop.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calsub.o ../cal3d/calsub.cpp

cv-calvector.o : ../cal3d/calvector.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-calvector.o ../cal3d/calvector.cpp

cv-streamsource.o : ../cal3d/streamsource.cpp $(CAL3DHEADERS) ../calview/cv-userdata.h
	$(COMPILE) -DCALUSERDATA="<cv-userdata.h>" -o cv-streamsource.o ../cal3d/streamsource.cpp

################################################
# ChangeLogs
################################################
ChangeLog:
	cvs2cl --show-dead --prune

################################################
# Generic Build Rules
################################################

%.o: %.cpp
	$(COMPILE) $< -o $@

%.o: %.c
	$(CCCOMPILE) $< -o $@
