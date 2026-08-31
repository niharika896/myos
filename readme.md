# OS Project

A simple operating system project built for the **i686 (32-bit x86)** architecture.

## Prerequisites

Install the required dependencies:

```bash
sudo apt update
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo
```

Install QEMU:

```bash
sudo apt install qemu-system-x86
```

## Cross Compiler Setup

This project uses an `i686-elf` cross-compiler.

### 1. Download Binutils and GCC

```bash
mkdir -p ~/src
cd ~/src

wget https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
wget https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/gcc-14.1.0.tar.gz

tar -xvf binutils-2.42.tar.gz
tar -xvf gcc-14.1.0.tar.gz

rm *.tar.gz
```

### 2. Configure Environment Variables

Add the following to `~/.bashrc`:

```bash
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
```

Reload the configuration:

```bash
source ~/.bashrc
```

Verify:

```bash
echo $TARGET
echo $PREFIX
```

You should see:

```text
i686-elf
/home/<username>/opt/cross
```

### 3. Build Binutils

```bash
cd ~/src

mkdir build-binutils
cd build-binutils

../binutils-2.42/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror

make
make install
```

### 4. Build GCC

```bash
cd ~/src

mkdir build-gcc
cd build-gcc

../gcc-14.1.0/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers

make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc

make install-gcc
make install-target-libgcc
```

Verify the cross-compiler:

```bash
i686-elf-gcc --version
```

## Clone the Repository

```bash
git clone <repository-url>
cd <repository-name>
```

## Build and Run

Once the dependencies and cross-compiler are installed, simply run:

```bash
make run
```

This will build the OS and launch it using QEMU.

## Clean

To remove generated build files:

```bash
make clean
```

## Toolchain

* **Architecture:** i686 (32-bit x86)
* **Target:** `i686-elf`
* **Binutils:** 2.42
* **GCC:** 14.1.0
* **Emulator:** QEMU
