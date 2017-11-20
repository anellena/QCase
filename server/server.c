/*
 * TODO - file description
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xmlRequests.h"


int testRequest(char *filePath) {
	FILE *testXml = fopen(filePath, "r");
	if (testXml == NULL) {
		printf("Could not open file: %m\n");
		return -1;
	}

	fseek(testXml, 0, SEEK_END);
	int size = ftell(testXml);
	fseek(testXml, 0, SEEK_SET);

	char *xmlContent = malloc(size*sizeof(*xmlContent));
	assert(xmlContent != NULL);

	fread(xmlContent, size, 1, testXml);
	printf("xml content: %s", xmlContent);

	char *response = NULL;
	processXml(xmlContent, size, &response);
	if (response != NULL)
		printf("Response: %s\n", response);

	free(xmlContent);
	fclose(testXml);

	return 0;
}

int main() {
	// TODO - what would be the better way to work with this path?
	testRequest("/home/anellena/QAssignment/QCase/tests/measuringClients/updateOK.xml");
	testRequest("/home/anellena/QAssignment/QCase/tests/requestClients/retrieveAll.xml");
	testRequest("/home/anellena/QAssignment/QCase/tests/requestClients/retrieveSome.xml");


	printf("Starting the project.\n");

	return 0;
}
