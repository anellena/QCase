/*
 * TODO - file description
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ezxml.h>
#include <assert.h>

#define TAG_UPDATE 	 "update"
#define TAG_RETRIEVE "retrieve"
#define TAG_STATUS   "status"

enum requestType {
	UpdateRequest,
	RetrieveRequest
};

typedef enum requestType _requestType;

#define STORED_XML_FILENAME "/home/anellena/QAssignment/QCase/tests/storedXml.xml"

// TODO - document function
int processUpdateRequest(ezxml_t storedXml, ezxml_t receivedXml) {
	FILE *xmlFile;
	ezxml_t nextChildTag;

	printf("Will process update request\n");

	nextChildTag = receivedXml->child;
	if (nextChildTag == NULL) {
		printf("Received XML is invalid\n");
		return -1;
	}

	//open stored XML file for writing - move to inside update function
	xmlFile = fopen(STORED_XML_FILENAME, "w");
	if (xmlFile == NULL) {
		printf("Could not open file for writing.\n");
		ezxml_free(storedXml);
		return -1;
	}

	printf("%d\n", __LINE__);

	while (nextChildTag) {
		char *tagName = nextChildTag->name;
		printf("Next child name: %s - %d\n", nextChildTag->name, __LINE__);
		ezxml_t storedTag = ezxml_get(storedXml, tagName, -1);

		if (storedTag == NULL) {
			// create new tag
			ezxml_t newTag = ezxml_add_child(storedXml, nextChildTag->name, 0);
			newTag = ezxml_set_txt(newTag, nextChildTag->txt);
		}
		else {
			// update existing tag
			ezxml_set_txt(storedTag, nextChildTag->txt);
		}
		nextChildTag = nextChildTag->sibling;
	}

	char *updatedXml = ezxml_toxml(storedXml);
	if (updatedXml == NULL)
		return -1;

	printf("Updated XML: %s\n", updatedXml);

	// TODO - format XML before writing
	fwrite(updatedXml, strlen(updatedXml)*sizeof(char), 1, xmlFile);

	free(updatedXml);
	fclose(xmlFile);
	return 0;
}

// TODO - this is not read yet
int processRetrieveRequest(ezxml_t storedXml, ezxml_t receivedXml, char **xmlResponseBuffer) {
	// TODO
	return 0;
}


int processRequest(ezxml_t receivedXml, _requestType request, char **xmlResponseBuffer) {
	FILE *storedXml;

	//open stored XML file for reading
	storedXml = fopen(STORED_XML_FILENAME, "r");
	if (storedXml == NULL) {
		printf("Could not open file for reading.\n");
		return -1;
	}

	ezxml_t storedXmlParsed = ezxml_parse_fp(storedXml);
	fclose(storedXml);
	if (storedXmlParsed == NULL) {
		printf("Could not parse file.\n");
		return -1;
	}

	int result = 0;
	if (request == UpdateRequest) {
		result = processUpdateRequest(storedXmlParsed, receivedXml);
	}
	else if (request == RetrieveRequest) {
		result = processRetrieveRequest(storedXmlParsed, receivedXml, xmlResponseBuffer);
	}

	ezxml_free(storedXmlParsed);
	return result;
}

// TODO - document function
int processXml(char *xmlContent, int xmlLen, char **xmlResponseBuffer) {
	ezxml_t receivedXml;
	int result = 0;

	assert(xmlContent != NULL);

	receivedXml = ezxml_parse_str(xmlContent, xmlLen);
	if (receivedXml != NULL) {
		printf("First child: %s\n", receivedXml->name);

		if (strcmp(TAG_UPDATE, receivedXml->name) == 0) {
			result = processRequest(receivedXml, UpdateRequest, xmlResponseBuffer);
			*xmlResponseBuffer = NULL;
		}
		else if (strcmp(TAG_RETRIEVE, receivedXml->name) == 0)
			result = processRequest(receivedXml, RetrieveRequest, xmlResponseBuffer);
		else
			result = -1;
	}
	else {
		//log error
		result = -1;
	}

	return result;
}


