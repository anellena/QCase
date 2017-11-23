/*
 * Mock a client sending retrieve requests to the server every 15 minutes.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#define SERVER_PORT 6423
#define BUFFER_SIZE 64*1024

#define N_UPDATE_TESTS	8
char *testRetrieveFiles[N_UPDATE_TESTS] = {
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveAll.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveSome.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveExtraKeys.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveNotStored.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveInvalid.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveNoKeys.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveWrongParent.xml",
	"/home/anellena/QAssignment/QCase/tests/requestClients/retrieveKeyEmpty.xml"
};

static int sendtestFile(int fileIdx, int sockfd) {
	FILE *testXml = fopen(testRetrieveFiles[fileIdx], "r");

	if (testXml == NULL) {
		printf("Could not open file for reading: %m\n");
		return -1;
	}

	fseek(testXml, 0, SEEK_END);
	int size = ftell(testXml);
	fseek(testXml, 0, SEEK_SET);
	printf("File size: %d\n", size);

	char *xmlContent = malloc((size+1)*sizeof(*xmlContent));

	fread(xmlContent, size, 1, testXml);
	xmlContent[size] = '\0';
	printf("XML content: %s\n", xmlContent);

	if (send(sockfd, xmlContent, size, 0) <= 0) {
		printf("Could not send data to server.\n");
	}

	free(xmlContent);
	fclose(testXml);

	// Receive response
	char buffer[BUFFER_SIZE];
	int received = recv(sockfd, &buffer, BUFFER_SIZE, 0);
	buffer[received] = '\0';
	if (received > 0){
		printf("Response XML: %s.\n", buffer);
	}
	else {
		printf("No response.\n");
	}

	return 0;
}

static int testUpdate() {
	for (int i=0; i<N_UPDATE_TESTS; i++) {
		printf("---------------- BEGIN TEST %d ----------------\n", i);
		printf("File name: %s\n", testRetrieveFiles[i]);

		struct sockaddr_in servAddr;

		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1){
			printf("ERROR opening socket: %m\n");
			return -1;
		}

		memset(&servAddr, 0, sizeof(servAddr));

		printf("Connecting... ");
		// Fflush here forces that the printf is printed immediately
		fflush(stdout);

		struct hostent *server = gethostbyname("127.0.0.1");
		servAddr.sin_family = AF_INET;
		servAddr.sin_port = htons(SERVER_PORT);
		memcpy(&servAddr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
		if (connect(sockfd,(struct sockaddr *) &servAddr, sizeof(servAddr)) == -1){
			printf("Error connecting to the server %m\n");
			return -1;
		}
		printf("Connected!\n");

		if (sendtestFile(i, sockfd) < 0) {
			printf("Could not send file.\n");
		}

		sleep (10);

		close(sockfd);
		printf("Closed connection\n");
		printf("---------------- END TEST %d ----------------\n\n", i);
	}
	return 0;
}

int main()
{
	testUpdate();

	return 0;
}
