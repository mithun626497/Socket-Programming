/******************************************************************************************
* This file perform the following functionalities	   	   	  		  *
* 1) Upon boot, doctor program will call fork system call to duplicate the address space  *
*    so that both doctor1 and doctor2 functionality can be handled by same code 	  *
* 2) If the process id is non zero then do the processing of doctor1 else if pid = 0 then  *
*    do the processing for doctor2.							  *
* 3) will load the contents of doc#.txt based on which process is invoking.    	   	  *
* 4) Doctor1 program will create a UDP socket and will be listening on static port 41198  *
* 5) Doctor2 program will create a UDP socket and will be listening on static port 42198  *					   	  *
* 3) Doctor program receives request messages from either of the patients 		  *
*    of the format 'insurance#'	eg: insurance1				  		  *
* 4) Doctor will check with his database doc#.txt and will respond with   		  *	 	
*    the estimated cost of appointment of the format '#cost' eg:30	  		  *
******************************************************************************************/

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

#define MYPORT "41198"		//listening port for doctor1 
#define MYPORT2 "42198"		//listening port for doctor2 
#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
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
  	FILE *fp;
	char temp[20],ins1[20],ins2[20],ins3[20],ins1val[20],ins2val[20],ins3val[20],response[10];
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv,pid;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN],docAddr[INET6_ADDRSTRLEN];
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	void *addr;
	//forking to create 2 processes
	pid=fork(); 
	if(pid!=0)
	{	
		//if pid !=0 then open doc1.txt for doctor1.
		fp=fopen("doc1.txt","r");
		if(NULL == fp)
		{
		 	printf("ERROR: fopen failed !!!\n");
			return 1;
		}

		fscanf(fp,"%s %s %s %s %s %s",ins1,ins1val,ins2,ins2val,ins3,ins3val);
		fclose(fp);
	}
	else 
	{
		//Else if pid==0 then open doc2.txt for doctor2.
		fp=fopen("doc2.txt","r");
		if(NULL == fp)
		{
		 	printf("ERROR: fopen failed !!!\n");
			return 1;
		}

		fscanf(fp,"%s %s %s %s %s %s",ins1,ins1val,ins2,ins2val,ins3,ins3val);
		fclose(fp);
	}
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if(pid!=0)
	{	
		/*create UDP socket with port number 41198*/	
		if ((rv = getaddrinfo("nunki.usc.edu", MYPORT, &hints, &servinfo)) != 0) 
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
		// loop through all the results and bind to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) 
		{
			//create socket
			if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
			{
				perror("doctor1: socket failed");
				continue;
			}
			//bind the socket
			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
			{
				close(sockfd);
				freeaddrinfo(servinfo);
				perror("doctor1: bind failed");
				continue;
			}
			break;
		}
		if (p == NULL) 
		{
			fprintf(stderr, "doctor1: failed to bind socket\n");
			freeaddrinfo(servinfo);
			return 2;
		}
	}
	else
	{			
		/*create UDP socket with port number 42198*/	
		if ((rv = getaddrinfo("nunki.usc.edu", MYPORT2, &hints, &servinfo)) != 0) 
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
		// loop through all the results and bind to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) 
		{
			if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
			{
				perror("doctor1: socket failed");
				continue;
			}
			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
			{
				close(sockfd);
				perror("doctor1: bind failed");
				continue;
			}
			break;
		}
		if (p == NULL) 
		{
			fprintf(stderr, "doctor1: failed to bind socket\n");
			freeaddrinfo(servinfo);
			return 2;
		}
	}
	if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
	    perror("getsockname error");
	else
	{	
		addr = &(sin.sin_addr);
		inet_ntop(p->ai_family,addr,docAddr,sizeof docAddr);
	}
	addr_len = sizeof their_addr;
	inet_ntop(p->ai_family,get_in_addr((struct sockaddr *)p->ai_addr),docAddr, sizeof docAddr);

	while(1)
	{	
		//receive the message with request having insurance#
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) 
		{
			perror("recvfrom failed");
			freeaddrinfo(servinfo);
			close(sockfd);	
			exit(1);
		}
		buf[numbytes] = '\0';	
		if(pid!=0)
		{
			printf("\nPhase 3: Doctor 1 has a static UDP port %d and IP "
				"address %s.\n",ntohs(sin.sin_port),docAddr);
		}
		else
		{
			printf("\nPhase 3: Doctor 2 has a static UDP port %d and IP "
				"address %s.\n",ntohs(sin.sin_port),docAddr);
		}		
		if(!strcmp(buf,ins1) || !strcmp(buf,ins2) || !strcmp(buf,ins3))
		{
			if(pid!=0)
			{	
				printf("Phase 3: Doctor 1 receives the request from the patient with "
					"port number %u and insurance plan %s.\n",((struct sockaddr_in*)&their_addr)->sin_port,buf);
			}
			else
			{
				printf("Phase 3: Doctor 2 receives the request from the patient with "
					"port number %u and insurance plan %s.\n",((struct sockaddr_in*)&their_addr)->sin_port,buf);
							
			}	
			//fill in the response based on the insurance type			
			if(!strcmp(buf,ins1))
				strcpy(response,ins1val);
			else if (!strcmp(buf,ins2))
				strcpy(response,ins2val);
			else
				strcpy(response,ins3val);	
		}
		else	
		{	
			//if insurance type is not one of 1,2 or 3 respond invalid
			strcpy(response,"invalid");
		}
		//send the response to the patient
		if ((numbytes = sendto(sockfd, response, strlen(response), 0,(struct sockaddr *)&their_addr, addr_len)) == -1) 
		{
			perror("UDP sendto failed");
			freeaddrinfo(servinfo);
			close(sockfd);	
			exit(1);
		}
		else
		{	
			if(strcmp(response,"invalid"))
			{
				if(pid!=0)
				{
					printf("Phase 3: Doctor 1 has sent estimated price %s$ to patient "
						"with port number %u\n",response,((struct sockaddr_in*)&their_addr)->sin_port);	
				}
				else
				{
					printf("Phase 3: Doctor 2 has sent estimated price %s$ to patient "
						"with port number %u\n",response,((struct sockaddr_in*)&their_addr)->sin_port);	
				}
			}
			else
			{	
				printf("Phase 3: Invalid insurance Type.\n");
			}
		}
	}
	
	freeaddrinfo(servinfo);
	close(sockfd);	
	return 0;
	
}
