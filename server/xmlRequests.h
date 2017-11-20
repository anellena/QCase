/*
 * Process a xml to identify the request type and execute the request.
 *
 * param in  xmlContent			the xml to be processed
 * param in  xmlLen				length of the xmlContent TODO - double check
 * param out xmlResponseBuffer	when request is type retrieve this is filled with
 * 								the xml reponse. In this case caller MUST free it.
 *
 */
int processXml(char *xmlContent, int xmlLen, char **xmlResponseBuffer);
