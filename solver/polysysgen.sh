#!/bin/bash

NAME="generator"

cleanup()
{
    if [ `ls ../bin|grep ${NAME}|wc -l` -gt 0 ]; then
        rm -f ../bin/${NAME}*
    fi
}

display_help()
{
    echo "Random overdefined polyomial equations' generator over a finite field"
    echo "Usage : $0 -f <fieldsize> -n <no. of variables> -m <no. of equations> [-d <degree>]"
    echo "Arguments"
    echo "  -h                    : display this help"
    echo "  -f <fieldsize>        : set the size of the finite field"
    echo "  -n <no. of variables> : set the number of variables"
    echo "  -m <no. of equations> : set the number of equations"
    echo "  -d <degree>           : (optional) set the degree (default=2)"
    echo "  -o <output filename>  : (optional) set the name for output file"
    exit 0
}

cleanup
while getopts :hf:n:m:d:o: option
do
    case "$option" in
        h)
            display_help
            ;;
        n)
            N=$OPTARG
            ;;
        m)
            M=$OPTARG
            ;;
        f)
            FIELDSIZE=$OPTARG
            ;;
        d)
            DEG=$OPTARG
            ;;
        o)
            OUTPUTFILENAME=$OPTARG
            ;;
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

echo "size of the finite field : $FIELDSIZE"
echo "number of variables      : $N"
echo "number of equations      : $M"
echo "degree                   : $DEG"
echo "output filename format   : $OUTPUTFILENAME"

pushd ../ && \
    make FIELDSIZE=${FIELDSIZE} MAXVARS=${N} NPOLYS=${M} DEG=${DEG} ${NAME} &&\
    popd

pushd ../bin && \
    ./${NAME}_n${N}_m${M}_${FIELDSIZE} ${OUTPUTFILENAME} && \
    popd
