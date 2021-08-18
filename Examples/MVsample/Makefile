CC    	= g++ -fopenmp
CCFLAGS = 
OBJS 	= csr_complex.o utils.o practice.o

all	: mytest

mytest	: $(OBJS)
	$(CC) -o spmv $(CCFLAGS) $(OBJS)
#	cp spmv ../Runs/

clean :
	rm -rf *.o spmv ./result

.SUFFIXES : .cpp .o

.cpp.o:   ; $(CC) -c $(CCFLAGS) $(INC) $?


