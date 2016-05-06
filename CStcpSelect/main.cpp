/*
 Client/Server
 This program will connect to the Signon server using a separate thread 
 It will then use select to listen for messages from peers to begin P2P communication
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <map>
#include "cipher.h"
#include <string>
#include <iostream>
#include <thread>
#include "signOnServer.h"


#define PORT "5490"   // port we'll connecting to signon server with
char remoteIP[INET6_ADDRSTRLEN];

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// get port, IPv4 or IPv6:
in_port_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return (((struct sockaddr_in*)sa)->sin_port);
    }
    
    return (((struct sockaddr_in6*)sa)->sin6_port);
}


//map that hold users
typedef std::pair<struct sockaddr_storage, int> pair_addr;
std::map<std::string, pair_addr> chatUsers;

//check to see if user with Username is already signed in
bool userExists(std::string username, std::map<std::string, pair_addr> chatUsers){
    for(auto user : chatUsers)
    {
        if (user.first == username) return true;
    }
    return false;
}
/*
 encryption function
 */
std::string crypt(std::string msg)
{
    std::string eMsg = "";
    int i = 0;
    while (i < msg.length())
    {
        eMsg += encr[msg[i]];
        i++;
    }
    return eMsg;
}

/*
 Decryption function
 Assumption: msg format will be: 1;clientName;msgPort#
 */
std::string* deCrypt(char* msg)
{
    std::string * rMsg = new std::string[2];
    // empty msg
    if (msg[0] == '\0')
    {
        rMsg[0] = "empty reply";
        rMsg[1] = "empty reply";
        return rMsg;
    }
    
    int i = 2; //ignore first 2 characters
    int numSemi = 0;
    //get username
    while ( msg[i] != '\0' && numSemi < 1)
    {
        if (msg[i] == '5'){
            numSemi++;
            i++;
            break;
        }
        rMsg[0] += decr[ msg[i] ];
        i++;
    }
    while ( msg[i] != '\0' && msg[i] != '#')
    {
        
        rMsg[1] += decr[ msg[i] ];
        i++;
    }
    return rMsg;
}


//return all users line separated as c string
std::string getAllUsers(std::map<std::string, pair_addr> chatUsers)
{
    return "bob";
}


int main(void)
{
    //link up other thread to signOnServer
    std::thread t(connectToSignOnServer);
    t.detach();
    
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number
    
    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    
    char buf[256];    // buffer for client data
    int nbytes;
    
    //char remoteIP[INET6_ADDRSTRLEN]; moved global
    
    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;
    
    struct addrinfo hints, *ai, *p;
    
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);
    
    struct timeval tv; //setting timeout for select function
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    
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
        
        // Allow listoner to handle multiple connections
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
    for(;;) {
        
        char consoleInput;
        std::cout << "End of one select loop. Would you like to send a message?";
        std:: cin >> consoleInput;
        
        if (consoleInput == 'y'){
            std::cout << "\ninside create socket \n";
            int socket_desc;
            struct sockaddr_in server;
            
            //Create socket
            socket_desc = socket(AF_INET , SOCK_STREAM , 0);
            if (socket_desc == -1)
            {
                printf("Could not create socket");
            }
            
            server.sin_addr.s_addr = inet_addr("127.0.0.1");
            server.sin_family = AF_INET;
            server.sin_port = htons( 3490 );
            
            //Connect to remote server
            if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
            {
                puts("connect error");
                return 1;
            }
            
            std::string clientName = "chris";
            std::string signOn = "1;" + crypt2( clientName + ";msgPort") + "#";
            char* cSignOn = const_cast<char*>(signOn.c_str());
            
            if (send(socket_desc, cSignOn, strlen(cSignOn), 0) == -1)
                perror("send");
            FD_SET(socket_desc, &master);
            fdmax = socket_desc;
        }
        std::cin.ignore();
        
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1) {
            perror("select");
            exit(4);
        }
        
        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,(struct sockaddr *)&remoteaddr,&addrlen);
                    
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        
                        inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr), remoteIP, INET6_ADDRSTRLEN);
                        printf("selectserver: new connection from %s on ""socket %d\n", remoteIP,newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        //terminate data from connection
                        buf[sizeof buf] = '\0';
                        std::string* message = deCrypt(buf);
                        //chatUsers.insert( std::make_pair(message[0], usersAddr) );
                        printf("Received username: %s msg: %s\n", message[0].c_str(), message[1].c_str());
                        fflush(stdout);
                        const char* msgNum = "4;";
                        
                        std::string welcome = "4;" +
                        crypt("Welcome to the group\n Users Loggedin:\n" +  getAllUsers(chatUsers));
                        char* cWelcome = const_cast<char*>(welcome.c_str());
                        if (send(i, cWelcome, strlen(getAllUsers(chatUsers).c_str()), 0) == -1)
                            perror("send");
                        
                        //let every know that somone has joined the chat
                        for(j = 0; j <= fdmax; j++) {
                            if (FD_ISSET(j, &master)) {
                                // except the server and the client who just joined
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
       
    } // END for(;;)
    
    return 0;
}