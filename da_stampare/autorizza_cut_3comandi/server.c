#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#define MAX_REQUEST_SIZE (64 * 1024)
int autorizza(const char *username, const char *password)
{
	return 1;
}
void handler(int signo)
{
        int status;
        (void)signo;
        while (waitpid(-1, &status, WNOHANG) > 0)
                continue;
}
int main(int argc, char **argv)
{
        int sd, err, on;
        struct addrinfo hints, *res;
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags   = SA_RESTART;
        sa.sa_handler = handler;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(EXIT_FAILURE);
        }
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags    = AI_PASSIVE;
        if ((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0) {
                fprintf(stderr, "Errore setup indirizzo bind: %s\n", gai_strerror(err));
                exit(EXIT_FAILURE);
        }
        if ((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
                perror("Errore in socket");
                exit(EXIT_FAILURE);
        }
        on = 1;
        if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
                perror("setsockopt");
                exit(EXIT_FAILURE);
        }
        if (bind(sd, res->ai_addr, res->ai_addrlen) < 0) {
                perror("Errore in bind");
                exit(EXIT_FAILURE);
        }
        freeaddrinfo(res);
        if (listen(sd, SOMAXCONN) < 0) {
                perror("listen");
                exit(EXIT_FAILURE);
        }
        for(;;) {
                int ns, pid;
                if ((ns = accept(sd, NULL, NULL)) < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                }
                if ((pid = fork()) < 0) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                } else if (pid == 0) { 
                        uint8_t len[2];
                        char username[MAX_REQUEST_SIZE];
                        char password[MAX_REQUEST_SIZE];
                        char categoria[MAX_REQUEST_SIZE];
                        size_t username_len, password_len, categoria_len;
                        const uint8_t ack[4]  = { 0, 2, 'O', 'K' };
                        const uint8_t nack[4] = { 0, 2, 'N', 'O' };
                        close(sd);
                        if (read_all(ns, len, 2) < 0) {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }
                        username_len = (size_t)len[1] | ((size_t)len[0] << 8);
                        memset(username, 0, sizeof(username));
                        if (read_all(ns, username, username_len) < 0) {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }
                        if (read_all(ns, len, 2) < 0) {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }
                        password_len = (size_t)len[1] | ((size_t)len[0] << 8);
                        memset(password, 0, sizeof(password));
                        if (read_all(ns, password, password_len) < 0) {
                                perror("read");
                                exit(EXIT_FAILURE);
                        }
                        if (autorizza(username, password) == 1) {
                                puts("autorizzato"); fflush(stdout);
                                if (write_all(ns, ack, sizeof(ack)) < 0) {
                                        perror("write");
                                        exit(EXIT_FAILURE);
                                }
                        } else {
                                puts("non autorizzato"); fflush(stdout);
                                if (write_all(ns, nack, sizeof(nack)) < 0) {
                                        perror("write");
                                        exit(EXIT_FAILURE);
                                }
                                close(ns);
                                exit(EXIT_SUCCESS);
                        }
                        for (;;) {
                                int pipe_n1n2[2], pipe_n2n3[2], pipe_n3f[2];
                                int pid_n1, pid_n2, pid_n3, nread, read_so_far, response_len;
                                char response[65536];
                                if (read_all(ns, len, 2) < 0) {
                                        perror("read");
                                        exit(EXIT_FAILURE);
                                }
                                categoria_len = len[1] | (len[0] << 8);
                                memset(categoria, 0, sizeof(categoria));
                                if (read_all(ns, categoria, categoria_len) < 0) {
                                        perror("read");
                                        exit(EXIT_FAILURE);
                                }
                                if (pipe(pipe_n1n2) < 0) {
                                        perror("pipe");
                                        exit(EXIT_FAILURE);
                                }
                                pid_n1 = fork();
                                if (pid_n1 < 0) {
                                        perror("fork");
                                        exit(EXIT_FAILURE);
                                } else if (pid_n1 == 0) {
                                        char filename[MAX_REQUEST_SIZE + 256];
                                        snprintf(filename, sizeof(filename), 
                                                 "%s.txt", categoria);
                                        close(ns);
                                        close(pipe_n1n2[0]);
                                        close(1);
                                        if (dup(pipe_n1n2[1]) < 0) {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n1n2[1]);
                                        execlp("cut", "cut", "-f", "1,3,4", "-d", ",", filename, NULL);
                                        perror("exec");
                                        exit(EXIT_FAILURE);
                                }
                                if (pipe(pipe_n2n3) < 0) {
                                        perror("pipe");
                                        exit(EXIT_FAILURE);
                                }
                                pid_n2 = fork();
                                if (pid_n2 < 0) {
                                        perror("fork");
                                        exit(EXIT_FAILURE);
                                } else if (pid_n2 == 0) {
                                        close(ns);
                                        close(pipe_n1n2[1]);
                                        close(pipe_n2n3[0]);
                                        close(0);
                                        if (dup(pipe_n1n2[0]) < 0) {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n1n2[0]);
                                        close(1);
                                        if (dup(pipe_n2n3[1]) < 0) {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n2n3[1]);
                                        execlp("sort", "sort", "-r", "-n", NULL);
                                        perror("exec");
                                        exit(EXIT_FAILURE);
                                }
                                if (pipe(pipe_n3f) < 0) {
                                        perror("pipe");
                                        exit(EXIT_FAILURE);
                                }
                                pid_n3 = fork();
                                if (pid_n3 < 0) {
                                        perror("fork");
                                        exit(EXIT_FAILURE);
                                } else if (pid_n3 == 0) {
                                        close(ns);
                                        close(pipe_n1n2[0]);
                                        close(pipe_n1n2[1]);
                                        close(pipe_n2n3[1]);
                                        close(pipe_n3f[0]);
                                        close(0);
                                        if (dup(pipe_n2n3[0]) < 0) {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n2n3[0]);
                                        close(1);
                                        if (dup(pipe_n3f[1]) < 0) {
                                                perror("dup");
                                                exit(EXIT_FAILURE);
                                        }
                                        close(pipe_n3f[1]);
                                        execlp("head", "head", "-n", "10", NULL);
                                        perror("exec");
                                        exit(EXIT_FAILURE);
                                }
                                close(pipe_n1n2[0]);
                                close(pipe_n1n2[1]);
                                close(pipe_n2n3[0]);
                                close(pipe_n2n3[1]);
                                close(pipe_n3f[1]);
                                read_so_far = 0;
                                while ((nread = read(pipe_n3f[0], response + read_so_far, 
                                                     sizeof(response) - read_so_far)) > 0) {
                                        read_so_far += nread;
                                }
                                if (nread < 0) {
                                        perror("read");
                                        exit(EXIT_FAILURE);
                                }
                                if (read_so_far > 65535) {
                                        fprintf(stderr, "Troppi dati\n");
                                        exit(EXIT_FAILURE);
                                }
                                response_len = read_so_far;
                                len[0] = (uint8_t)((response_len & 0xFF00) >> 8);
                                len[1] = (uint8_t)(response_len & 0x00FF);
                                if (write_all(ns, len, 2) < 0) {
                                        perror("write");
                                        exit(EXIT_FAILURE);
                                }
                                if (write_all(ns, response, response_len) < 0) {
                                        perror("write");
                                        exit(EXIT_FAILURE);
                                }
                        }
                } 
                close(ns);
        }
        close(sd);
        return 0;
}
