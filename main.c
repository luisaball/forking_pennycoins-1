#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>

#define NCHILD       16
#define PENNY_MSG_SZ 32
#define RAND_SPAN    (RAND_MAX/4)

struct child_data {
	int   child_off;
	int   pending;		/* does the child have pending data? */
	int   pipe[2]; 		/* pipe to send the bytecoin return message to parent. [0] is readable, [1] writable */
	int   nbpipe[2]; 	/* pipe to send the bytecoin return message to parent with the read-side being non-blocking */
	char *shmem;		/* shared memory to send the bytecoin return message to parent */
};

struct child_data children[NCHILD];
char *shmem;

void
create_pipes(struct child_data *d)
{
	/* d->pipe[0] is the "read" end of the pipe, and d->pipe[1] is for "writes" */
 	if (pipe(d->pipe)) {
		perror("pipe creation error");
		exit(EXIT_FAILURE);
	}
	if (pipe(d->nbpipe)) {
		perror("nbpipe creation error");
		exit(EXIT_FAILURE);
	}
	if (fcntl(d->nbpipe[0], F_SETFL, O_NONBLOCK)) {
		perror("nbpipe error setting to non-blocking");
		exit(EXIT_FAILURE);
	}
}

void
create_shmem(struct child_data *d)
{
	assert(shmem != NULL);

	d->shmem = shmem + (d->child_off * PENNY_MSG_SZ);
}

void
ipc_init(void)
{
	int i;

	shmem = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	for (i = 0; i < NCHILD; i++) {
		struct child_data *c = &children[i];

		c->child_off = i;
		c->pending   = 1;
		create_shmem(c);
		create_pipes(c);
	}
}

/*
 * I hear bitcoin is amazing. But who wouldn't want a pennycoin? Lets
 * do some pennycoin mining!
 *
 * TODO 1: update this to send over the non-blocking channel (the
 * write is still blocking!!!!).
 *
 * TODO 2: send the message back in shared memory instead of over a
 * pipe!
 */
void
pennycoin_mining(struct child_data *us, char byte)
{
	char return_message[PENNY_MSG_SZ];
	unsigned int i;
	(void)us;

	/* emulate trying to find a specific byte! */
	while (rand() % RAND_SPAN != byte) ;

	for (i = 0; i < PENNY_MSG_SZ; i++) {
		return_message[i] = byte;
	}

	/* Add code here to send the return message back to the parent! */
	write(us->nbpipe[1], return_message, PENNY_MSG_SZ);

	return;
}

int
main(int argc, char *argv[])
{
	int i, msgs_received = 0;
	time_t t;
	(void)argc; (void)argv;

	srand((unsigned int)time(&t));
	ipc_init();

	for (i = 0; i < NCHILD; i++) {
		switch (fork()) {
		case -1: {
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
		case 0: {	/* child */
			pennycoin_mining(&children[i], i+1);
			exit(EXIT_SUCCESS);
		}}
	}


	/*
	 * Parent logic to understand. Remember your TODOs!
	 *
	 * TODO 1: update this to receive over the non-blocking
	 * channel, the `read` should be non-blocking so that we can
	 * check for the *next* available pennycoin.
	 *
	 * TODO 2: receive the message from a child in shared memory
	 * instead of over a pipe!
	 */
	while (msgs_received < NCHILD) {
		for (i = 0; i < NCHILD; i++) {
			char msg[PENNY_MSG_SZ];
			int ret;

			/* Skip over the children for which we've already received a message */
			if (!children[i].pending) continue;

			/* read from the child. */
			ret = read(children[i].nbpipe[0], msg, PENNY_MSG_SZ);
			if (ret < 0) {
				if (errno == EAGAIN) {
					/* non-blocking logic is here! */
					continue; /* Just keep on going through all the children */
				} else {
					perror("Parent reading from child");
					exit(EXIT_FAILURE);
				}
			}

			/* We got a message! */
			assert(ret == PENNY_MSG_SZ);
			assert(msg[0] == i+1);
			children[i].pending = 0;
			msgs_received++;
			printf("Mining %d done, cha-ching!\n", i);
		}
	}

	for (i = 0; i < NCHILD; i++) {
		int status;

		wait(&status);
	}
        munmap(shmem, 4096);

	return 0;
}
