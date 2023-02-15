#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string
char *compute_get_request(char *host, char *url, char *cookie, char *target);

// computes and returns a POST request string
char *compute_post_request(char *host, char *url, char *content_type, 
                                            	char *target, char *jwt);

// computes and returns a DELETE request string
char *compute_delete_request(char *host, char *url, char *jwt);

#endif
