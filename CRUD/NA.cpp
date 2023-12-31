/* Client code in C */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <iostream>
  
  using namespace std;
 
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
 
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(9034);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    //separar buffer con wri sino nunca sale del
    char wri[256];
    char buffer[256];
    string response;

    do{
      	response="";
      	
      	
      	n = read(SocketFD,buffer,1);
      
      	//size campo
      	n = read(SocketFD,buffer,5);
	buffer[n] = '\0';
	int tam = atoi(buffer); 
	//response+=buffer;
	//cout<<response<<endl;
	
	//campo
	n = read(SocketFD,buffer,tam);
	buffer[n] = '\0';
	response+=buffer;
	
	//size data
	n = read(SocketFD,buffer,5);
	buffer[n] = '\0';
	tam = atoi(buffer); 
	//response+=buf;
	//cout<<response<<endl;
	response+=":";
	//data
	n = read(SocketFD,buffer,tam);
	buffer[n] = '\0';
	response+=buffer;
	cout<<response<<endl;

      
    }
    
    while(strcmp(buffer,"chau\n"));
    printf("fin de conexion");
    shutdown(SocketFD, SHUT_RDWR);
 
    close(SocketFD);
    return 0;
  }
