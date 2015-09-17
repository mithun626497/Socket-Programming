/********************************************************************************************
* This program perform the following functionalities					    *
* 1) Upon bootup healthcenterserver (HCS) loads the contents of the file users.txt and then *
*    save the list of registered patients and their passwords	    			    *
* 2) HCS will be listening on the static port number 21198 with TCP socket		    *
* 3) Upon accepting a connection, HCS checks if the request is "authenticate". If so then   *
*    it will check with its database if the username and password is same for the patient   *
*    requesting authentication.					    			    *
* 4) Upon successful authentication server replies "success" else "failure" to the patient  *
* 5) If the next request message is "available" from the patient then HCS will check if the * 
*    patient is authenticated. 								    *
* 6) If yes then HCS will open the availibilities.txt file and read the list of available   * 
*     appointments and will reply to the patient with the list of available appointments.   *
* 7) If the next request received is a message with selected index, HCS will check if the   *
*    requested appointment is still available. If yes then it will provide the doctor name  * 
*    and doctor port number to the patient. If not available it will reply "notavailable".  * 
* 8) If any of the above steps doesnt happen in the sequence mentioned then HCS will reply  * 
*    with appropriate error message.							    *
*********************************************************************************************/
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
#define BACKLOG 10 	// to set the maximum number of clients that can send request.

//signal handler to reap the dead processes.code reused from Beej tutorial
void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// To get sockaddr, IPv4 or IPv6:code reused from Beej tutorial
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
	FILE *fp1,*fp2; 					// file pointers for users.txt and availibilites.txt
	char pat1[30],pat2[20],pwd1[20],pwd2[20],temp[50]; 	// to store the contents of users.txt
	int sockfd, new_fd; 					// listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p; 			// to get the address info to create socket.
	struct sockaddr_storage their_addr; 			// connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN],ipstr[INET6_ADDRSTRLEN],srvip[INET6_ADDRSTRLEN];
	int rv,bytesRecvd;
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	void *addr;
	char buf[BUFFSIZE],reqType[50],*token,usr[20],pwd[20],response[20],docDetails[20];
	char authRsp[200]="";
	int reserved[6]={0},i,j,idxint;
	char *idxSelected,*idxSelChar;

	//opening users.txt file
	fp1=fopen("users.txt","r");				
  	if(NULL==fp1)
   	{
		printf("ERROR: fopen() returning NULL for users.txt\n");
		return 1;
   	}  
   
   	//Read the username and password of both patients.
	fscanf(fp1,"%s %s %s %s",pat1,pwd1,pat2,pwd2);
	fclose(fp1);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	// to get the address information of server itself.
	if ((rv = getaddrinfo("nunki.usc.edu", SRVPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "ERROR!! getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	//Get the first match of ip address and create socket and then bind the socket to server address.
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		//creating a socket
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("server: socket()failed");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
		{
			perror("setsockopt() failed");
			close(sockfd);
			freeaddrinfo(servinfo);
			exit(1);
		}
		//binding the socket
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "server: failed to bind\n");
		freeaddrinfo(servinfo);
		return 2;
	}
	
	//get the socket details of server and print the same
	if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
	    perror("getsockname error");
	else
	{	
		addr = &(sin.sin_addr);
		inet_ntop(p->ai_family,addr,ipstr,sizeof ipstr);
		strcpy(srvip,ipstr);
		printf("Phase 1: The Health Center Server has the "
		       "port number %d and ip address %s.\n",ntohs(sin.sin_port),ipstr);
	
	}
	//listen on the static port 21198. Code reused from Beej Tutorial	
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen() failed");
		freeaddrinfo(servinfo);
		close(sockfd);
		exit(1);
	}
	//signal handler to reap dead processes. Code reused from Beej Tutorial
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction() failed");
		freeaddrinfo(servinfo);
		close(sockfd);
		exit(1);
	}
	freeaddrinfo(servinfo);
	while(1) 
	{ 
		//Accept the client
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) 
		{
			if(errno==EINTR)			
			continue;
			perror("accept");
		}
		inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
		//forking to handle multiple patients
		if (!fork()) 
		{ 	// this is the child process
			close(sockfd); // child doesn't need the listener
			//Receiving the response for authentication from Health Center Server
			if ((bytesRecvd = recv(new_fd, buf, BUFFSIZE-1, 0)) == -1)
			{
				perror("first recv error!!!");
				exit(1);
			}
			else
			{
				buf[bytesRecvd] = '\0';
				token=strtok(buf," ");
				strcpy(reqType,token);
				token=strtok(NULL," ");
				strcpy(usr,token);
				token=strtok(NULL," ");
				strcpy(pwd,token);
				if(!strcmp(reqType,"authenticate"))
				{
					printf("Phase 1: The Health Center Server has received request "
					       "from a patient with username %s and password %s.\n",usr,pwd);
					
					if(!strcmp(usr,pat1))	
					{
						if(!strcmp(pwd,pwd1))
						{	
							strcpy(response,"success");
						}
						else
						{
							strcpy(response,"failure");
						}
					}
					else if(!strcmp(usr,pat2))
					{
						if(!strcmp(pwd,pwd2))
						{
							strcpy(response,"success");
						}
						else
						{
							strcpy(response,"failure");
						}
					}
					else
					{
						strcpy(response,"failure");
					}
				}
				else
				{
					strcpy(response,"invalid");
				}
				//sending response for the received request.
				if (send(new_fd,response,strlen(response), 0) == -1)
					perror("send error!!!");
				else
				{
					printf("Phase 1: The Health Center Server sends the response %s "
						"to the patient with username %s.\n",response,usr);
				}	
			}
			// Receive the request for available appointments.
			if ((bytesRecvd = recv(new_fd, buf, BUFFSIZE-1, 0)) == -1)
			{
				perror("second recv error!!!");
				exit(1);
			}
			else
			{
				buf[bytesRecvd] = '\0';
				token=strtok(buf," ");
				strcpy(reqType,token);
				if(!strcmp(reqType,"available"))
				{	
					printf("Phase 2: The Health Center Server, receives a request for "
						"available time slots from the patients with port number"
						":%u and ip address %s\n",((struct sockaddr_in*)&their_addr)->sin_port,s);
					
					fp2=fopen("availabilities.txt","r");
					if(NULL==fp2)
					{
						printf("ERROR: fopen() returning NULL for availabilities.txt\n");
						return 1;
					}
					for(i=1;i<=6;i++)
					{	
						fgets(temp,50,fp2);
						if(reserved[i]!=1)
						{
							//set the reserved value after getting the index reply from patient. not here
							token=strtok(temp," ");							
							strcat(authRsp,token);
							strcat(authRsp," ");
							token=strtok(NULL," ");							
							strcat(authRsp,token);
							strcat(authRsp," ");
							token=strtok(NULL," ");							
							strcat(authRsp,token);
							strcat(authRsp," ");
						}
					}	
					if(!strcmp(authRsp,""))
					{
						strcpy(authRsp,"failure");
					}
					//sending the list of available appointments				
					if (send(new_fd,authRsp,strlen(authRsp), 0) == -1)
							perror("second send error!!!");
					else
					{
						printf("Phase 2: The Health Center Server sends available time slots to "
							"patient with username %s\n",usr);
					}	
				}
				else
				{	//if the request name is other than "available" then return invalid
					if (send(new_fd,"invalid",strlen("invalid"), 0) == -1)
							perror("third send error!!!");
				}
			}
			//receive the selected index among the list of appointments available
			if ((bytesRecvd = recv(new_fd, buf, BUFFSIZE-1, 0)) == -1)
			{
				perror("second recv error!!!");
				exit(1);
			}
			else
			{
				buf[bytesRecvd] = '\0';
				idxSelected=strtok(buf," ");
				idxSelChar=idxSelected;				
				idxint= atoi(idxSelected);				
				printf("Phase 2: The Health Center Server receives a request for "
					"appointment %d from patient with port number %u and "
					"username %s\n",idxint,((struct sockaddr_in*)&their_addr)->sin_port,usr);				
				if(reserved[idxint]==0)
				{	
					rewind(fp2);	
					for(j=1;j<=idxint;j++)
					{	
						fgets(temp,50,fp2);
												
						if(reserved[j]!=1)
						{
							token=strtok(temp," ");
							if(j==atoi(token))
							{
								//I got day for the token
								token = strtok(NULL," ");
								//I got time for the token
								token = strtok(NULL," ");
								//I got doctor for the token
								token = strtok(NULL," ");
								strcpy(docDetails,token);
								//I got port for the token
								token=strtok(NULL," ");
								strcat(docDetails," ");
								strcat(docDetails,token);
							}
							//set the selected appointment to reserved
							reserved[j]=1;	
						}
						
					}
					//if the requested appointment is not available reply "notavailable"
					if(j==idxint && !strcmp(docDetails,""))
						strcpy(docDetails,"notavailable");
				}
				else
				{
					//if the requested appointment is not available reply "notavailable"
					strcpy(docDetails,"notavailable");
				}
				//sending the doctor name and port number to patient
				if (send(new_fd,docDetails,strlen(docDetails), 0) == -1)
					perror("third send error!!!");
				else
				{	
					//if appointment is available then print confirm else print reject
					if(strcmp(docDetails,"notavailable"))
					{
					printf("Phase 2: The Health Center Server confirms the following "
						"appointment %d to patient with username %s.\n",idxint,usr);
					}
					else
					{					
					printf("Phase 2: The Health Center Server rejects the following "
						"appointment %d to patient with username %s.\n",idxint,usr);
					}
									
					printf("\nPhase 1: The Health Center Server has the "
					       "port number %s and ip address %s.\n",SRVPORT,ipstr);

				}
			}
			close(new_fd);
			exit(0);
		}
		close(new_fd);
		
	}//end of while
	return 0;	
}
