#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LISTEN_BACKLOG 10
#define MAX_CLIENTS 100

// Macro for handling errors
#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(void) {
  // Initialize needed variables
  struct sockaddr_in my_addr, peer_addr; 
  int client_fds[MAX_CLIENTS];
  int client_count = 0;
  int max_fd = 0;
  fd_set read_fds, all_fds;
  socklen_t peer_addr_size;

  // Create the sockfd which is just a file descriptor
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    handle_error("socket");
  }

  // Bind address to socket
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  my_addr.sin_port = htons(8080);
  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)
    handle_error("bind");
  printf("Bound to address.\n");


  // Listen
  if (listen(sockfd, LISTEN_BACKLOG) == -1)
    handle_error("listen");
  printf("Listening on 127.0.0.1:8080\n");


  // Initialize the file descriptor set
  FD_ZERO(&all_fds);
  FD_SET(sockfd, &all_fds);
  max_fd = sockfd;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_fds[i] = -1;
  }

  while (1) {
    read_fds = all_fds;

    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
      handle_error("select");
    }

    if (FD_ISSET(sockfd, &read_fds)) {
      peer_addr_size = sizeof(peer_addr);
      int connfd = accept(sockfd, (struct sockaddr *)&peer_addr, &peer_addr_size);
      if (connfd == -1) {
        perror("accept");
        continue;
      }

      int added = 0;
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] == -1) {
          client_fds[i] = connfd;
          FD_SET(connfd, &all_fds);
          if (connfd > max_fd) {
            max_fd = connfd;
          }

          added = 1;
          break;
        }
      }

      if (!added) {
        printf("Too many clients connected, rejecting.");
        close(connfd);
      }
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
      int client_fd = client_fds[i];
      if (client_fd != -1 && FD_ISSET(client_fd, &read_fds)) {
        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
          buffer[bytes_read] = '\0';
          printf("Received from %d: %s\n", client_fd, buffer);

          for (int j = 0; j < MAX_CLIENTS; j++) {
            if (client_fds[j] != -1 && client_fds[j] != client_fd) {
              write(client_fds[j], buffer, strlen(buffer));
            }
          }
        } else if (bytes_read == 0) {
          printf("Client %d disconnected.\n", client_fd);
          close(client_fd);
          FD_CLR(client_fd, &all_fds);
          client_fds[i] = -1;
        } else {
          perror("read");
        }
      }
    }
  }

  close(sockfd);
  return 0;
}
