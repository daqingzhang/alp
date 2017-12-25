# alp
Examples for Linux programming

# Eamxple Lists

c1
-------------------------------------------------------------------------------
1. Using GCC to compile and link c source files
2. Using Makefile scripts

	-L -I -D -O -c -o -l -g --static -Werror -Wall

3. Using GDB to debug program
4. Using man and info

c2
-------------------------------------------------------------------------------
1. Getting main() arguments and parsing it as options

	getopt_long();

2. Getting environment varibles and exit code

	setenv();
	getenv();

3. Using standard IO: stdin, stdout, stderr

	printf();
	fprintf();
	fflush();

4. Using temporary file

	mkstemp();
	unlink();
	read();
	write();
	close();

c2-2
-------------------------------------------------------------------------------
1. Writing and Using static library.

c2-3
-------------------------------------------------------------------------------
1. Writing and Using dynamic library.
2. Dynamically loading library

	LD_LIBRARY_PATH;

3. Library dependencies

c3
-------------------------------------------------------------------------------
1. Getting process ID

	getpid();

2. Creating and killing process

	kill();
	fork();

3. Zombie process

c3-2
-------------------------------------------------------------------------------
1. Cleaning up sub-process

	wait();

c3-3
-------------------------------------------------------------------------------
1. using system() to start a new process

c3-4
-------------------------------------------------------------------------------
1. cleanup a child process asynchonously.
The signal SIGCHLD 's action is changed for invoking wait().
	waitpid();

c4
-------------------------------------------------------------------------------
1. Thread creation and joining

	pthread_self();
	pthread_create();
	pthread_join();
	pthread_cancel();

2. mutex in thread

	pthread_mutex_init();
	pthread_mutex_lock();
	pthread_mutex_unlock();


c4-2
-------------------------------------------------------------------------------
1. semaphore in thread

	sem_init();
	sem_post();
	sem_wait();


c5
-------------------------------------------------------------------------------
1. shared memory for interprocess communincation

	shmget();
	shmat();
	shmdt();
	shmctl();

c5-2
-------------------------------------------------------------------------------
1. local socket for interprocess communication
communication methead:
	(1) pipe;
	(2) shared memory;
	(3) semaphore;
	(4) socket;

c5-3
-------------------------------------------------------------------------------
1. inet socket for interprocess communication
There is a server that can be connected by telnet via port 1234.

c5-4
-------------------------------------------------------------------------------
1. multiple inet sockets for interprocess communication.
There is a server that detects two port at the same time. If any port command
comes, server will be connected by a client(usually a telnet).

demo-calc
-------------------------------------------------------------------------------


demo-dh
-------------------------------------------------------------------------------

demo-env
-------------------------------------------------------------------------------

demo-io
-------------------------------------------------------------------------------

demo-lib
-------------------------------------------------------------------------------

demo-signal
-------------------------------------------------------------------------------

demo-sys
-------------------------------------------------------------------------------

demo-time
-------------------------------------------------------------------------------

