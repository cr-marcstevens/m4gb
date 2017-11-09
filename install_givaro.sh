#/bin/bash

: ${GIVARO_INSTALL_PREFIX:=$(pwd)/givaro}

rm -rf givaro-git
git clone --depth 1 https://github.com/linbox-team/givaro.git givaro-git || exit 1

cd givaro-git
autoreconf --install && \
./configure --prefix=$GIVARO_INSTALL_PREFIX && \
make && \
make install && \
cd .. && \
exit 0

cd ..
exit 1
