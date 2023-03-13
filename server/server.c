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
#define MAX_CLIENTS 2
#define MAX_BUFFER_SIZE_RECV 100
#define MAX_BUFFER_SIZE_SEND 200
#define FILENAME "F3.txt"
#define TERMINATION_STR "1111"

struct thread_struct {
    int arg;
    int socket_filed;
};

void *recv_data_from_client(void *thread_struct_arg) {
    struct thread_struct* thread_struct_arg__ = thread_struct_arg;
    int client_socket =  thread_struct_arg__->socket_filed;
    int socket_arg = thread_struct_arg__->arg;
    char recv_buffer[MAX_BUFFER_SIZE_RECV] = {0};
    FILE *temp_fp;

    char temp_filename[30];
    sprintf(temp_filename, "temp%d.txt", socket_arg);
    // Open file for writing buffer data
    temp_fp = fopen(temp_filename, "w");

    // Read data from client socket and write to file
    int valread;
    printf("Opened the temporary file now for P%d...\n",socket_arg);
    while ((valread = read(client_socket, recv_buffer, MAX_BUFFER_SIZE_RECV)) > 0) {
        printf("Data Read:%d \n", valread);
        char* pch;
        pch = strstr(recv_buffer, TERMINATION_STR);
        if(pch != NULL){
            char * buffer_ptr = recv_buffer;
            while(buffer_ptr!=pch) {
                fputc(*buffer_ptr, temp_fp);
                buffer_ptr += 1;
            }
            break;
        }
        fwrite(recv_buffer, sizeof(char), valread, temp_fp);
        memset(recv_buffer, 0, MAX_BUFFER_SIZE_RECV);
    }

    // Close file and client socket
    fclose(temp_fp);

    printf("Successfully copied data from P%d to temporary file.\n",socket_arg);

    return NULL;
}

int main(int argc, char const *argv[]) {

    int server_filed, new_socket, client_sockets[MAX_CLIENTS], valread;
    struct thread_struct thread_struct_arg[MAX_CLIENTS];
    struct sockaddr_in address;
    int address_len = sizeof(address);
    pthread_t client_thread[MAX_CLIENTS];

    // Create server socket file descriptor
    if ((server_filed = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_filed, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set socket address and bind socket to address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_filed, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    else printf("Successfully bound socket to port %d.\n", PORT);

    // Start listening on server socket for incoming connections
    if (listen(server_filed, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    else printf("Listening for connections on port%d\n", PORT);
    int i;
    // Initialize client sockets to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }
     // Wait for incoming connection
    // Accept files from client
    // Add new client socket to array of sockets and start new thread to handle connection
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
            if ((new_socket = accept(server_filed, (struct sockaddr *)&address, (socklen_t*)&address_len)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }   
            else
                printf("Accepted  a connection.\n");
            
            client_sockets[i] = new_socket;
            thread_struct_arg[i].socket_filed = client_sockets[i];
            thread_struct_arg[i].arg = i+1;
            if (pthread_create(&client_thread[i], NULL, recv_data_from_client, (void*)&thread_struct_arg[i]) < 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }
    }

    for(i=0; i<MAX_CLIENTS; i++)
        (void) pthread_join(client_thread[i], NULL);

    
    FILE* temp1, *temp2, *final3;
    char temp1_name[] = "temp1.txt";
    char temp2_name[] = "temp2.txt";
    char final3_name[] = "F3.txt";

    temp1 = fopen(temp1_name, "r");
    temp2 = fopen(temp2_name, "r");
    final3 = fopen(final3_name, "w");

    char buffer[101];
    
    while ((valread = fread(buffer, 1, 100, temp1)) > 0) {
        fwrite(buffer, sizeof(char), valread, final3);
        memset(buffer, 0, 100);
    }

    while ((valread = fread(buffer, 1, 100, temp2)) > 0) {
        fwrite(buffer, sizeof(char), valread, final3);
        memset(buffer, 0, 100);
    }

    fclose(temp1);
    fclose(temp2);
    fclose(final3);

    printf("F3.txt has been generated.\n");

    FILE* f3;
    f3 = fopen("F3.txt", "r");
    if (f3 == NULL) {
        printf("Error opening file: %s\n", FILENAME);
        return -1;
    }

    char buffer_send[MAX_BUFFER_SIZE_SEND+1] ={0};
    // Read data from file and send to server
    size_t bytes_read;
    int c1_sent;
    int c2_sent;
    while ((bytes_read = fread(buffer_send, sizeof(char), MAX_BUFFER_SIZE_SEND, f3)) > 0) {
        c1_sent = send(client_sockets[0], buffer_send, bytes_read, 0);
        c2_sent = send(client_sockets[1], buffer_send, bytes_read, 0);
        printf("Data sent to P1:%d\nData sent to P2:%d", c1_sent, c2_sent);
        memset(buffer_send, 0, MAX_BUFFER_SIZE_SEND);
    }

    // Close file and client socket
    fclose(f3);

    printf("F3 sent to P1 and P2.\n");

     for(i=0; i<MAX_CLIENTS; i++)
        close(client_sockets[i]);

    // Close server socket
    close(server_filed);

    return 0;
}