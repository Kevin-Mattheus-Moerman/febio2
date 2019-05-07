# Make include file for FEBio on Mac
# This makefile assumes that llvm was installed with homebrew in its default directories (brew install llvm)

LLVM_PATH = /usr/local/opt/llvm

CC = $(LLVM_PATH)/bin/clang++

# Remove -DHAVE_LEVMAR and $(LEV_LIB) from LIBS if not linking with the Lourakis levmar routine.
# Remove -DHAVE_GSL and $(GSL_LIB) from LIBS if not linking with the GNU scientific library.
DEF = -DPARDISO -DMKL_ISS -DHAVE_LEVMAR -DHAVE_GSL -DHAVE_ZLIB -DSVN

FLG = -O3 -fopenmp -fPIC -std=c++11

# Pardiso solver
MKL_PATH = $(MKLROOT)/lib/
MKL_LIB = $(MKL_PATH)libmkl_intel_lp64.a $(MKL_PATH)libmkl_intel_thread.a $(MKL_PATH)libmkl_core.a -pthread -lz

#Levmar library
LEV_LIB = /usr/local/lib/liblevmar.a

#GSL library
GSL_LIB = /usr/local/lib/libgsl.a

LIBS = -L$(LLVM_PATH)/lib -L$(FEBDIR)build/lib $(LEV_LIB) $(LEV_LIB) $(GSL_LIB) $(MKL_LIB)

INC = -I$(LLVM_PATH)/include -I$(FEBDIR) -I$(FEBDIR)build/include
