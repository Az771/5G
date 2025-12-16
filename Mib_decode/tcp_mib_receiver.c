#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "BCCH-BCH-Message.h"
#include "asn_application.h"

int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // listen on all local interfaces
    servaddr.sin_port = htons(9000);

    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        close(listenfd);
        return 1;
    }

    if(listen(listenfd, 1) < 0) {
        perror("listen");
        close(listenfd);
        return 1;
    }

    printf("Receiver: listening on port 9000...\n");

    connfd = accept(listenfd, NULL, NULL);
    if(connfd < 0) {
        perror("accept");
        close(listenfd);
        return 1;
    }

    printf("Receiver: client connected.\n");

    /* 1. Read length prefix */
    uint32_t net_len;
    ssize_t n = read(connfd, &net_len, sizeof(net_len));
    if(n != sizeof(net_len)) {
        perror("read length");
        close(connfd);
        close(listenfd);
        return 1;
    }

    uint32_t mib_len = ntohl(net_len);
    printf("Receiver: expecting %u bytes MIB\n", mib_len);

    if(mib_len > 1024) {
        fprintf(stderr, "Receiver: length too large\n");
        close(connfd);
        close(listenfd);
        return 1;
    }

    /* 2. Read MIB bytes */
    uint8_t buffer[1024];
    size_t total_read = 0;
    while(total_read < mib_len) {
        n = read(connfd, buffer + total_read, mib_len - total_read);
        if(n <= 0) {
            perror("read mib");
            close(connfd);
            close(listenfd);
            return 1;
        }
        total_read += n;
    }

    printf("Receiver: got %zu bytes of MIB\n", total_read);

    /* 3. Decode using uper_decode */
    BCCH_BCH_Message_t *msg = NULL;
    asn_dec_rval_t dr = uper_decode(
        NULL,
        &asn_DEF_BCCH_BCH_Message,
        (void **)&msg,
        buffer,
        mib_len,
        0,
        0
    );

    if(dr.code != RC_OK) {
        fprintf(stderr, "Receiver: decode failed\n");
        close(connfd);
        close(listenfd);
        return 1;
    }

    printf("Receiver: decode success. XER dump:\n");
    xer_fprint(stdout, &asn_DEF_BCCH_BCH_Message, msg);

    close(connfd);
    close(listenfd);
    return 0;
}
