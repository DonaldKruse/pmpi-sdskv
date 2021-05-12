CC = mpicc

FLAGS_PMPI = -Wall -g -shared 
LIBS_PMPI = $(pkg-config --libs margo) 
#-lmargo -lmercury -labt -lssg
TARGETS=main-generate ping-pong pmpi-lib.so main-get
OBJS_PMPI = pmpi-lib.o pmpi-common.o
INCLUDE_PMPI = pmpi-common.h

CFLAGS=-g -Wall `pkg-config --cflags margo` `pkg-config --cflags ssg`  `pkg-config --cflags sdskv-server` `pkg-config --cflags sdskv-client`
LDLIBS=`pkg-config --libs sdskv-server` `pkg-config --libs sdskv-client` `pkg-config --libs margo` 

all: $(TARGETS)

.PHONY: all

pmpi-common.o: pmpi-common.c pmpi-common.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ pmpi-common.c $(LDLIBS)

pmpi-lib.o: pmpi-lib.o pmpi-common.h
	$(CC) $(CFLAGS) -fPIC -c -o $@ pmpi-lib.c $(LDLIBS)

pmpi-lib.so: $(OBJS_PMPI) 
	$(CC) $(CFLAGS) -fPIC -shared -o $@ $(OBJS_PMPI) $(LDLIBS)

main-get: main-get.c
	$(CC) -o $@ $(CFLAGS)  main-get.c $(LDLIBS)

main-generate: main-generate.c
	$(CC) -o $@ $(CFLAGS) main-generate.c $(LDLIBS)

ping-pong: ping-pong.c
	$(CC) -o $@ $(CFLAGS) ping-pong.c $(LDLIBS)

#ping-pong: mpi_pi_send.c
#	$(CC) -o $@ $(CFLAGS) mpi_pi_send.c $(LDLIBS)
# cannot run mpi on login node

clean:
	rm -f *~ *.o $(TARGETS)
