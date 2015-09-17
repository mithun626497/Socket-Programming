/*****************************************************************************************************************
* This program perform the following functionalities:								 *
* 1)  Upon boot patient2.c will read the input text file patient2.txt and store the username and password in     * 
*     memory.												         *
* 2)  It creates a TCP socket to the server running on static port 21198					 *
* 3)  Once TCP connection is created it sends message 'authenticate patient2 pasword'				 *
* 4)  If server replies back success for authentication then it proceed to phase2 and request for appointment.   *
* 5)  If server replies failure then patient2 will terminate the connection and exit.				 *
* 6)  In phase 2, patient request for available appointments, will select one of the appointments returned and   *
*     send a message 'selection index#'.									 *
* 7)  The reply for selection request will be doctor's name and port number if selection is successful else 	 *
*     reply will be 'notavailable'.										 *
* 8)  If patient 2 receives 'notavailable' from server then it closes TCP connection and stop the process.       *
* 9)  Patient2 will close the TCP socket and will create a UDP socket for the doctor's port number.  		 *
* 10) Patient2 will send the insurance details in a message 'insurance#' and receives the estimated cost of      *
*     appointment.												 *
* 11) After receiving the estimated cost of appointment, patient2 will close UDP socket and will stop all the    *
*     running processes. 											 *
*     														 * 
*****************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define SRVPORT "21198" //The HealthCenterServer static port for which the client will be connecting to.
#define BUFFSIZE 100    //Buffersize value to receive the message sent by server.

// get sockaddr, IPv4 or IPv6: code reused from beej tutorial
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void)
{
	FILE *fp1,*fp2;					//to handle the txt files
	char usr[20],pwd[20],insurance[20];		//for storing the txt file input values
	char authMsg[50]="authenticate";				//to store the authenticate message
	int streamSockFd,dgramSockFd;				//to store the socket descriptor of TCP socket   	
	struct addrinfo hints,*servinfo, *p;
	int rv,bytesRecvd;
	struct sockaddr_in sin;
	struct sockaddr addr2;
	socklen_t len = sizeof(sin);
	char ipstr[INET6_ADDRSTRLEN];
	void *addr;
	char buf[BUFFSIZE],*ptr,temp[20],*token;
	char recvdApp[200]="",recvd2App[200],index,inputIdx[6],doc[10],docPort[6],dp[6];
	int i,j,valid=0;
	char s[INET6_ADDRSTRLEN];
	int numbytes;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	char hostname[20]="nunki.usc.edu";
	char portname[20]="";
	char docName[10];
	/* Open patient2.txt in read mode to read the credentials of patient2*/
   	fp1=fopen("patient2.txt","r");
  	if(NULL==fp1)
   	{
		printf("ERROR: fopen() returning NULL for patient2.txt\n");
		return 1;
   	}  
   
   	/*Read the username and password of patient2 and store it in usr and pwd respectively*/
	fscanf(fp1,"%s %s",usr,pwd);
	fclose(fp1);
		
	/* Open patient2insurance.txt in read mode to read the insurance type */
	fp2 = fopen("patient2insurance.txt","r");
	if(NULL==fp2)
	{
		printf("ERROR: fopen() returning NULL for patient2insurance.txt\n");
		return 1;
	}	
	/* Read the insurance and store it in insurance[]*/	
	fscanf(fp2,"%s",insurance);
	fclose(fp2);

	/*Code for creating TCP socket to the Health center Server for authentication*/
	/*Load up the address structure with getaddrinfo()*/
	
	/*Beej Code Reuse - Start*/
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((rv = getaddrinfo("nunki.usc.edu", SRVPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{	
		//create a TCP Socket for the port 21198
		if ((streamSockFd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
		{
			perror("client: socket error");
			continue;
		}
		//Connect to the server
		if (connect(streamSockFd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(streamSockFd);
			perror("client: connect error");
			continue;
		}
		break;
	}//end of for loop
	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect\n");
		freeaddrinfo(servinfo); 
		return 2;
	}

	/*Beej Code Reuse - End*/
	//to get the address details of my machine	
	if (getsockname(streamSockFd, (struct sockaddr *)&sin, &len) == -1)
	    perror("getsockname error");
	else
	{	
		addr = &(sin.sin_addr);
		inet_ntop(p->ai_family,addr,ipstr,sizeof ipstr);
		printf("Phase 1: Patient 2 has the TCP port number %d and ip address %s.\n",ntohs(sin.sin_port),ipstr);
	}
	
	//Creating the request message to authenticate
	strcat(authMsg," ");	
	strcat(authMsg,usr);
	strcat(authMsg," ");
	strcat(authMsg,pwd);	
	//sending the request message
	if (send(streamSockFd,authMsg,strlen(authMsg), 0) == -1)
		perror("send error!!!");
	else
	{
		printf("Phase 1: Authentication request from Patient 2 with username %s and password %s "
		       " has been sent to the Health Center Server\n",usr,pwd);
	}
	
	//Receiving the response for authentication from Health Center Server
	if ((bytesRecvd = recv(streamSockFd, buf, BUFFSIZE-1, 0)) == -1)
	{
		perror("recv error!!!");
		exit(1);
	}
	else
	{
		buf[bytesRecvd] = '\0';
		printf("Phase 1: Patient 2 authentication result: %s.\n",buf);	
	}
	
	
	//start of phase 2
	if(!strcmp(buf,"success"))
	{
		//Display that we have reached the end of Phase 1
		printf("Phase 1: End of Phase 1 for Patient2.\n");
		
		//send the request for available appointments
	 	if (send(streamSockFd,"available",strlen("available"), 0) == -1)
			perror("send error!!!");

		//Receive the appointment list sent by the server		
		if ((bytesRecvd = recv(streamSockFd, buf, BUFFSIZE-1, 0)) == -1)
		{
			perror("recv error!!!");
			exit(1);
		}
		else
		{
			buf[bytesRecvd] = '\0';
			strcpy(recvdApp,buf);
			token=strtok(recvdApp," ");
			strcpy(recvd2App,buf);
			//if the response message is failure then display no appointments available			
			if(!strcmp(token,"failure"))
			{
				printf("Phase 2: No Appointments available\n");
				//if authentication fails close the connection and exit.
				printf("\n");
				freeaddrinfo(servinfo); 
				close(streamSockFd);
				return 1;

			}			
			//if response is invalid then display invalid request are available
			else if(!strcmp(token,"invalid"))
			{
				printf("Invalid request. Exiting...\n");
				//if authentication fails close the connection and exit.
				printf("\n");
				freeaddrinfo(servinfo); 
				close(streamSockFd);
				return 1;

			}
			else
			{	
				//else display the available appointments.
				printf("Phase 2: The following appointments are available for Patient 2:\n");
				ptr=recvd2App;
				i=0;
				while(*ptr != '\0')
				{
					strncpy(temp,ptr,11);
					temp[11]='\0';
					printf("%s\n",temp);
					inputIdx[i]=temp[0];
					i++;
					ptr = ptr+11;	
				}
				valid=0;				
				while(valid!=1)
				{	
					//ask the user to enter the appointment index from one in the listed appointments
					printf("Please enter the preferred appointment index and press enter: \n");
					scanf(" %c",&index);
				
					for(j=0;j<i;j++)
					{	
						if(index==inputIdx[j])
						{	
							valid=1;
							break;		
						}
		
					} 
					if(j==i && valid!=1){
						printf("invalid input.\n");
					}
				  
				}
				//send the requested appointment details
				if (send(streamSockFd,&index,sizeof index, 0) == -1)
					perror("send error!!!");
		
			}	
		}
		//receive the response for appointment request
		if ((bytesRecvd = recv(streamSockFd, buf, BUFFSIZE-1, 0)) == -1)
		{
			perror("recv error!!!");
			freeaddrinfo(servinfo); 
			close(streamSockFd);
			exit(1);
		}
		else
		{
			buf[bytesRecvd] = '\0';
			token=strtok(buf," ");
			//if the response is success then print the reservation successful message.
			if(strcmp(token,"notavailable"))
			{
				strcpy(doc,token);
				strncpy(docName,doc,4);
				docName[4]='\0';
				token=strtok(NULL," ");	
				strcpy(docPort,token);
				strncpy(dp,docPort,5);
				dp[5]='\0';
	
				printf("Phase 2: The requested appointment is available and "
					"reserved for Patient2. The assigned doctor port is %s\n",dp);
			}
			else
			{	
				//else print not available
				printf("Phase 2: The requested appointment from Patient 2 is not available. Exiting...\n");
				printf("\n");
				freeaddrinfo(servinfo); 
				close(streamSockFd);
				return 1;
			}
		}
	}
	else
	{	
		//if authentication fails close the connection and exit
		printf("Exiting...\n");
		freeaddrinfo(servinfo); 
		close(streamSockFd);
		return 1;
	}
	freeaddrinfo(servinfo); 
	close(streamSockFd);
	
	//code for udp socket - code reused from beej tutorial
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	//create UDP socket to the doctor port number received in phase 1
	if ((rv = getaddrinfo(hostname,dp, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "second getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	memset(&addr2, 0, sizeof addr2);
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((dgramSockFd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("UDP socket failed");
			continue;
		}
		if (bind(dgramSockFd,(struct sockaddr *)&addr2, sizeof addr2) == -1) 
		{
			close(dgramSockFd);
			perror("patient2: UDP bind Failed");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "failed to bind socket\n");
		freeaddrinfo(servinfo); 
		return 2;
	}
	inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr),ipstr, sizeof ipstr);
	if (getsockname(dgramSockFd, (struct sockaddr *)&sin, &len) == -1)
		    perror("getsockname error");
	else
	{	
		addr = &(sin.sin_addr);
		printf("Phase 3: Patient 2 has dynamic UDP port number %u and IP address %s\n",ntohs(sin.sin_port),ipstr);
	}

	//send the insurance type to the server
	if ((numbytes = sendto(dgramSockFd, insurance, strlen(insurance), 0,p->ai_addr, p->ai_addrlen)) == -1) 
	{
		perror("UDP sendto failed");
		freeaddrinfo(servinfo); 
		close(dgramSockFd);
		exit(1);
	}
	else
	{
		printf("Phase 3: The cost estimation request from patient2 with insurance plan %s has "
			"been sent to the doctor with \n port number %s and ip address %s\n",insurance,dp,ipstr);
	}
	//receive the response for the insurance request.
	if ((numbytes = recvfrom(dgramSockFd, buf, BUFFSIZE-1 , 0,(struct sockaddr *)p->ai_addr, &(p->ai_addrlen))) == -1) 
	{
		perror("recvfrom failed");
		freeaddrinfo(servinfo); 
		close(dgramSockFd);
		exit(1);
	}
	buf[numbytes] = '\0';
	//if request is successfully responded then print the estimated cost.
	if(strcmp(buf,"invalid"))
	{
		printf("Phase 3: Patient 2 receives %s$ estimation cost from doctor with port "
			"number %s and name %s\n",buf,dp,docName);
	}	
	else	
	{
		//else print invalid insurance type in the request
		printf("Invalid insurance type\n");
		printf("\n");
		freeaddrinfo(servinfo);
		close(dgramSockFd);
		return 1;	
	}
	printf("Phase 3: End of Phase 3 for Patient2\n");
	printf("\n");
	freeaddrinfo(servinfo);
	close(dgramSockFd);
	return 0;
}
