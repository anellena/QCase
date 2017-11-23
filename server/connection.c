/*
 * This module handles the server connection with the clients.
 *
 */

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "xmlRequests.h"

#define MAX_CONN 4
#define BUFFER_SIZE 64*1024
#define PORT 6423

static int stop;

static void stop_server(int sig){
    (void)sig;
    stop = 1;
}

struct connected_socket {
    int used, fd;
};

static int connectedSocketGetMaxFD(struct connected_socket* cs){
    int maxFD = -1;
    for(int i = 0; i < MAX_CONN; i++){
        if (cs[i].used && cs[i].fd > maxFD){
            maxFD = cs[i].fd;
        }
    }
    return maxFD;
}

static void connectedSocketSetFDs(struct connected_socket* cs, fd_set* set){
    for(int i = 0; i < MAX_CONN; i++){
        if (cs[i].used){
            FD_SET(cs[i].fd, set);
        }
    }
}

static int connectedSocketAdd(struct connected_socket* cs, int newFD){
     for(int i = 0; i < MAX_CONN; i++){
        if (!cs[i].used){
            cs[i].used = 1;
            cs[i].fd = newFD;
            return 0;
        }
    }
    return -1;
}

static void connectedSocketProcessData(struct connected_socket* cs, fd_set* set){
    char buffer[BUFFER_SIZE];
    char *response = NULL;
    for(int i = 0; i < MAX_CONN; i++){
        if (cs[i].used && FD_ISSET(cs[i].fd, set)) {
            int received = recv(cs[i].fd, &buffer, BUFFER_SIZE, 0);
            if (received > 0){
            	buffer[received] = '\0';
                printf("Received %d bytes (%s) from %d.\n\n", received, buffer, cs[i].fd);
                int res = processXml(buffer, received, &response);
                if (res < 0) {
                	printf("Connection %d closed.\n\n", cs[i].fd);
                	close(cs[i].fd);
                	cs[i].used = 0;
                }
                if (response != NULL) {
                	send(cs[i].fd, response, strlen(response)+1, 0);
                	free(response);
                }
            }
            else {
                if(received == 0){
                    printf("Connection %d closed.\n\n", cs[i].fd);
                }
                else{
                    printf("Error receiving from %d: %m.\n", cs[i].fd);
                }
                close(cs[i].fd);
                cs[i].used = 0;
            }
        }
    }
}

static void connectedSocketClose(struct connected_socket* cs){
    for(int i = 0; i < MAX_CONN; i++){
        if(cs[i].used){
            close(cs[i].fd);
            cs[i].used = 0;
        }
    }
}

/*
 * Waits for a connection to receive a request.
 * Can handle up to a limited number of connections at the same time.
 */
int waitForConnection() {
    struct sockaddr_in addrServer;
    int listenSocket;
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1){
        printf("Listening socket couldn't be started: %m\n");
        return 1;
    }

    if(fcntl(listenSocket, F_SETFL, O_NONBLOCK) == -1){
        printf("Couldn't set the socket to nonblocking.\n");
        close(listenSocket);
        return 1;
    }

    struct connected_socket connData[MAX_CONN];
    memset(connData, 0, sizeof(struct connected_socket)*MAX_CONN);

    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addrServer.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr *)&addrServer, sizeof(struct sockaddr)) == -1){
        printf("Error binding the listening socket. Error: %m\n");
        return 1;
    }

    int maxQueue = 5;
    if(listen(listenSocket, maxQueue) == -1){
        printf("Error executing the listen in the listening thread. Error: %m\n");
        return 1;
    }
    signal(SIGINT, stop_server);

    struct timeval timeout;
    fd_set rfds;
    while(!stop){
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&rfds);
        int maxFD = connectedSocketGetMaxFD(connData);
        maxFD = listenSocket > maxFD ? listenSocket + 1 : maxFD + 1;
        FD_SET(listenSocket, &rfds);
        connectedSocketSetFDs(connData, &rfds);
        int res = select(maxFD, &rfds, NULL, NULL, &timeout);
        if (res > 0){
            if (FD_ISSET(listenSocket, &rfds)) {
                int newConnection = accept(listenSocket, NULL, NULL);
                if (newConnection != -1){
                    if (connectedSocketAdd(connData, newConnection) == -1) {
                        printf("Error, we have no more connection slots.\n\n");
                        close(newConnection);
                    }
                    else {
                        printf("New connection!\n\n");
                    }
                }
                else {
                    printf("Error accepting a new connection %m\n");
                }
            }
            connectedSocketProcessData(connData, &rfds);
        }
    }

    printf("Finishing the server.\n");
    connectedSocketClose(connData);
    close(listenSocket);

    return 0;
}
