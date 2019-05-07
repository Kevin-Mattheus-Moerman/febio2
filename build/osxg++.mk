# Make include file for FEBio on Linux

include $(FEBDIR)build/osx.mk

CC = g++

FLG = -O3 -fPIC -std=c++11

INC = -I$(FEBDIR) -I$(FEBDIR)build/include

LIBS -= -pthread