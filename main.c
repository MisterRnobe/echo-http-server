#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define PORT 5000
#define MAX 4096

char headers[] = "HTTP/1.1 200\n"
                 "X-Content-Type-Options: nosniff\n"
                 "X-XSS-Protection: 1; mode=block\n"
                 "Cache-Control: no-cache, no-store, max-age=0, must-revalidate\n"
                 "Pragma: no-cache\n"
                 "Expires: 0\n"
                 "X-Frame-Options: DENY\n"
                 "Content-Type: application/json;charset=iso-8859-1\n\n";

void handleRequest(int connfd);

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(int argc, char *argv[]) {
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    time_t ticks;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    printf("Server has started...\n");
    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);
    while (1) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
        ticks = time(NULL);
        //snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        handleRequest(connfd);
        close(connfd);
        sleep(1);
    }
}

void handleRequest(int connfd) {
    char buff[MAX];
    bzero(buff, MAX);
    // read the message from client and copy it in buffer
    read(connfd, buff, sizeof(buff));

    //
    char *textParam;
    int i = 4, indexStart = -1, indexEnd = -1;
    for (i = 0; buff[i] != '\n'; i++) {
        if (buff[i] == '?') {
            for (; buff[i] != '='; i++);
            indexStart = i + 1;
            for (; buff[i] != ' '; i++);
            indexEnd = i;
        }
    }
    //
    char *response = malloc(sizeof(headers));
    memcpy(response, headers, sizeof(headers));

    if (indexStart > 0) {
        int len = indexEnd - indexStart;
        textParam = malloc(sizeof(char) * (len + 1));
        memcpy(textParam, &buff[indexStart], len);
        textParam[len] = '\0';
        free(response);
        response = concat(headers, textParam);
        free(textParam);
    } else {
        free(response);
        response = concat(headers, "Hello world");
    }


    printf("Received:\n%s\n", buff);
    printf("Response:\n%s\n", response);
    write(connfd, response, strlen(response));
}