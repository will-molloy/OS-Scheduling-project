/*
	The threads.
	@author: Will Molloy, wmol664
*/

void threadYield();

void thread1() {
	int i;
	for (i = 0; i < 2; i++) {
		puts("hi");
		threadYield();
	}
}

void thread2() {
	int i;
	for (i = 0; i < 5; i++) {
		puts("bye");
		threadYield();
	}
}

void thread3() {
	int i;
	for (i = 0; i < 15; i++) {
		puts("hey");
		threadYield();
	}
}

const int NUMTHREADS = 5;

typedef void (*threadPtr)();

threadPtr threadFuncs[] = {thread1, thread2, thread1, thread2, thread3};
