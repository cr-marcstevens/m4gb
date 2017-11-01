#!/bin/bash

NAME="m4gb"
INFORMAT=""

display_help()
{
    echo "Solve overdefined polyomial equation system over a finite field"
    echo "Usage : $0 [-s m4gb|openf4|fgb] [-f #] [-n #] [-v #] [-t #] <inputfile> [<outputfile>]"
    echo "Arguments"
    echo "  -h                  : display this help"
    echo "  -s m4gb|openf4|fgb  : choose solver backend: m4gb (default), openf4, fgb"
    echo "  -f #                : force the size of the finite field (default=auto)"
    echo "  -n #                : force the number of variables (default=auto)"
    echo "  -v #                : set verbosity 0-3 (default: 3)"
    echo "  -t #                : set number of threads (default: auto)"
    echo "  <inputfile>         : the input file"
    echo "  <outputfile>        : the output file"
    exit 0
}

OPTIONS=""
while getopts :f:n:s:t:v:h option
do
    case "$option" in
        f) FIELDSIZE=$OPTARG ;;
        h) display_help ;;
        n) MAXVARS=$OPTARG ;;
        s) NAME=$OPTARG ;;
        t) OPTIONS="$OPTIONS --nrthreads $OPTARG" ;;
        v) OPTIONS="$OPTIONS --loglevel $OPTARG" ;;
        \?)
            echo "Invalid options: -$OPTARG" >&2
            exit 1
            ;;
        :)
            echo "Option -$OPTARG requires an argument" >&2
            exit 1;
            ;;
    esac
done
shift $((OPTIND-1))

INPUTFILENAME="$1"
OUTPUTFILENAME="$2"

echo "Inputfile: $INPUTFILENAME"
if [ ! -f "$INPUTFILENAME" ] || [ ! -r "$INPUTFILENAME" ]; then
    echo "Cannot read inputfile: $INPUTFILENAME"
    exit 1
fi

# autodetect FIELDSIZE: first for 'default' format, then for 'mqchallenge' format
if [ -z "$FIELDSIZE" ]; then
    FIELDSIZE=`grep -i "[$].*fieldsize.*[0-9]\+" $INPUTFILENAME | grep -o "[0-9]\+" | head -n 1`
fi
if [ -z "$FIELDSIZE" ]; then
    #Galois Field : GF(251)
    #Galois Field : GF(2)[x] / x^8 + x^4 + x^3 + x^2 + 1
    GFTEXT=`grep -i "Galois Field.*:" $INPUTFILENAME | cut -d: -f2 | head -n1`
    if [ ! -z "$GFTEXT" ]; then
        FIELDCHAR=`echo $GFTEXT | cut -d'(' -f2 | cut -d')' -f1`
        FIELDEXT=`echo $GFTEXT | grep -o "\\^[1-9]\+" | tr -d '\\^' | head -n1`
	FIELDSIZE=$FIELDCHAR
        if [ ! -z "$FIELDEXT" ]; then
	    for ((i=1;i<FIELDEXT; ++i)); do
		FIELDSIZE=$(($FIELDSIZE * $FIELDCHAR))
	    done
	fi
    fi
fi
if [ -z "$FIELDSIZE" ]; then
    echo "The field size could not be detected: use -f #"
    exit 1
fi
echo "Fieldsize: $FIELDSIZE"

# autodetect MAXVARS: first for 'default' format, then for 'mqchallenge' format
if [ -z "$MAXVARS" ]; then
    MAXVARS=`grep -i "[$].*vars.*[0-9]\+" $INPUTFILENAME | grep -o "[0-9]\+" | head -n 1`
fi
if [ -z "$MAXVARS" ]; then
    MAXVARS=`grep -i "Number of variables.*:.*[0-9]\+" $INPUTFILENAME | grep -o "[0-9]\+" | head -n 1`
fi
if [ -z "$MAXVARS" ]; then
    echo "The number of variables could not be detected: use -n #"
    exit 1
fi
echo "Variables: $MAXVARS"
if [ ! -z "$OUTPUTFILENAME" ]; then
    OPTIONS="$OPTIONS -o $OUTPUTFILENAME"
fi

# autodetect 'mqchallenge' format
if grep -i "^[^#]*Galois Field.*:.*[0-9]" "$INPUTFILENAME" >/dev/null && grep -i "^[^#]*Number of variables.*:.*[0-9]" "$INPUTFILENAME" >/dev/null; then
    OPTIONS="$OPTIONS --mqchallenge"
fi

SOLVERBIN=bin/solver_${NAME}_n${MAXVARS}_gf${FIELDSIZE}

echo "Solver   : $SOLVERBIN"
echo "Options  : $OPTIONS"

re='^[0-9]+$'
if ! [[ $FIELDSIZE =~ $re ]] || [ $FIELDSIZE -lt 2 ]; then
    echo "The field size (-f) must be a positive integer >= 2"
    exit 1
fi
if ! [[ $MAXVARS =~ $re ]] || [ $MAXVARS -lt 1 ]; then
    echo "The number of variables (-n) must be a positive integer"
    exit 1
fi


echo ""
echo "=== Building solver binary ==="
make ${SOLVERBIN} || exit 1

echo ""
echo "=== Running solver binary ==="

echo ${SOLVERBIN} ${OPTIONS} -s -i ${INPUTFILENAME}
/usr/bin/time ${SOLVERBIN} ${OPTIONS} -s -i ${INPUTFILENAME}
