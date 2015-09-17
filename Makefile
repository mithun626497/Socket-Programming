## makefile to compile the following .c files in order: healthcenterserver.c, doctor.c, patient1.c, patient2.c
## the .o files will be saved in the obj folder.
## the command used is "gcc -o yourfileoutput yourfile.c -Isocket -lnsl -lresolv"
## where yourfile.c stands for each of the .c files

CC=gcc

WORK_DIR=.
EXECUTABLE_DIR=./executable

all: healthcenterserver doctor patient1 patient2

healthcenterserver: $(WORK_DIR)/healthcenterserver.c
			$(CC) -o healthcenterserveroutput $(WORK_DIR)/healthcenterserver.c -lsocket -lnsl -lresolv

doctor: $(WORK_DIR)/doctor.c
			$(CC) -o doctoroutput $(WORK_DIR)/doctor.c -lsocket -lnsl -lresolv

patient1: $(WORK_DIR)/patient1.c
			$(CC) -o patient1output $(WORK_DIR)/patient1.c -lsocket -lnsl -lresolv

patient2: $(WORK_DIR)/patient2.c
			$(CC) -o patient2output $(WORK_DIR)/patient2.c -lsocket -lnsl -lresolv

clean:	
	rm -rf *output*        


