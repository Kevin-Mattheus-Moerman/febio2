# Make include file for FEBio on Mac

CC = icpc

# Remove -DHAVE_LEVMAR and $(LEV_LIB) from LIBS if not linking with the Lourakis levmar routine.
DEF = -DPARDISO -DMKL_ISS -DHAVE_LEVMAR -DHAVE_GSL -DHAVE_ZLIB -DSVN

FLG = -Os -qopenmp -fPIC -static-intel -no-intel-extensions -std=c++11 -Wl,-rpath,@executable_path

# Pardiso solver
INTELROOT = $(subst /mkl,,$(MKLROOT))
INTEL_INC = $(INTELROOT)/compiler/include
INTEL_LIB = -L$(INTELROOT)/compiler/lib/
MKL_PATH = $(MKLROOT)/lib/
MKL_LIB = $(MKL_PATH)libmkl_intel_lp64.a $(MKL_PATH)libmkl_intel_thread.a $(MKL_PATH)libmkl_core.a \
	-liomp5 -pthread -lz

#Levmar library
LEV_LIB = -llevmar

GSL_LIB = /usr/local/lib/libgsl.a

LIBS = -L$(FEBDIR)build/lib -L$(INTELROOT)/compiler/lib/ $(LEV_LIB) $(GSL_LIB) $(MKL_LIB)

INC = -I$(INTEL_INC) -I$(FEBDIR) -I$(FEBDIR)build/include
