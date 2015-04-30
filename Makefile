
#SRC += \
#	./common/DataReader.cpp \
#	./common/NeuronalPCA.cpp \
#	./common/RandomNumber.cpp \
#	./common/Interpolation.cpp \
#	./common/GrowingNeuralGas.cpp \
#	Neurosorter.cpp \

# this is a comment
SRC += \
	plx2csv.c \


OBJ=$(SRC:.c=.o) # replaces the .c from SRC with .o
EXE=plx2csv

CC=gcc
CFLAGS=-Wall
INC=-I.
LDFLAGS=
LIBS=
RM=rm

%.o : %.c         
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

.PHONY : all     # .PHONY ignores files named all
all: $(EXE)      # all is dependent on $(EXE) to be complete

$(EXE): $(OBJ)   # $(EXE) is dependent on all of the files in $(OBJ) to exist
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

.PHONY : clean   # .PHONY ignores files named clean
clean:
	$(RM) $(OBJ)

##This Makefile can be invoked in any of the following ways:

## for the above file will compile everything required by the all label
#make

## effectively the same as the previous line
#make all

## 'clean' the project of stale files
## for this Makefile, remove hello.o, main.o and core
# make clean

## clean and then build all
#make clean all

## build all and then clean
#make all clean 

