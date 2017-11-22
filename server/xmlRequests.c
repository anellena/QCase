/*
 * TODO - file description
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ezxml.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define TAG_UPDATE 	 "update"
#define TAG_RETRIEVE "retrieve"
#define TAG_STATUS   "status"
#define TAG_KEY      "key"

enum requestType {
	UpdateRequest,
	RetrieveRequest
};

typedef enum requestType _requestType;

#define STORED_XML_FILENAME "/home/anellena/QAssignment/QCase/tests/storedXml.xml"

/*
 * Iterate through the tags of the received XML to identify if tag is on it.
 */
bool tagInRetrieveRequest(ezxml_t receivedXml, ezxml_t tag) {
	ezxml_t nextTag = receivedXml->child;

	while(nextTag) {
		if (strcmp(TAG_KEY, nextTag->name) == 0 && strcmp(tag->name, nextTag->txt) == 0)
			return true;

		nextTag = nextTag->next;
	}

	return false;
}

/*
 * Process a xml to execute a retrieve request.
 */
int processRetrieveRequest(ezxml_t storedXml, ezxml_t receivedXml, char **xmlResponseBuffer) {
	ezxml_t nextStoredTag, nextReceivedTag;
	bool retrieveAll = false;

	assert(xmlResponseBuffer != NULL);

	printf("Processing retrieve request...\n");

	nextStoredTag = storedXml->child;
	if (nextStoredTag == NULL) {
		printf("There is no information stored to send.");
		return -1;
	}

	nextReceivedTag = receivedXml->child;
	// If no child, then means retrieve all
	if (nextReceivedTag == NULL) {
		retrieveAll = true;
	}

	ezxml_t responseXml = ezxml_new(TAG_STATUS);
	int countResponseTags = 0;
	while (nextStoredTag) {
		if (retrieveAll || tagInRetrieveRequest(receivedXml, nextStoredTag)) {
			ezxml_t newTag = ezxml_add_child(responseXml, nextStoredTag->name, 0);
			newTag = ezxml_set_txt(newTag, nextStoredTag->txt);
			countResponseTags++;
		}
		nextStoredTag = nextStoredTag->sibling;
	}

	if (countResponseTags == 0) {
		printf("Invalid request, none of the stored tags match the retrieve criteria.\n");
		return -1;
	}

	*xmlResponseBuffer = ezxml_toxml(responseXml);
	printf("Response XML: %s\n", *xmlResponseBuffer);


	printf("Finished update request.\n");
	return 0;
}

/*
 * Process a xml to execute an update request.
 */
int processUpdateRequest(ezxml_t storedXml, ezxml_t receivedXml) {
	FILE *xmlFile;
	ezxml_t nextReceivedTag;

	printf("Processing update request...\n");

	nextReceivedTag = receivedXml->child;
	if (nextReceivedTag == NULL) {
		printf("Received XML is invalid\n");
		return -1;
	}

	xmlFile = fopen(STORED_XML_FILENAME, "w");
	if (xmlFile == NULL) {
		printf("Could not open file for writing: %m.\n");
		ezxml_free(storedXml);
		return -1;
	}

	while (nextReceivedTag) {
		char *tagName = nextReceivedTag->name;

		// Make sure tag has a value
		if (nextReceivedTag->txt != NULL && (strcmp(nextReceivedTag->txt, "") != 0)) {
			ezxml_t storedTag = ezxml_get(storedXml, tagName, -1);

			if (storedTag == NULL) {
				// create new tag
				//printf("nextReceivedTag->name received: %s.\n", nextReceivedTag->name);
				ezxml_t newTag = ezxml_add_child(storedXml, nextReceivedTag->name, 0);
				newTag = ezxml_set_txt(newTag, nextReceivedTag->txt);
			}
			else {
				// update existing tag
				//printf("Existing tag: %s.\n", nextReceivedTag->name);
				ezxml_set_txt(storedTag, nextReceivedTag->txt);
			}
		}

		nextReceivedTag = nextReceivedTag->sibling;
	}

	char *updatedXml = ezxml_toxml(storedXml);
	if (updatedXml == NULL)
		return -1;

	printf("Updated XML: %s\n", updatedXml);

	// TODO - format XML before writing
	fwrite(updatedXml, strlen(updatedXml)*sizeof(char), 1, xmlFile);

	free(updatedXml);
	fclose(xmlFile);

	printf("Finished update request.\n");
	return 0;
}

/*
 * Initialize files and structures for executing requests.
 */
int processRequest(ezxml_t receivedXml, _requestType request, char **xmlResponseBuffer) {
	FILE *storedXml;

	//open stored XML file for reading
	storedXml = fopen(STORED_XML_FILENAME, "r");
	if (storedXml == NULL) {
		printf("Could not open file for reading: %m.\n");
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

/*
 * see xmlRequests.h
 */
int processXml(char *xmlContent, int xmlLen, char **xmlResponseBuffer) {
	ezxml_t receivedXml;

	assert(xmlContent != NULL);

	receivedXml = ezxml_parse_str(xmlContent, xmlLen);
	if (receivedXml == NULL || receivedXml->name == NULL) {
		return -1;
	}

	if (strcmp(TAG_UPDATE, receivedXml->name) == 0) {
		printf("Update request.\n");
		return processRequest(receivedXml, UpdateRequest, xmlResponseBuffer);
	}
	else if (strcmp(TAG_RETRIEVE, receivedXml->name) == 0) {
		printf("Retrieve request.\n");
		return processRequest(receivedXml, RetrieveRequest, xmlResponseBuffer);
	}
	else {
		printf("Invalid request\n");
		return -1;
	}
}


