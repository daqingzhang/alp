# alp
Examples for Linux programming

# Eamxple Lists

c1
-------------------------------------------------------------------------------
1. Using GCC to compile and link c source files
2. Using Makefile scripts
3. Using GDB to debug program
4. Using man and info

c2
-------------------------------------------------------------------------------
1. Getting main() arguments and parsing it as options
2. Getting environment varibles and exit code: setenv(), getenv()
3. Using standard IO: stdin, stdout, stderr: fprintf(), fflush()
4. Using temporary file: mkstemp(), unlink(), read() , write(), close()

c2-2
-------------------------------------------------------------------------------
1. Writing and Using static library.

c2-3
-------------------------------------------------------------------------------
1. Writing and Using dynamic library.
2. Dynamically loading library: LD_LIBRARY_PATH
3. Library dependencies

c3
-------------------------------------------------------------------------------
1. Getting process ID: getpid()
2. Creating and killing process: kill(), fork()
3. Zombie process

c3-2
-------------------------------------------------------------------------------
1. Cleaning up sub-process: wait()

c4
-------------------------------------------------------------------------------
1. Thread creation and joining

	pthread_self();
	pthread_create();
	pthread_join();
	pthread_cancel();

2. Using mutex in thread

	pthread_mutex_init();
	pthread_mutex_lock();
	pthread_mutex_unlock();


c4-2
-------------------------------------------------------------------------------
1. Using semaphore in thread

	sem_init();
	sem_post();
	sem_wait();


c5
-------------------------------------------------------------------------------
1. Using shared memory for interprocess communincation

	shmget();
	shmat();
	shmdt();


c5-2
-------------------------------------------------------------------------------
1. Using socket for interprocess communication

