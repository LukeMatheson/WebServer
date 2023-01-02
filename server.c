#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

char * parseRequestHeader(char *header);

char * createResponseHeader(char *data);

int main(int argc, char *argv[]) {
    int sock, commsock, n, fd, i;
    struct sockaddr_in listener;
    char *filename;
    char *response;
    char buf[512];
    char filebuf[1024];
    fd_set rfds;
    struct timeval tv;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("error: socket()\n");
        exit(1);
    }

    n = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int)) < 0) {
        printf("error: setsockopt(), errno: %d\n", errno);
        exit(2);
    }

    listener.sin_family = AF_INET;
    listener.sin_port = htons(3022);
    listener.sin_addr.s_addr = htonl(INADDR_ANY);
    tv.tv_sec = 20;
    tv.tv_usec = 0;


    if (bind(sock, (struct sockaddr *) &listener, sizeof(listener)) < 0) {
        printf("error: bind()\n");
        exit(3);
    }
    
    while (1) {
        if (listen(sock, 5) < 0) {
            printf("error: listen()\n");
            exit(4);
        }
        
        printf("TCP Server listening....\n");
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);

        // Added select here to have a condition for server to exit instead of just using Control + C to shut down. Server will time out and exit if there are no new requests
        // for 20 seconds.
        fd = select(sock + 1, &rfds, NULL, NULL, &tv);
        if (fd == -1) {
            printf("error: select()\n");
            exit(5);
        }
        else if (FD_ISSET(sock, &rfds)) {
            commsock = accept(sock, NULL, NULL);
        }
        else {
            printf("No data within 30 seconds.\n");
            break;
        }
        
        n = recv(commsock, buf, sizeof(buf), MSG_WAITALL);
        buf[n] = '\0';

        printf("Request received from client, processing.....\n");
        filename = parseRequestHeader(buf);

        if ((fd = open(filename, O_RDONLY)) < 0) {
            printf("error: open()\n");
            exit(6);
        }
        
        n = read(fd, filebuf, sizeof(filebuf));
        filebuf[n] = '\0';

        response = createResponseHeader(filebuf);
        printf("Response sent to client...\n");
        n = send(commsock, response, strlen(response), MSG_CONFIRM);

        close(commsock);
    }
    
    printf("After program loop on server side, closing main socket and exiting program\n");
    close(sock);
}

char * parseRequestHeader(char *header) {
    int n, i;
    
    for (n = 0; n < strlen(header); n++) {
        if (header[n] == '/') {
            break;
        }
    }

    for (i = n; i < strlen(header); i++) {
        if (header[i] == ' ') {
            break;
        }
    }

    header[i] = '\0';
    header += n + 1;

    return header;
}

char * createResponseHeader(char *data) {
    int length;
    char *response = (char *) malloc(2048);
    char *status = "HTTP/1.0 200 OK\r\n";
    char *type = "Content-Type: text/html\r\n";
    char *crlf = "\r\n";
    char lengthchar[10];

    length = strlen(data);
    sprintf(lengthchar, "%d", length);

    // Building Response Header
    strncpy(response, status, strlen(status));
    strcat(response, type);
    strcat(response, "Content-Length: ");
    strcat(lengthchar, crlf);
    strcat(response, lengthchar);
    strcat(response, crlf);

    // Header completed, ready for body data to be added.
    strcat(response, data);

    return response;

}
