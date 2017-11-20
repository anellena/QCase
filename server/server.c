/*
 * TODO - file description
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xmlRequests.h"


int testUpdateRequest(char *filePath) {
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

	processXml(xmlContent, size, NULL);

	free(xmlContent);
	fclose(testXml);

	return 0;
}

int main() {
	// TODO - what would be the better way to work with this path?
	testUpdateRequest("/home/anellena/QAssignment/QCase/tests/measuringClients/updateOK.xml");

	printf("Starting the project.\n");

	return 0;
}
