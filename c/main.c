#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define BACKLOG 50

int main(int argc, char** argv) {
  // Create address
  int sockfd, connfd;
  struct sockaddr_in my_addr, peer_addr;
  socklen_t my_addr_len, peer_addr_len;

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
    perror("socket");

  // Bind to socket
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(8080);
  if(inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr) == -1)
    perror("addr");

  if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == -1)
    perror("bind");

  // Listen on socket
  if (listen(sockfd, BACKLOG) == -1)
    perror("listen");

  // Accept conn
  peer_addr_len = sizeof(peer_addr);
  connfd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_len);
  if (connfd == -1)
    perror("accept");

  const char* response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "\r\n"
    "Hello, world!";

  write(connfd, response, strlen(response));
}
