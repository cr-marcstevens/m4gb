#/bin/bash

: ${OPENF4_INSTALL_PREFIX:=$(pwd)/openf4}

rm -rf openf4-git
git clone --depth 1 https://github.com/cr-marcstevens/openf4 openf4-git || exit 1

cd openf4-git
autoreconf --install && \
./configure --prefix=$OPENF4_INSTALL_PREFIX && \
make && \
make install && \
cd .. && \
exit 0

cd ..
exit 1
