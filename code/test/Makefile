# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles yieldtest createlock print destroylock acquirelock releaselock createcondition cv_wait_signal invalidtest rand diff_lock diff_cond exectest exechelper passportoffice

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

yieldtest.o: yieldtest.c
	$(CC) $(CFLAGS) -c yieldtest.c
yieldtest: yieldtest.o start.o
	$(LD) $(LDFLAGS) start.o yieldtest.o -o yieldtest.coff
	../bin/coff2noff yieldtest.coff yieldtest

createlock.o: createlock.c
	$(CC) $(CFLAGS) -c createlock.c
createlock: createlock.o start.o
	$(LD) $(LDFLAGS) start.o createlock.o -o createlock.coff
	../bin/coff2noff createlock.coff createlock

print.o: print.c
	$(CC) $(CFLAGS) -c print.c
print: print.o start.o
	$(LD) $(LDFLAGS) start.o print.o -o print.coff
	../bin/coff2noff print.coff print

destroylock.o: destroylock.c
	$(CC) $(CFLAGS) -c destroylock.c
destroylock: destroylock.o start.o
	$(LD) $(LDFLAGS) start.o destroylock.o -o destroylock.coff
	../bin/coff2noff destroylock.coff destroylock

acquirelock.o: acquirelock.c
	$(CC) $(CFLAGS) -c acquirelock.c
acquirelock: acquirelock.o start.o
	$(LD) $(LDFLAGS) start.o acquirelock.o -o acquirelock.coff
	../bin/coff2noff acquirelock.coff acquirelock

createcondition.o: createcondition.c
	$(CC) $(CFLAGS) -c createcondition.c
createcondition: createcondition.o start.o
	$(LD) $(LDFLAGS) start.o createcondition.o -o createcondition.coff
	../bin/coff2noff createcondition.coff createcondition

cv_wait_signal.o: cv_wait_signal.c
	$(CC) $(CFLAGS) -c cv_wait_signal.c
cv_wait_signal: cv_wait_signal.o start.o
	$(LD) $(LDFLAGS) start.o cv_wait_signal.o -o cv_wait_signal.coff
	../bin/coff2noff cv_wait_signal.coff cv_wait_signal

invalidtest.o: invalidtest.c
	$(CC) $(CFLAGS) -c invalidtest.c
invalidtest: invalidtest.o start.o
	$(LD) $(LDFLAGS) start.o invalidtest.o -o invalidtest.coff
	../bin/coff2noff invalidtest.coff invalidtest

releaselock.o: releaselock.c
	$(CC) $(CFLAGS) -c releaselock.c
releaselock: releaselock.o start.o
	$(LD) $(LDFLAGS) start.o releaselock.o -o releaselock.coff
	../bin/coff2noff releaselock.coff releaselock

rand.o: rand.c
	$(CC) $(CFLAGS) -c rand.c
rand: rand.o start.o
	$(LD) $(LDFLAGS) start.o rand.o -o rand.coff
	../bin/coff2noff rand.coff rand

diff_lock.o: diff_lock.c
	$(CC) $(CFLAGS) -c diff_lock.c
diff_lock: diff_lock.o start.o
	$(LD) $(LDFLAGS) start.o diff_lock.o -o diff_lock.coff
	../bin/coff2noff diff_lock.coff diff_lock

diff_cond.o: diff_cond.c
	$(CC) $(CFLAGS) -c diff_cond.c
diff_cond: diff_cond.o start.o
	$(LD) $(LDFLAGS) start.o diff_cond.o -o diff_cond.coff
	../bin/coff2noff diff_cond.coff diff_cond

exectest.o: exectest.c
	$(CC) $(CFLAGS) -c exectest.c
exectest: exectest.o start.o
	$(LD) $(LDFLAGS) start.o exectest.o -o exectest.coff
	../bin/coff2noff exectest.coff exectest

exechelper.o: exechelper.c
	$(CC) $(CFLAGS) -c exechelper.c
exechelper: exechelper.o start.o
	$(LD) $(LDFLAGS) start.o exechelper.o -o exechelper.coff
	../bin/coff2noff exechelper.coff exechelper

passportoffice.o: passportoffice.c
	$(CC) $(CFLAGS) -c passportoffice.c
passportoffice: passportoffice.o start.o
	$(LD) $(LDFLAGS) start.o passportoffice.o -o passportoffice.coff
	../bin/coff2noff passportoffice.coff passportoffice
clean:
	rm -f *.o *.coff
