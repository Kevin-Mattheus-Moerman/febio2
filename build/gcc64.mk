# Make include file for FEBio on Linux

include $(FEBDIR)build/lnx64.mk

LEV_INC = /home/sci/mherron/Resources/levmar-2.6

CC = g++

FLG = -O3 -fPIC -fopenmp -std=c++11

INC = -I$(FEBDIR) -I$(FEBDIR)build/include -I$(LEV_INC)
