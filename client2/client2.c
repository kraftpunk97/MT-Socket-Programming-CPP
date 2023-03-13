#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

#define PORT 8083
#define MAX_BUFFER_SIZE_SEND 100
#define MAX_BUFFER_SIZE_RECV 200
#define FILENAME "F2.txt"

void *send_file(void *socket_desc) {
    int client_filed = *(int*)socket_desc;
    char buffer_send[MAX_BUFFER_SIZE_SEND] = {0};
    FILE *fp;

    // Open file for reading
    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        printf("Error opening file: %s\n", FILENAME);
        return NULL;
    }

    // Read data from file and send to server
    printf("About to transmit data...\n");
    size_t bytes_read;
    while ((bytes_read = fread(buffer_send, sizeof(char), MAX_BUFFER_SIZE_SEND, fp)) > 0) {
        send(client_filed, buffer_send, bytes_read, 0);
        memset(buffer_send, 0, MAX_BUFFER_SIZE_SEND);
    }

    // Close file
    fclose(fp);
    printf("Done sending data for client2.\n");

    return NULL;
}

int main(int argc, char const *argv[]) {
    printf("This is client2...\n");
    char server_ip[20];
    int client_filed;
    struct sockaddr_in server_address;
    pthread_t thread_id;

    printf("Enter the server IP address: ");
    scanf("%s", server_ip);

    // Create client socket file descriptor
    if ((client_filed = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set server address and connect to server
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }
    if (connect(client_filed, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");
    // Start thread to send file to server
    if (pthread_create(&thread_id, NULL, send_file, (void*)&client_filed) < 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }


    // Wait for client to finish sending data to server.
    pthread_join(thread_id, NULL);


    FILE* f3;
    f3 = fopen("F3_P2.txt", "w");

    int valread;
    char buffer_recv[MAX_BUFFER_SIZE_RECV+1];
    while ((valread = read(client_filed, buffer_recv, MAX_BUFFER_SIZE_RECV)) > 0) {
        fwrite(buffer_recv, sizeof(char), valread, f3);
        memset(buffer_recv, 0, MAX_BUFFER_SIZE_RECV);
    }

    fclose(f3);
    printf("F3_P2.txt copied from server.\n");
    close(client_filed);
    printf("Closing connection...\n");
    return 0;
}