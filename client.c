#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson-master/parson.h"
#include "parson-master/parson.c"

#define HOST "34.241.4.235"
#define REGISTER_ACCESS_ROUTE "/api/v1/tema/auth/register"
#define LOGIN_ACCESS_ROUTE "/api/v1/tema/auth/login"
#define LIBRARY_ACCESS_ROUTE "/api/v1/tema/library/access"
#define ALL_BOOKS_ACCESS_ROUTE "/api/v1/tema/library/books"
#define ADD_BOOK_ACCESS_ROUTE "/api/v1/tema/library/books"
#define LOGOUT_ACCESS_ROUTE "/api/v1/tema/auth/logout"
#define PAYLOAD_TYPE "application/json"
#define STRING_LENGTH 30
#define LENGTH 600

void compute_register_request(int sockfd) {
    char *message;
    char *response;
    char *username = (char *)malloc(sizeof(char));
    char *password = (char *)malloc(sizeof(char));

    printf("username=");
    scanf("%s", username);

    printf("password=");
    scanf("%s", password);

    // uses functions from parser.h in order to create a json object
    JSON_Value *my_value = json_value_init_object();
    JSON_Object *my_object = json_value_get_object(my_value);

    json_object_set_string(my_object, "username", username);
    json_object_set_string(my_object, "password", password);

    // converts the json to string
    char *target = json_serialize_to_string_pretty(my_value);

    message = compute_post_request(HOST, REGISTER_ACCESS_ROUTE, PAYLOAD_TYPE,
                                                                target, NULL);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    char *string = "error";
    if (strstr(response, string)) {
        printf("The username is already taken!\n");
        printf("Please register with another username!\n");
    } else {
        printf("200 - OK - User is successfully registered!\n");
    }
}

char* compute_login_request(int sockfd) {
    char *message;
    char *response;
    char *username = (char *)malloc(sizeof(char));
    char *password = (char *)malloc(sizeof(char));

    printf("username=");
    scanf("%s", username);

    printf("password=");
    scanf("%s", password);

    // uses functions from parser.h in order to create a json object
    JSON_Value *my_value = json_value_init_object();
    JSON_Object *my_object = json_value_get_object(my_value);

    json_object_set_string(my_object, "username", username);
    json_object_set_string(my_object, "password", password);

    // converts the json to string
    char *target = json_serialize_to_string_pretty(my_value);

    message = compute_post_request(HOST, LOGIN_ACCESS_ROUTE, PAYLOAD_TYPE,
                                                                target, NULL);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    char *string = "error";
    char *error_message = strstr(response, string);
    if (error_message) {
        if(strstr(error_message, "Credentials")) {
            printf("Incorrect credentials!\n");
            printf("Please try again!\n");
        } else {
            printf("There is no account with this username!\n");
            printf("Please register and then come back!\n");
        }
    } else {
        printf("200 - OK - User is successfully logged in!\n");

        // extracts the cookie from the server's response
        char *cookie = strstr(response, "Set-Cookie: ");
        cookie = strtok(cookie, ";");
        cookie = strtok(cookie, " ");
        cookie = strtok(NULL, " ");
        return cookie;
    }

    return NULL;
}

char* compute_enter_library_request(int sockfd, char *cookie) {
    char *message;
    char *response;

    message = compute_get_request(HOST, LIBRARY_ACCESS_ROUTE, cookie, NULL);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    
    // extracts the jwt from the server's response
    char *jwt = strstr(response, "token");
    jwt = strtok(jwt, "\"");
    jwt = strtok(NULL, "\"");
    jwt = strtok(NULL, "\"");

    if (jwt) {
        return jwt;
    }
    return NULL;
}

// extracts the books from the server's response
char *extract_payload(char *server_response) {
    char *response = strstr(server_response, "timeout");
    response = strtok(response, "\n");
    response = strtok(NULL, "\n");
    response = strtok(NULL, "\n");
    return response;
}

void compute_get_books_request(int sockfd, char *jwt) {
    char *message;
    char *response;

    message = compute_get_request(HOST, ALL_BOOKS_ACCESS_ROUTE, NULL, jwt);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    response = extract_payload(response);
    puts(response);
}

// verifies if a string represents a number or not
// without looking at the last character
int verify_number_without_last(char *str) {
    int l = strlen(str) - 1;
    while(l--) {
        if(str[l] >= '0' && str[l] <= '9') {
            continue;
        }
        return 0; // is not a number
    }
    return 1;     // is a number
}

// verifies if a string represents a number or not
int verify_number(char *str) {
    int l = strlen(str);
    while(l--) {
        if(str[l] >= '0' && str[l] <= '9') {
            continue;
        }
        return 0; // is not a number
    }
    return 1;     // is a number
}

void compute_get_book_request(int sockfd, char *jwt) {
    char *message;
    char *response;

    char *book_id = (char *)malloc(sizeof(char));
    printf("id=");
    scanf("%s", book_id);

    if (!verify_number(book_id)) {
        printf("Invalid ID!\n");
    } else {
        char *route = (char *)calloc(40, sizeof(char));
        char str[100] = "/api/v1/tema/library/books/";
        route = strcat(str, book_id);

        message = compute_get_request(HOST, route, NULL, jwt);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        char *error = "error";
        if (strstr(response, error)) {
            printf("Book not found! Please try again!\n");
        } else {
            printf("200 - OK - Here is your book:\n");

            response = extract_payload(response);
            puts(response);
        }
    }
    free(book_id);
}

void compute_add_book_request(int sockfd, char *jwt) {
    char *message;
    char *response;
    char *title = (char *)malloc(LENGTH * sizeof(char));
    char *author = (char *)malloc(LENGTH * sizeof(char));
    char *genre = (char *)malloc(LENGTH * sizeof(char));
    char *publisher = (char *)malloc(LENGTH * sizeof(char));
    char *page_count = (char *)malloc(LENGTH * sizeof(char));

    fgets(title, LENGTH - 1, stdin);

    printf("title=");
    memset(title, 0, LENGTH);
    fgets(title, LENGTH - 1, stdin);

    printf("author=");
    memset(author, 0, LENGTH);
    fgets(author, LENGTH - 1, stdin);

    printf("genre=");
    memset(genre, 0, LENGTH);
    fgets(genre, LENGTH - 1, stdin);

    printf("publisher=");
    memset(publisher, 0, LENGTH);
    fgets(publisher, LENGTH - 1, stdin);

    printf("page_count=");
    memset(page_count, 0, LENGTH);
    fgets(page_count, LENGTH - 1, stdin);

    if(!verify_number_without_last(page_count)) {
        printf("The page_count has an incorrect format!\n");
        printf("Please try again!\n");
    } else {
        // uses functions from parser.h in order to create a json object
        JSON_Value *my_value = json_value_init_object();
        JSON_Object *my_object = json_value_get_object(my_value);

        json_object_set_string(my_object, "title", title);
        json_object_set_string(my_object, "author", author);
        json_object_set_string(my_object, "genre", genre);
        json_object_set_string(my_object, "page_count", page_count);
        json_object_set_string(my_object, "publisher", publisher);

        // converts the json to string
        char *target = json_serialize_to_string_pretty(my_value);

        message = compute_post_request(HOST, ADD_BOOK_ACCESS_ROUTE,
                                            PAYLOAD_TYPE, target, jwt);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        char *error = "error";
        if (strstr(response, error)) {
            printf("Invalid or incomplete information for the book!\n");
            printf("Please try again!\n");
        } else {
            printf("200 - OK - Your book was successfully");
            printf(" added to the library!\n");
        }
    }
}

void compute_delete_book_request(int sockfd, char *jwt) {
    char *message;
    char *response;

    char *book_id = (char *)malloc(sizeof(char));
    printf("id=");
    scanf("%s", book_id);

    if (!verify_number(book_id)) {
        printf("Invalid ID!\n");
    } else {
        char *route = (char *)calloc(40, sizeof(char));
        char str[100] = "/api/v1/tema/library/books/";
        route = strcat(str, book_id);

        message = compute_delete_request(HOST, route, jwt);

        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);

        char *error = "error";
        if (strstr(response, error)) {
            printf("The book you are trying to delete does not exist!\n");
            printf("Please try again!\n");
        } else {
            printf("200 - OK - The book has been deleted!\n");
        }
    }

    free(book_id);
}

void compute_logout_request(int sockfd, char *cookie) {
    char *message;
    char *response;

    message = compute_get_request(HOST, LOGOUT_ACCESS_ROUTE, cookie, NULL);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);

    char *error = "error";
    if (strstr(response, error)) {
        printf("Logout failed! Please try again!\n");
    } else {
        printf("200 - OK - Logout successful!\n");
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    char *client_request = malloc(sizeof(char) * STRING_LENGTH);
    int login_just_once = 0;    // 0 - not logged in, 1 - logged in
    char *cookie, *jwt;
    int entered_library = 0; // 0 - not entered the library,
                             // 1 - entered the library

    while (1) {
        scanf("%s", client_request);
        sockfd = open_connection(HOST, 8080,
                                AF_INET, SOCK_STREAM, 0);

        if (!strcmp(client_request, "register")) {
            compute_register_request(sockfd);
        }
        else if (!strcmp(client_request, "login")) {
            if(login_just_once) {
                printf("User is already logged in!\n");
            } else if (!login_just_once) {
                cookie = compute_login_request(sockfd);
                if (cookie) {
                    login_just_once = 1;
                }
            }
        }
        else if (!strcmp(client_request, "enter_library")) {
            if (!login_just_once) {
                printf("No user is logged in!\n");
            } else {
                jwt = compute_enter_library_request(sockfd, cookie);
                printf("User has successfully entered the library!\n");
                entered_library = 1;
            }
        }
        else if(!strcmp(client_request, "get_books")) {
            if (!entered_library) {
                printf("User does not have access to the library!\n");
            } else {
                compute_get_books_request(sockfd, jwt);
            }
        }
        else if (!strcmp(client_request, "get_book")) {
            if (!entered_library) {
                printf("User does not have access to the library!\n");
            } else {
                compute_get_book_request(sockfd, jwt);
            }
        }
        else if (!strcmp(client_request, "add_book")) {
            if (!entered_library) {
                printf("User does not have access to the library!\n");
            } else {
                compute_add_book_request(sockfd, jwt);
            }
        }
        else if (!strcmp(client_request, "delete_book")) {
            if (!entered_library) {
                printf("User does not have access to the library!\n");
            } else {
                compute_delete_book_request(sockfd, jwt);
            }
        }
        else if (!strcmp(client_request, "logout")) {
            if(!login_just_once) {
                printf("No user is logged in at the moment!\n");
            } else {
                login_just_once = 0;
                entered_library = 0;
                compute_logout_request(sockfd, cookie);
            }
        }
        else if (!strcmp(client_request, "exit")) {
            break;
        }
        else {
            printf("Invalid request! Please try again!\n");
        }
    }

    close(sockfd);
    free(client_request);
    return 0;
}
