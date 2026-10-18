FROM ubuntu:26.04

ARG BINUTILS_VERSION=2.47
ARG GCC_VERSION=16.2.0
ARG NEWLIB_VERSION=4.6.0.20260123

ENV TARGET=mips64-elf
ENV PREFIX=/opt/hng64
ENV PATH=${PREFIX}/bin:${PATH}

RUN apt-get update
RUN apt-get install -y --no-install-recommends \
    build-essential ca-certificates curl xz-utils bison flex texinfo \
    libgmp-dev libmpfr-dev libmpc-dev
RUN rm -rf /var/lib/apt/lists/*

WORKDIR /build

RUN curl -fsSLO https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz
RUN tar -xf binutils-${BINUTILS_VERSION}.tar.xz
RUN mkdir build-binutils
WORKDIR /build/build-binutils
RUN ../binutils-${BINUTILS_VERSION}/configure \
    --target=${TARGET} --prefix=${PREFIX} \
    --disable-nls --disable-werror
RUN make -j"$(nproc)"
RUN make install

WORKDIR /build
RUN curl -fsSLO https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz
RUN tar -xf gcc-${GCC_VERSION}.tar.xz
RUN mkdir build-gcc
WORKDIR /build/build-gcc
RUN ../gcc-${GCC_VERSION}/configure \
    --target=${TARGET} --prefix=${PREFIX} \
    --with-arch=vr4300 --with-tune=vr4300 \
    --enable-languages=c --without-headers --with-newlib \
    --disable-multilib --disable-shared --disable-threads \
    --disable-libssp --disable-nls --disable-werror
RUN make -j"$(nproc)" all-gcc all-target-libgcc
RUN make install-gcc install-target-libgcc

# -G0 so newlib works without a gp
WORKDIR /build
RUN curl -fsSLO https://sourceware.org/pub/newlib/newlib-${NEWLIB_VERSION}.tar.gz
RUN tar -xf newlib-${NEWLIB_VERSION}.tar.gz
RUN mkdir build-newlib
WORKDIR /build/build-newlib
RUN CFLAGS_FOR_TARGET="-O2 -G0" ../newlib-${NEWLIB_VERSION}/configure \
    --target=${TARGET} --prefix=${PREFIX} \
    --disable-nls --disable-werror
RUN make -j"$(nproc)" all-target-newlib
RUN make install-target-newlib

WORKDIR /
RUN rm -rf /build

ENTRYPOINT ["/bin/bash"]
