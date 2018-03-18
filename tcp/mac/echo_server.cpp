#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    /**
     * sockfd :
     * newsockfd :
     * portno : 
     */
    int sockfd, newsockfd, portno;

    socklen_t clilen; /** sys/socket.h, which is an unsigned opaque integral type of length of at least 32 bits */

    char buffer[256];
    /**
        struct sockaddr_in{  
            short sin_family;  // 주소패밀리
            unsigned short sin_port;  // 포트
            struct in_addr sin_addr;  // 주소
            char sin_zero[8];  // SOCKADDR 사이즈와 동일하게 padding
        };
    */  
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

    /**
        SOCKET socket(
            _In_  int af, -> 소켓이 사용할 프로토콜 체계 
            _In_  int type, -> 소켓의 타입
            _In_  int protocol -> 프로토콜 체계중 실제적으로 사용할 프로토콜
        );
    */ 
    /**
     * AF_ : 주소체계 (AF_INET : ipv4, AF_INET6 : ipv6, AF_LOCAL : local 통신을 위한 unix )
     * PF_ : 프로토콜체계 (PF_INET, PF_INET6, PF_LOCAL, PF_PACKET: Low level socket을 위한 인터페이스, PF_IPX: IPX 노벨 프로토콜)
     * SOCK_STREAM : 연결 지향형 소켓
     * SOCK_DGRAM : 비연결 지향형 소켓
     * IPPROTO_TCP, IPPROTO_UDP, 0을 써도 자동으로 생성됨
     */ 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     bzero(buffer,256);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s\n",buffer);
     n = write(newsockfd,"I got your message",18);
     if (n < 0) error("ERROR writing to socket");
     close(newsockfd);
     close(sockfd);
     return 0; 
}