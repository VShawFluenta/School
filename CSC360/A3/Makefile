# Compiler and flags
CC = gcc
CFLAGS = -Wall  -std=c99 -g

# Executables
EXECS = diskinfo disklist diskget  diskput
TARGET = diskinfo disklist diskget diskput

# Object files
OBJS = diskFunctionsLibrary.o

# Default target
all: $(EXECS)

# Compile the library object file
diskFunctionsLibrary.o: diskFunctionsLibrary.c diskFunctionsLibrary.h
	$(CC) $(CFLAGS) -c diskFunctionsLibrary.c -o diskFunctionsLibrary.o

# Compile and link each executable
diskinfo: diskinfo.o $(OBJS)
	$(CC) $(CFLAGS) diskinfo.o $(OBJS) -o diskinfo

disklist: disklist.o $(OBJS)
	$(CC) $(CFLAGS) disklist.o $(OBJS) -o disklist

diskget: diskget.o $(OBJS)
	$(CC) $(CFLAGS) diskget.o $(OBJS) -o diskget

diskput: diskput.o $(OBJS)
	$(CC) $(CFLAGS) diskput.o $(OBJS) -o diskput

# Compile the source files into object files
diskinfo.o: diskinfo.c diskFunctionsLibrary.h
	$(CC) $(CFLAGS) -c diskinfo.c -o diskinfo.o

disklist.o: disklist.c diskFunctionsLibrary.h
	$(CC) $(CFLAGS) -c disklist.c -o disklist.o

diskget.o: diskget.c diskFunctionsLibrary.h
	$(CC) $(CFLAGS) -c diskget.c -o diskget.o

diskput.o: diskput.c diskFunctionsLibrary.h
	$(CC) $(CFLAGS) -c diskput.c -o diskput.o

# Clean up the build
clean:
	rm -f $(EXECS) *.o

.PHONY: all clean