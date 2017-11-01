#!/bin/sh

SOLVERNAME=`echo $(basename $0) | cut -d. -f3`
FIELDSIZE=`echo $(basename $0) | cut -d. -f1 | tr -d gf`
MAXVARS=`echo $(basename $0) | cut -d. -f2 | tr -d n`
echo "SOLVERNAME=$SOLVERNAME"
echo "MAXVARS=$MAXVARS"
echo "FIELDSIZE=$FIELDSIZE"

SOLVER="bin/solver_${SOLVERNAME}_n${MAXVARS}_gf${FIELDSIZE}"
INFILE="testdata/gf$FIELDSIZE/${FIELDSIZE}_${MAXVARS}_mq-test0.in"
SOLFILE="testdata/gf$FIELDSIZE/${FIELDSIZE}_${MAXVARS}_mq-test0.degrevlex"
TMPFILE="tests/${FIELDSIZE}_${MAXVARS}_mq-sage0.${SOLVERNAME}.out"
OPTIONS="--solve --mqchallenge --nrthreads 1"

echo "SOLVER=$SOLVER"
echo "OPTIONS=$OPTIONS"
echo "INFILE=$INFILE"
echo "SOLFILE=$SOLFILE"
echo "TMPFILE=$TMPFILE"

echo "====== BUILDING SOLVER ======"
make $SOLVER || exit 1

echo "====== RUNNING SOLVER ======"
if [ ! -x $SOLVER ]; then echo "Solver $SOLVER not found"; exit 1; fi
if [ ! -f $INFILE ]; then echo "Input file $INFILE not found"; exit 1; fi
if [ ! -f $SOLFILE ]; then echo "Solution file $SOLFILE not found"; exit 1; fi
$SOLVER $OPTIONS -i $INFILE -o $TMPFILE || exit 1

echo "====== OUTPUT ======"
cat $TMPFILE
echo "====== VERIFIED SOLUTION ======"
cat $SOLFILE

if [ "$(cat $SOLFILE)" != "$(cat $TMPFILE)" ]; then
	echo "!!! Output does not match solution !!!"
	rm -f $TMPFILE
	exit 1
fi
rm -f $TMPFILE
exit 0
