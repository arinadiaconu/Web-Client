#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *cookie, char *target)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // writes the method name, URL and protocol type
    sprintf(line, "GET %s HTTP/1.1", url);

    compute_message(message, line);

    // adds the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adds the cookie
    if (cookie != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Cookie: ");

        strcat(line, cookie);
        strcat(line, ";");

        compute_message(message, line);
    }

    // adds the target
    if (target != NULL) {
        memset(line, 0, LINELEN);
        strcat(line, "Authorization: Bearer ");

        strcat(line, target);
        compute_message(message, line);
    }

    // adds new line
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, 
                                            char *target, char *jwt) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // writes the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // adds the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    //adds the target in authorization
    if (jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    // adds payload length
    int length = strlen(target);
    sprintf(line, "Content-Length: %d", length);
    compute_message(message, line);

    compute_message(message, "");   

    // adds payload
    memset(line, 0, LINELEN);
    strcat(message, target);


    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *jwt) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // writes the method name, URL and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);

    compute_message(message, line);

    // adds the host
    memset(line, 0, LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    //adds the target in authorization
    if (jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // adds new line
    compute_message(message, "");

    free(line);
    return message;
}
