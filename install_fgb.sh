#!/bin/bash

mkdir fgb
cd fgb

echo "Attempting to install FGb library:"
echo "http://www-polsys.lip6.fr/~jcf/FGb/FGb/index.html"
echo ""

if [ -z "$(uname -a | grep -i 'x86_64')" ]; then
	echo "Error: FGb library only available for x86_64"
	exit 1
fi
if [ -z "$(uname -a | grep -i 'linux')" ]; then
	echo "Error: only linux supported, try manual installation"
	exit 1
fi

BASEURL="http://www-polsys.lip6.fr/~jcf/FGb/C"
if [ ! -f .tmpfile ]; then
	if wget -nv "$BASEURL/index.html" -O .tmpfile ; then :; else
		echo "Error: could not retrieve information from website" 
		exit 1
	fi
fi

LATESTFILE=$(grep -o -i '"[^"]*FGb[^"]*.x64.tar.gz"' .tmpfile | tr -d \" | head -n1)
if [ -z $LATESTFILE ]; then
	echo "Error: could not retrieve latest version from website"
	exit 1
fi
FILENAME=$(basename $LATESTFILE)
LATESTFILE="$BASEURL/$LATESTFILE"

if [ ! -f $FILENAME ]; then
	if wget -nv "$LATESTFILE" ; then :; else
		echo "Error: could not retrieve file from website"
		exit 1
	fi
fi

if tar -xzvf $FILENAME ; then :; else
	echo "Error: archive corrupted"
	exit 1
fi

if [ ! -d call_FGb ]; then
	echo "directory fgb/callFGb does not exist. Archive corrupt?"
	exit 1
fi

cd call_FGb

echo "=== Testing compilation of fgb programs ==="
make || exit 1

## FGb is quite problematic: call_fgb.h defines C functions, but requires C++ linking, it also defines global variables
## So we add a wrapper header file that adds 'extern "C"' when approriate and prevents some defines
## And we build an extra helper C file that also has to be linked with

echo "=== Generating wrapper header fgh.h ==="
FGBDIR=nv/maple/C
cat <<EOF > $FGBDIR/fgb.h
#ifndef FGB_H
#define FGB_H

#define LIBMODE 1
#define CALL_FGB_DO_NOT_DEFINE

#ifdef __cplusplus

extern "C" {
#include "call_fgb.h"
}

#else

#include "call_fgb.h"

#endif

#endif
EOF

grep -H . $FGBDIR/fgb.h

echo "=== Generating wrapper library libfgbdef.a ==="
echo '#include "call_fgb.h"' > $FGBDIR/fgbdef.c
grep -H . $FGBDIR/fgbdef.c

CC="${CC:-gcc}"
CFLAGS="${CFLAGS:--m64}"
AR="${AR:-ar}"

COMPILE="$CC -I$FGBDIR -I$FGBDIR/../../int -I$FGBDIR/../../protocol -o $FGBDIR/fgbdef.o -c $FGBDIR/fgbdef.c"
ARLIB="$AR rcs $FGBDIR/x64/libfgbdef.a $FGBDIR/fgbdef.o"
if echo "$COMPILE" && $COMPILE ; then :; else
	echo "Could not compile fgbdef.c"
	exit 1
fi
if echo "$ARLIB" && $ARLIB ; then :; else
	echo "Could not make fgbdef.a"
	exit 1
fi
echo "=== Finished installing fgb ==="
