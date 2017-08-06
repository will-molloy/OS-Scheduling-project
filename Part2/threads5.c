/*
	The threads.
*/

void threadYield();

void thread1() {
	int i;
	for (i = 0; i < 15; i++) {
		puts("one");
		threadYield();
	}
}

void thread2() {
	int i;
	for (i = 0; i < 2; i++) {
		puts("two");
		threadYield();
	}
}
void thread3() {
	int i;
	for (i = 0; i < 3; i++) {
		puts("three");
		threadYield();
	}
}
void thread4() {
	int i;
	for (i = 0; i < 1; i++) {
		puts("four");
		threadYield();
	}
}
void thread5() {
	int i;
	for (i = 0; i < 4; i++) {
		puts("five");
		threadYield();
	}
}


const int NUMTHREADS = 5;

typedef void (*threadPtr)();

threadPtr threadFuncs[] = {thread1, thread2, thread3, thread4, thread5};
