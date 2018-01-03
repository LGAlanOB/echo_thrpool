#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define die(x) { perror(x); exit(EXIT_FAILURE); }

#define BUFSIZE 4096
#define NPTHREAD 2

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *
thr_do(void *arg)
{
	int listenfd, connfd;
	char buf[BUFSIZE];
	int nrecv;

	listenfd = (int)arg;
	for (;;) {
		pthread_mutex_lock(&mutex);
		if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
			perror("accept");
			continue;
		}
		pthread_mutex_unlock(&mutex);

		while ((nrecv = recv(connfd, buf, BUFSIZE, 0)) > 0) {
			if (send(connfd, buf, nrecv, 0) != nrecv) {
				close(connfd);
				break;
			}
		}
		if (nrecv == -1)
			perror("recv");
		close(connfd);
	}	
}

int
main(int argc, char **argv)
{
	int listenfd;
	pthread_t tid;

	if (argc == 2) {
		listenfd = tcp_listen(NULL, argv[1]);
	} else if (argc == 3) {
		listenfd = tcp_listen(argv[1], argv[2]);
	} else {
		fprintf(stderr, "Usage: %s [hostname] " 
				"servname\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NPTHREAD; i++) {
		pthread_create(&tid, NULL, 
					&thr_do, (void *)listenfd);
	}
	pause();
}


