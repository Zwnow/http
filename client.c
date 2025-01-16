#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(void) {
  struct sockaddr_in addr;
  socklen_t addr_size;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    handle_error("socket");
  }

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  addr.sin_port = htons(8080);

  if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
    close(sockfd);
    handle_error("connect");
  }

  while (1) {
    fd_set read_fds;
    FD_ZERO(&read_fds);

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    int max_fd = sockfd;

    // Wait for activity on either stdin or socket
    if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
      handle_error("select");
    }
    
    // Check if there's data from the server
    if (FD_ISSET(sockfd, &read_fds)) {
      char buffer[1024] = {0};
      ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer) - 1);

      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Server: %s\n", buffer);
      } else if (bytes_read == 0) {
        printf("Server closed the connection.\n");
        break;
      } else {
        handle_error("read");
      }
    }

    // Check if there is user input
    if (FD_ISSET(STDIN_FILENO, &read_fds)) {
      char message[200];
      if (fgets(message, sizeof(message), stdin) != NULL) {
        message[strcspn(message, "\n")] = '\0';
        if (write(sockfd, message, strlen(message)) == -1) {
          handle_error("write");
        }
      }
    }
  }

  close(sockfd);
  return 0;
}
