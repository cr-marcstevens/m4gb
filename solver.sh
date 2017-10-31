#!/bin/bash

NAME="m4gb"

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

if [ -z "$FIELDSIZE" ]; then
    FIELDSIZE=`grep -i "[$].*fieldsize.*[0-9]\+" $INPUTFILENAME | grep -o "[0-9]\+" | head -n 1`
fi
if [ -z "$FIELDSIZE" ]; then
    echo "The field size could not be detected: use -f #"
    exit 1
fi
if [ -z "$MAXVARS" ]; then
    MAXVARS=`grep -i "[$].*vars.*[0-9]\+" $INPUTFILENAME | grep -o "[0-9]\+" | head -n 1`
fi
if [ -z "$MAXVARS" ]; then
    echo "The number of variables could not be detected: use -n #"
    exit 1
fi

SOLVERBIN=bin/solver_${NAME}_n${MAXVARS}_gf${FIELDSIZE}

echo "Inputfile: $INPUTFILENAME"
echo "Fieldsize: $FIELDSIZE"
echo "Variables: $MAXVARS"
echo "Options  : $OPTIONS"
echo "Solver   : $SOLVERBIN"

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
