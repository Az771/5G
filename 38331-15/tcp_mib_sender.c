#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "mib_common.h"

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    uint8_t buffer[128];
    size_t mib_len;

    /* 1. Encode MIB into buffer */
    if(encode_mib_to_buffer(buffer, sizeof(buffer), &mib_len) != 0) {
        fprintf(stderr, "Failed to encode MIB\n");
        return 1;
    }

    printf("Sender: MIB encoded length = %zu bytes\n", mib_len);

    /* 2. Create TCP socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9000);

    /* ✅ RECEIVER IP */
    inet_pton(AF_INET, "172.16.139.235", &servaddr.sin_addr);

    /* 3. Connect to receiver */
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    /* 4. Send length prefix */
    uint32_t net_len = htonl((uint32_t)mib_len);
    if(write(sockfd, &net_len, sizeof(net_len)) != sizeof(net_len)) {
        perror("write length");
        close(sockfd);
        return 1;
    }

    /* 5. Send MIB bytes */
    ssize_t sent = write(sockfd, buffer, mib_len);
    if(sent != (ssize_t)mib_len) {
        perror("write mib");
        close(sockfd);
        return 1;
    }

    printf("Sender: sent %zd bytes MIB to receiver\n", sent);

    close(sockfd);
    return 0;
}
