#!/bin/sh
#

## Test for executables
TestEXE()
{
  TEMP=`type $1`
  if [ $? != 0 ]; then
    echo "Error: $1 not installed";
    exit 1;
  fi
}

## Test for files to unpack
TestFile()
{
  if [ ! -f "$1" ]; then 
    echo "Error: $1 not found";
    exit 1;
  fi
}

TestEXE "make";
TestEXE "gcc";
TestEXE "flex";
TestEXE "patch";
TestEXE "tar";
TestEXE "gzip";
TestEXE "autoconf";
TestEXE "gperf";
TestEXE "bison";
#Test if exists: gmp and mpfr libs


TestFile "binutils-2.20.1.tar.gz";
TestFile "newlib-1.17.0.tar.gz";
TestFile "gcc-4.4.2.tar.gz";
TestFile "binutils-2.20.1-vb.patch";
TestFile "gcc-4.4.2-vb.patch";

export CFLAGS="-Wno-error"
export CXXFLAGS="-Wno-error"

## Build Binutils
tar zxvf binutils-2.20.1.tar.gz
cd binutils-2.20.1
cat ../binutils-2.20.1-vb.patch | patch -p1
cat ../binutils-2.20-vb_blitter-20101122.patch | patch -p1
cat ../binutils-2.20-vb_blitter-20130904.patch | patch -p1
cd ..
mkdir binutil_build
cd binutil_build
../binutils-2.20.1/configure --target=v810 --prefix=/opt/gccvb
make all install
if [ $? != 0 ]; then
  echo "Error: building binutils";
  exit 1;
fi
cd ..


# Build a minimal GCC 
tar zxvf gcc-4.4.2.tar.gz
cd gcc-4.4.2
cat ../gcc-4.4.2-vb.patch | patch -p1
cat ../gcc-4.4.2-vb_blitter-20101123.patch | patch -p1
cat ../gcc-4.4.2-vb_jweinberg-20110908.patch | patch -p1
cp -fp ../extra/lib1funcs.asm gcc/config/v810/
cd ..

PATH=$PATH:/opt/gccvb
mkdir gcc_build
cd gcc_build
../gcc-4.4.2/configure --target=v810 --prefix=/opt/gccvb --enable-languages=c --without-headers --with-newlib --disable-shared --disable-threads
make all-gcc install-gcc
if [ $? != 0 ]; then
  echo "Error: building gcc";
  exit 1;
fi
cd ..

# Build Newlib
# if this fails thain dos2unix the newlib folder
tar zxvf newlib-1.17.0.tar.gz
cd newlib-1.17.0
cat ../newlib-1.17.0-vb.patch | patch -p1
cd ..

mkdir newlib_build
cd newlib_build
PATH=$PATH:/opt/gccvb/bin
ln -s /opt/gccvb/bin/v810-gcc /opt/gccvb/bin/v810-cc 
../newlib-1.17.0/configure --target=v810 --prefix=/opt/gccvb
make all install
if [ $? != 0 ]; then
  echo "Error: building newlib";
  exit 1;
fi
cd ..

# Build GCC again with no restrictions
cd gcc_build
../gcc-4.4.2/configure --target=v810 --prefix=/opt/gccvb --enable-languages=c
make all install
if [ $? != 0 ]; then
  echo "Error: building gcc full";
  exit 1;
fi
cd ..