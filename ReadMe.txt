README

Full Name:  Mithun Maragiri
Student ID: 5210213198


What Have I done in the assignment:
- Implemented the healthcenterserver program to listen on port 21198 and accept connections.
  authenticate the patient, provide the list of available appointments and also provide 
  the details of doctor name and doctor port number as requested by the patient.
- Implemented patient1 and patient2 programs to act as clients and send requests to the 
  healthcenterserver and doctor programs. The requests to healthcenterserver will be to 
  authenticate the patient, request to provide the list of available appointments, select one 
  from the available list of appointments. Now make UDP socket to doctor program and send request
  for the estimated cost of visit and receive the estimated cost of visit from the doc.
- Implemented single doctor program to support 2 doctors. fork system call is used to achieve 
  the same. Single doctor executable will address the requests of both the doctors. The request 
  will be to provide the estimated cost of visit for a particular insurance type.


FILES USED
-TXT Files:  patient1.txt, patient2.txt,users.txt,availabilities.txt,doc1.txt,doc2.txt,
             patient1insurance.txt, patient2insurance.txt
-.C FIles:   healthcenterserver.c, doctor.c, patient1.c and patient2.c
- One Makefile is used to build the executables.


WHAT EACH FILE DOES:
1) patient1.txt, patient2.txt : contain the username and password for the patient1 and patient2 respectively.
2) users.txt : Contains the list of patients who are registered to online medical appointment system.
3) availabilities.txt : Contains the list of available appointments.
4) doc1.txt, doc2.txt : contains the estimated cost of visit values for insurances 1,2 and 3 for doctors 1 and 2
5) patient1insurance.txt, patient2insurance.txt : contains the insurance type opted by the patients 1 and 2.
6) healthcenterserver.c : contains the code for the health center server implementation to meet the requirements
   as explained in the problem description.
7) doctor.c : single file to handle the requests for both doctor 1 and doctor 2. fork() is used to handle requests for
   both doctors in same file.
8) patient1.c and patient2.c: contains the code for the program execution of patients, to meet the requirement criteria 
   of the patients as explained in the problem description.
9) Makefile builds the 4 executables namely healthcenterserveroutput,patient1output,patient2output,doctoroutput

HOW TO USE
Please follow the below steps to use the online medical appointment system.
1) copy the zip file ee450_maragiri_session2.tar.gz and extract it.
2) step into the folder ee450_maragiri_session2 by using the command "cd ee450_maragiri_session2"
3) the extraction should contain 8 .txt files, 4 .c files, 1 makefile, 1 Readme file.
4) enter the command "make clean"
5) Then enter the command "make"
6) This generates the executables for the .c files in the name healthcenterserveroutput, doctoroutput, 
   patient1output and patient2output
7) Please bring up the executables one after the other in the same sequence as mentioned below.
   - enter ./healthcenterserver
   - open a new terminal and go to the same path and enter ./doctor
8) Now the servers are ready and listening 
9) To test sending request from patient1, open a new terminal and go inside ee450_maragiri_session2 
   - enter ./patient1
   - This starts the communication between patient and healthcenter server. 
   - the display messages as explained in the project description can be seen on the console.
   - you will be asked to select one of the available appointments.
   - select one of the available index values.
   - Upon selection you will receive the port number and doc name 
   - The patient program will then make a request over UDP and the estimated cost of visit can be seen 
     on the console if there are no errors.
10) You can repeat the same for patient2.
   - enter ./patient2 and repeat step 9
11) once all the testing is done. please press Control+C, or Control+z to kill the server processes.
12) enter the command ps -all and check if any of the above 4 executable names are present. 
13) if found please kill the processes using the command kill -9 XXX where XXX is the PID of the process.

FORMAT OF MESSAGES EXCHANGED 
NOTE: All formats are without quotes
1) Request message from patient to healthcenterserver over TCP has the following format
   -"authenticate username password"
2) Authentication response from healthcenterserver to patient is of the format
   -"success" upon successful authentication
   -"failure" upon failure
3) Request message to get the available appointments is of the format
   -"available"
4) the response message from server to patient has the list of appointments of the format
   -"1 Wed 05pm"
    "3 Thu 11AM" etc
   -"notavailable" if appointment is not available
5) the selected index message is of the format
   -"3"
6) Upon successful reservation the response message will be of the format
   -"doc1 41198" or "doc2 42198" etc
7) The request message for the estimation of the visiting cost over UDP is of the form
   -"insurance1" or "insurance2" or "insurance3"
8) The response message for the requested insurance is of the format
   -"30" or "40" or "60" etc or "invalid" if insurance type is not supported.
     
CONDITIONS OF FAILURE
The project will work only if the healthcenterserver and doctor programs are loaded and running 
before loading the patient program.

REUSED CODE FROM BEEJ TUTORIAL
The following code has been reused for the socket programming implementation.

The following function calls have been used from Beej Tutorial
 - sigchld_handler(int s)
 - void *get_in_addr(struct sockaddr *sa)
 - getaddrinfo()
 - socket()
 - setsockopt()
 - bind()
 - listen()
 - sigaction(SIGCHLD, &sa, NULL)

