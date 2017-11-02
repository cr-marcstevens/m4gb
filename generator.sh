#!/bin/bash

NAME="generator"

display_help()
{
    echo "Random overdefined polyomial equations' generator over a finite field"
    echo "Usage : $0 -f <fieldsize> -n <#vars> -m <#equations> [-d <maxdegree>] [-r] [-s <seed>] [-o <basename>]"
    echo "Arguments"
    echo "  -h                    : display this help"
    echo "  -f <fieldsize>        : set the size of the finite field"
    echo "  -n <#vars>            : set the number of variables"
    echo "  -m <#equations>       : set the number of equations"
    echo "  -d <maxdegree>        : (optional) set the maximum degree (default=2)"
    echo "  -s <seed>             : (optional) set pseudo random number generator seed"
    echo "  -o <output basename>  : (optional) set the basename for output file (file ext=.in)"
    echo "  -r                    : (optional) force a random root of the system (file ext=.ans)"
    exit 0
}
if [ -z "$1" ]; then
	display_help
fi

OPTIONS=""
while getopts :f:n:m:d:o:s:rh option
do
    case "$option" in
        h) display_help ;;
        f) FIELDSIZE=$OPTARG ;;
        n) N=$OPTARG ;;
        m) M=$OPTARG ;;
        d) DEG=$OPTARG ;;
        r) OPTIONS="--forceroot $OPTIONS" ;;
        s) OPTIONS="--seed $OPTARG $OPTIONS" ;;
        o) OUTPUTFILENAME=$OPTARG ;;
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

if [ -z "$FIELDSIZE" ]; then
    echo "The field size is not set (use -f)"
    exit 1
fi
if [ -z "$M" ]; then
    echo "The number of equations is not set (use -m)"
    exit 1
fi
if [ -z "$N" ]; then
    echo "The number of variables is not set (use -n)"
    exit 1
fi
if [ -z "$DEG" ]; then
    DEG=2 #default degree is quadratic
fi
if [ -z "$OUTPUTFILENAME" ]; then
    OUTPUTFILENAME=${FIELDSIZE}_n${N}_m${M}
fi

GENERATOR=bin/${NAME}_n${N}_d${DEG}_gf${FIELDSIZE}

echo "Size of the finite field : $FIELDSIZE"
echo "Number of variables      : $N"
echo "Number of equations      : $M"
echo "Maximum degree           : $DEG"
echo "Output file name         : $OUTPUTFILENAME.in"
echo "(Answer file name)       : $OUTPUTFILENAME.ans"
echo ""
echo "Generator binary         : $GENERATOR"
echo "Options                  : $OPTIONS"

re='^[0-9]+$'
if ! [[ $FIELDSIZE =~ $re ]] || [ $FIELDSIZE -lt 2 ]; then
    echo "The field size (-f) must be a positive integer >= 2"
    exit 1
fi
if ! [[ $N =~ $re ]] || [ $N -lt 1 ]; then
    echo "The number of variables (-n) must be a positive integer"
    exit 1
fi
if ! [[ $M =~ $re ]] || [ $M -lt 1 ]; then
    echo "The number of equations (-m) must be a positive integer"
    exit 1
fi
if ! [[ $DEG =~ $re ]] || [ $DEG -lt 1 ]; then
    echo "The degree (-d) must be an integer"
    exit 1
fi

if [ $M -le $N ]; then
    echo "the number of equations must be greater than the number of variables"
    exit 1
fi


echo ""
echo "=== Building generator binary ==="
make FIELDSIZE=${FIELDSIZE} MAXVARS=${N} DEG=${DEG} ${GENERATOR} || exit 1

echo ""
echo "=== Running generator binary ==="
${GENERATOR} -m ${M} -o ${OUTPUTFILENAME} ${OPTIONS}
