#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char* parsePageLinks(char* buf);

char* selectNextPage(char* links);

void displayWithoutTags(char* buf);

int main(int argc, char *argv[]) {
    int sock, n, i;
    char buf[512];
    char* bufpointer;
    char* pageLinks;
    char *filename;
    char *header_part1 = "GET /";
    char *header_part2 = " HTTP/1.0";
    struct sockaddr_in listener;

    if (argc != 2) {
        printf("error: invalid amount of arguments\n");
        exit(1);
    }

    listener.sin_family = AF_INET;
    listener.sin_port = htons(3022);
    listener.sin_addr.s_addr = htonl(INADDR_ANY);

    filename = argv[1];

    while (1) {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("error: socket()\n");
            exit(2);
        }
        
        if ((n = connect(sock, (struct sockaddr *) &listener, sizeof(listener))) < 0) {
            printf("error: connect()\n");
            exit(3);
        }

        printf("Client connected to Server\n");

        memset(buf, 0, sizeof(buf));
        strcpy(buf, header_part1);
        strcat(buf, filename);
        strcat(buf, header_part2);
    
        send(sock, buf, sizeof(buf), MSG_CONFIRM);

        n = recv(sock, buf, sizeof(buf), MSG_WAITALL);
        buf[n] = '\0';

        // Stripping out header so that display function is only passed body data
        bufpointer = buf;
        for (i = 0; i < strlen(buf); i++) {
            if (buf[i] == '<') {
                bufpointer += i;
                break;
            }
        }

        displayWithoutTags(bufpointer);
        pageLinks = parsePageLinks(buf);
        printf("Page Links: \n");
        filename = selectNextPage(pageLinks);
        printf("next page: %s\n", filename);

        close(sock);
    }
}

char* parsePageLinks(char* buf) {
    int i, n;
    char* linksbuf = malloc(sizeof(char) * 256);
    char *returnvalue;
    char *link = "<a href";
    char *newline = "\n";
    char *bufpointer;
    char *linkpointer;

    bufpointer = buf;
    while ((linkpointer = strstr(bufpointer, link)) != NULL) {
        bufpointer = linkpointer;
        for (i = 0; i < strlen(linkpointer); i++) {
            if (linkpointer[i] == '"') {
                for (n = i + 1; n < strlen(linkpointer); n++) {
                    if (linkpointer[n] == '"') {
                        bufpointer += i + 1;
                        if (strlen(linksbuf) == 4) {
                            strncpy(linksbuf, bufpointer, (n - (i + 1)));
                            linksbuf[n - i] = '\n';
                            linksbuf[n - (i - 1)] = '\0';
                        }
                        else {
                            strncat(linksbuf, bufpointer, (n - (i + 1)));
                            strcat(linksbuf, newline);
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    
    returnvalue = linksbuf;

    return returnvalue;
}

char * selectNextPage(char* links) {
    int i, n, count, newlines, input;
    char countchar;
    char formatting = ')';
    char space = ' ';
    char* nextPage;
    
    if (strlen(links) > 0) {      
        count = 1;
        countchar = count + '0';
        putchar(countchar);
        putchar(formatting);
        putchar(space);
        for (i = 0; i < strlen(links); i++) {
            if (links[i] == '\n' && i != (strlen(links) - 1)) {
                putchar(links[i]);  
                count++;
                countchar = count + '0';
                putchar(countchar);
                putchar(formatting);
                putchar(space);   
            }
            else {
                putchar(links[i]);
            }
        } 
    }
    else {
        return "There are no links on this page.\n";
    }

    while (1) {
        printf("Enter the integer value for the next page you would like to access: ");
        scanf("%d", &input);
        if (input > 0 && input <= count) {
            break;
        }
        else {
            printf("Invalid value given, please try again.\n");
        }
    }

    newlines = 0;
    for (i = 0; i < strlen(links); i++) {
        if (links[i] == '\n') {
            newlines++;
        }
        if (newlines == (input - 1)) {
            if (newlines == 0) {
                links += i;
            }
            
            else {
                links += i + 1;
            }

            for (i = 0; i < strlen(links); i++) {
                if(links[i] == '\n') {
                    links[i] = '\0';
                    break;
                }
            }
            break;
        }
    }
    return links;
}

void displayWithoutTags(char* buf) {
    int n, i, count;
    char tempbuf[512];
    
    count = 0;
    for (i = 0; i < strlen(buf); i++) {
        if (buf[i] == '\n') {
            tempbuf[count] = buf[i];
            count++;
        }
        if (buf[i] == '>' && i != (strlen(buf) - 2)) {
            i++;
            while (buf[i] != '<') {
                tempbuf[count] = buf[i];
                count++;
                i++;
            }
        }
    }

    tempbuf[count] = '\0';
    printf("-----------------------------------------------------------------------------");
    printf("%s", tempbuf);
    printf("-----------------------------------------------------------------------------\n");
}