#include <sys/types.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <map>
#include <fstream>
#include <iostream>

using namespace std;

#define PORT "9034" // port we're listening on
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void){
	fd_set master; // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int fdmax; // maximum file descriptor number
	int listener; // listening socket descriptor
	int identificador;
	
	//mapa que va a identificar que file es de que thread, el int es el socket, string el mensaje
	map<int,string> files;
	
	//vector de threads para cada conexion de socket
	vector<thread> socket_threads;
	
	int newfd; // newly accept()ed socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	socklen_t addrlen;
	char buf[256]; // buffer for client data
	int nbytes;
	char remoteIP[INET6_ADDRSTRLEN];
	int yes=1; // for setsockopt() SO_REUSEADDR, below
	int i, j, rv,count=0;
	string response;
	struct addrinfo hints, *ai, *p;
	
	FD_ZERO(&master); // clear the master and temp sets
	FD_ZERO(&read_fds);
	
	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) 
	{
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
	
	
	// main loop
	for(;;) 
	{
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
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
					if (newfd > fdmax) { // keep track of the max
					fdmax = newfd;
					}
					printf("selectserver: new connection from %s on "
					"socket %d\n",
					inet_ntop(remoteaddr.ss_family,
					get_in_addr((struct sockaddr*)&remoteaddr),
					remoteIP, INET6_ADDRSTRLEN),
					newfd);

					
					}
				} 
				else {
				
					// handle data from a client
					if ((nbytes = recv(i, buf, 9 , 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
						// connection closed
						printf("selectserver: socket %d hung up\n", i);
						} 
						else {
						perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} 
					else {
					// we got some data from a client
						//nbytes = recv(i, buf, 1 , 0);
						buf[nbytes] = '\0';
						string id = "";
						id += buf[nbytes-1];
						identificador = atoi(id.c_str());
						
						if(files[i]=="\0") {
							files[i] = " ";
							cout<<"a"<<endl;
							}
						
						response = "";
						response +=to_string(count);count++;
						response +=":";
						response += buf;
						response +=":";
						nbytes = recv(i, buf, 1 , 0);//:
						buf[nbytes] = '\0';
						nbytes = recv(i, buf, 7 , 0);
						buf[nbytes] = '\0';
						response += buf;
						response +=":";
						int tam = atoi(buf); 
						nbytes = recv(i, buf, 1 , 0);//:
						buf[nbytes] = '\0';
						nbytes = recv(i, buf, tam , 0);
						buf[nbytes] = '\0';
						response+=buf;
						response +=":";
						nbytes = recv(i, buf, 1 , 0);//:
						buf[nbytes] = '\0';
						
						nbytes = recv(i, buf, 10 , 0);
						buf[nbytes] = '\0';
						response+=buf;
						response +="\n";
						nbytes = recv(i, buf, 1 , 0);//:
						buf[nbytes] = '\0';
						
						files[i] += response;
						//cout<<response<<endl;
						

					for(j = 0; j <= fdmax; j++) {
					// send to everyone
						if (FD_ISSET(j, &master)) {
							 //except the listener and ourselves
							if (j != listener && j != i) {
							if (send(j, response.c_str(),  strlen(response.c_str()), 0) == -1) {
							perror("send");
							}
							}
						}
					}
					}
				} // END handle data from client
			} // END got new incoming connection
			
			string nombreArchivo = "file_";
			nombreArchivo+=to_string(identificador);
			nombreArchivo+=".txt";
			ofstream archivo;
			// Abrimos el archivo
			archivo.open(nombreArchivo.c_str(), fstream::out);
			// Y le escribimos redirigiendo
			archivo << files[i];
			// Finalmente lo cerramos
			archivo.close();
		
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!
	return 0;
}
