#!/bin/bash

######################################################
# 
# Setup OS lab tool chain (i386-elf-* cross compiler). 
#
# Tested on Mac OS, Ubuntu and Fedora.
#
# Author: Ryan Huang <huang@cs.jhu.edu>
#
# Example Usage: ./toolchain-build.sh /home/ryan/318/toolchain
#

perror()
{
  >&2 echo $1
  exit 1
}

download_and_check()
{
  local fname=$(basename $1)
  local fdirname="${fname%.tar.*}"
  cd $CWD/src
  if [ ! -f $fname ]; then
    wget $1 
    if [ ! -f $fname ]; then
      perror "Failed to download $1"
    fi
  fi
  local checksum=$(shasum -a 256 $fname | cut -d' ' -f 1)
  if [ "$checksum" != "$2" ]; then
    perror "Corrupted or missing source $1: expecting $2 got $checksum"
  fi
  echo "Downloaded and verified $fname from $1"
  if [ ! -d $fdirname ]; then
    echo "Extracting $fname to $fdirname..."
    if [ $fname == *.tar.gz ]; then
      tar xzf $fname 
    elif [ $fname == *.tar.bz2 ]; then
      tar xjf $fname
    elif [ $fname == *.tar.xz ]; then
      tar xJf $fname
    else
      perror "Unrecognized archive extension $fname"
    fi
  else
    echo "$fname is already extracted"
  fi
}

usage()
{
  cat <<EOF

  Usage: $0 [options] [DEST_DIR] [TOOL]

    -h: display this message

    DEST_DIR: Base directory to store the downloeaded source code, build and 
              distribute the compiled toolchain.
    TOOL:     By default, this script build three targets: binutils, GCC, and GDB. 
              Specify a single target to download and build. Must be one of
              {binutils, gcc, gdb}.

  Example:
    1. $0 /home/ryan/318/toolchain
    2. $0 /home/ryan/318/toolchain gcc

EOF
}

if [ $# -eq 0 ]; then
  >&2 usage
  exit 1
fi

if [ "$1" == "-h" -o "$1" == "--help" ]; then
  usage
  exit 0
fi

tool=all
if [ $# -eq 2 ]; then
  tool=$(echo "$2" | tr '[:upper:]' '[:lower:]')
  if [ $tool != "binutils" -a $tool != "gcc" -a $tool != "gdb" ]; then
    perror "TOOL must be one of {binutils,gcc,gdb}"
  fi
fi

os="`uname`"
dist="`uname -m`"

mkdir -p $1/{src,$dist,build} || perror "Failed to create toolchain source and build directories"

CWD=$(cd $1 && pwd)
PREFIX=$CWD/$dist
export PATH=$PREFIX/bin:$PATH
if [ $os == "Linux" ]; then
  export LD_LIBRARY_PATH=$PREFIX/lib:$LD_LIBRARY_PATH
elif [ $os == "Darwin" ]; then
  export DYLD_LIBRARY_PATH=$PREFIX/lib:$DYLD_LIBRARY_PATH
else
  perror "Unsupported OS: $os"
fi

TARGET=i386-elf

# Download sources
if [ $tool == "all" -o $tool == "binutils" ]; then
  download_and_check https://ftp.gnu.org/gnu/binutils/binutils-2.27.tar.gz 26253bf0f360ceeba1d9ab6965c57c6a48a01a8343382130d1ed47c468a3094f
fi
if [ $tool == "all" -o $tool == "gcc" ]; then
  download_and_check https://ftp.gnu.org/gnu/gcc/gcc-6.2.0/gcc-6.2.0.tar.bz2 9944589fc722d3e66308c0ce5257788ebd7872982a718aa2516123940671b7c5
  if [ ! -d $CWD/src/gcc-6.2.0/mpfr ]; then
    cd $CWD/src/gcc-6.2.0 && contrib/download_prerequisites || perror "Failed to download pre-requisite for GCC"
  fi
fi
if [ $tool == "all" -o $tool == "gdb" ]; then
  download_and_check https://ftp.gnu.org/gnu/gdb/gdb-7.9.1.tar.xz cd9c543a411a05b2b647dd38936034b68c2b5d6f10e0d51dc168c166c973ba40
fi

if [ $tool == "all" -o $tool == "binutils" ]; then
  echo "Building binutils..."
  mkdir -p $CWD/build/binutils && cd $CWD/build/binutils
  ../../src/binutils-2.27/configure --prefix=$PREFIX --target=$TARGET \
    --disable-multilib --disable-nls --disable-werror || perror "Failed to configure binutils"
  make -j8 || perror "Failed to make binutils"
  make install
fi

if [ $tool == "all" -o $tool == "gcc" ]; then
  echo "Building GCC..."
  mkdir -p $CWD/build/gcc && cd $CWD/build/gcc 
  ../../src/gcc-6.2.0/configure --prefix=$PREFIX --target=$TARGET \
    --disable-multilib --disable-nls --disable-werror --disable-libssp \
    --disable-libmudflap --with-newlib --without-headers --enable-languages=c,c++ || perror "Failed to configure gcc"
  make -j8 all-gcc  || perror "Failed to make gcc"
  make install-gcc
  make all-target-libgcc || perror "Failed to libgcc"
  make install-target-libgcc
fi

if [ $tool == "all" -o $tool == "gdb" ]; then
  echo "Building gdb..."
  mkdir -p $CWD/build/gdb && cd $CWD/build/gdb 
  ../../src/gdb-7.9.1/configure --prefix=$PREFIX --target=$TARGET --disable-werror || perror "Failed to configure gdb"
  make -j8 || perror "Failed to make gdb"
  make install
fi

echo "*********************************************************"
echo "*                                                       *"
echo "* Congratulations! You've built the cross-compiler      *"
echo "* Don't forget to add the following to .bashrc          *"
echo "* export PATH=$PREFIX/bin:\$PATH                        *"
echo "*                                                       *"
echo "*********************************************************"
