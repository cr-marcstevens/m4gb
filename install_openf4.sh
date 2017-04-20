#/bin/bash

git clone https://github.com/nauotit/openf4 || exit 1
export OPENF4DIR=`pwd`/local

cd openf4
autoreconf --install && \
./configure --prefix=$OPENF4DIR && \
make && \
make install && \
cd .. && \
exit 0

cd ..
exit 1
