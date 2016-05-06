//
//  signOnServer.h
//  CStcpSelect
//
//  Created by Chris Walter on 5/5/16.
//  Copyright © 2016 Chris Walter. All rights reserved.
//

#ifndef signOnServer_h
#define signOnServer_h


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

#define SIGN_ON_PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once


// get sockaddr, IPv4 or IPv6:
void *get_in_addr2(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
 encryption function
 */
std::string crypt2(std::string msg)
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
std::string deCryptSignOn(std::string msg)
{
    std::string decrypted;
    int i = 2; //ignore first 2 characters
    while (msg[i] != '#' ||  i < msg.length() ){
        decrypted += decr[ msg[i] ];
        i++;
    }
    
    return decrypted;
}



//this listend
void connectToSignOnServer(){
    char* signOnServerIP = "127.0.0.1";
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(signOnServerIP, SIGN_ON_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        std::string clientName = "chris";
        std::string signOn = "1;" + crypt2( clientName + ";msgPort") + "#";
        char* cSignOn = const_cast<char*>(signOn.c_str());
        if (send(sockfd, cSignOn, strlen(cSignOn), 0) == -1)
            perror("send");
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
    
    inet_ntop(p->ai_family, get_in_addr2((struct sockaddr *)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);
    
    while(1)
    {
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        printf("client: received '%s'\n",deCryptSignOn(buf).c_str());
    }
    
    
    
    
    
    freeaddrinfo(servinfo); // all done with this structure
    close(sockfd);

    
}


#endif /* signOnServer_h */
