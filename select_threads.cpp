#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include <fstream>
#include <map>
#include <queue>
#include <mutex>
#include <iostream>
#include <mutex>  
#include <sstream>

using namespace std;


#define PORT "9034"   // port we're listening on

fd_set master;    // master file descriptor list
fd_set read_fds;  // temp file descriptor list for select()

const int buff_tam =66000;

 map<int, mutex> locks; 
 map<int,  queue<char *>> map_information;
 map<int,  string> response; 
 map<int,int> IDS;
 map<int,bool> linea_completa; //si el thread esta en una linea partida a la mitaf



// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void read_info(int socket_id){
    int nbytes;
    char id[10];
    char data[100];
    char chsum[11]; 

    int idCounter=0;
    
    //ID+size+chsum+nro de ':'
    int sizeNoData=9+7+10+3;
    int totalSize;
    string line_backup; 
    locks[socket_id].unlock();
			
    while(true){
  
        while (!map_information[socket_id].empty()) {
            //copia
            char *buffer3 = strdup(map_information[socket_id].front());
            //vacía el string del socket i
            map_information[socket_id].pop();

            //copiar la info a un objeto de istringstream
            //para poder pasarlo al getline
             istringstream Tss(buffer3);
             string tokenLine;
             
            while ( getline(Tss,tokenLine)) {
                //copia la info a line_backup
                line_backup = tokenLine;
                //copia tokenLine a liness que es un objeto del tipo istringstream
                 istringstream liness(tokenLine);
                 string tkn;

                //insertar 00001 en idCounterStr
                 string idCounterStr( to_string(IDS[socket_id]));
                idCounterStr.insert(0,5-idCounterStr.length(),'0');

                //si es que ese socket no esta siendo utlizado
                if(!linea_completa[socket_id]){
                    response[socket_id]=response[socket_id]+idCounterStr;
                    IDS[socket_id]++;
                }
                //file<<idCounterStr;

                int contador_parseo=0;

                //indicar que el final de cada linea será :
                //000001:00003:283 seria:
                //000001
                //00003
                //283

                while ( getline(liness,tkn,':')) {
                    if(contador_parseo==1){
                        //sizeNoData=9+7+10+3
                        //000000009 id
                        //0000045 size, por eso lo convertimos a stoi tkn ya que es la segunda iteracion contador_parseo 1
                        totalSize= sizeNoData +  stoi(tkn);
                    }
                    if(contador_parseo ++== 0 && linea_completa[socket_id]==1)
                    {
                        //si nos hemos quedado a la mitad de una lectura no colocamos los dos puntos
                        response[socket_id]=response[socket_id]+tkn;
                        continue;
                    }
                    //agregamos el parseo
                    response[socket_id]=response[socket_id]+":"+tkn;
                } 
                if(totalSize==tokenLine.length()||linea_completa[socket_id]==1)
                {
                    //si hemos llegado al final de la linea
                    response[socket_id]=response[socket_id]+"\n";
                    linea_completa[socket_id]=0;
                }
                else
                {
                    //si nos hemos quedado a la mitad de una lectura
                    linea_completa[socket_id]=1;
                }
            }
        }
        //bloqueamos el mutex del socket correspondiente
        locks[socket_id].lock();
    }
}


int main(){
    char buffer[buff_tam];    // buffer for client data
    int fdmax;        // maximum file descriptor number
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    int nbytes;
	  char remoteIP[INET6_ADDRSTRLEN];
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

	  struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
     vector< thread> thrds;

    // main loop
    int ready;
    for(;;) {
        read_fds = master; // copy it
        if ((ready=select(fdmax+1, &read_fds, NULL, NULL, NULL)) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
    "socket %d\n",inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr),remoteIP, INET6_ADDRSTRLEN),newfd);
    
    		        //creamos el nuevo campo del mapa con el valor del socket
                        map_information[newfd].push("");
                        //inicializamos su response que es el string donde acumularemos todo
                        response[newfd]=""; 
                        
                        locks[newfd].unlock();
                        
                        //lanzamos su thread
                        thrds.push_back( thread(read_info,newfd));
                    }
                } 
                  
                else {
                
                    //cantidad a leer
                    int maxR = 10000;
                    int n = read(i, buffer, maxR);
                    
                    if(n!=0)
                    {
                        buffer[n] = '\0';
                        
                        //desbloqueamos el mutex del socket correspondiente
                        locks[i].unlock();
			
			//almacenamos la info en el apartado del socket i del map_information
                        map_information[i].push(strdup(buffer));
                        
                        printf("%s\n",buffer);

                    }
                    else{

                        for(int j=5; j<=fdmax; j++){

			     //abrimos el file correspondiente a la data
                             fstream file("Files/file"+ to_string(j-5)+".txt", ios::out);
                             //escribimos en el file
                            file<<response[j];
                            
                            close(j);
                            FD_CLR(j,&master);
                        }
                    }

                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}
