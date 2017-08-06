/*
 ============================================================================
 Name        : OSA1.1.c
 Author      : Will Molloy, wmol664 (original by Robert Sheehan)
 Version     : 1.0
 Description : Switches between threads in linked list in creation order.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "littleThread.h"
#include "threads1.c" // rename this for different threads

Thread newThread; // the thread currently being set up
Thread mainThread; // the main thread
Thread currentThread; // the thread currently running
struct sigaction setUpAction;
const char *stateNames[] = { "SETUP" , "RUNNING", "READY", "FINISHED" }; // to print enum names
Thread threads[5];

/*
 * Called whenever there is a change in threads
 */
void printThreadStates(){
	printf("\nThread States\n=============\n");
	for (int t = 0; t < NUMTHREADS; t++){
		Thread thread = threads[t];
		printf("threadID: %d state:%s\n",thread->tid, stateNames[thread->state]);
	}
	printf("\n");
}

void removeThreadFromList(Thread thread){
	printf("\ndisposing %d\n", thread->tid);
	thread->prev->next = thread->next;
	thread->next->prev = thread->prev;
	thread->next = NULL;
	thread->prev = NULL;
	free(thread->stackAddr); // Wow!
}

/*
 * Switches execution from prevThread to nextThread.
 */
void switcher(Thread prevThread, Thread nextThread) {
	//printf("switcher(), prevThread: %d, nextThread: %d, current: %d\n", prevThread->tid, nextThread->tid, currentThread->tid);
	if (prevThread->state == FINISHED) { // it has finished
		removeThreadFromList(prevThread);
		currentThread->state = RUNNING;
		printThreadStates();
		longjmp(nextThread->environment, 1);
	}
	else if (setjmp(prevThread->environment) == 0) { // so we can come back here
		prevThread->state = READY;
		nextThread->state = RUNNING;
		printThreadStates();
		longjmp(nextThread->environment, 1);
	}
}

void scheduler(){
	while (currentThread->state != READY) {
		currentThread = currentThread->next;
		if (currentThread->state == FINISHED){
			// list exhausted (one finished thread left in it), go back to main thread
			removeThreadFromList(currentThread);
			longjmp(mainThread->environment, 1);
		}
	}
	//longjmp(currentThread->environment, 1);
	switcher(currentThread->prev, currentThread);
}

/*
 * Associates the signal stack with the newThread.
 * Also sets up the newThread to start running after it is long jumped to.
 * This is called when SIGUSR1 is received.
 */
void associateStack(int signum) {
	Thread localThread = newThread; // what if we don't use this local variable?
	localThread->state = READY; // now it has its stack
	if (setjmp(localThread->environment) != 0) { // will be zero if called directly
		(localThread->start)();
		localThread->state = FINISHED;
		scheduler(); // pick next thread to run
	}
}

/*
 * Sets up the user signal handler so that when SIGUSR1 is received
 * it will use a separate stack. This stack is then associated with
 * the newThread when the signal handler associateStack is executed.
 */
void setUpStackTransfer() {
	setUpAction.sa_handler = (void *) associateStack;
	setUpAction.sa_flags = SA_ONSTACK;
	sigaction(SIGUSR1, &setUpAction, NULL);
}

/*
 *  Sets up the new thread.
 *  The startFunc is the function called when the thread starts running.
 *  It also allocates space for the thread's stack.
 *  This stack will be the stack used by the SIGUSR1 signal handler.
 */
Thread createThread(void (startFunc)()) {
	static int nextTID = 0;
	Thread thread;
	stack_t threadStack;

	if ((thread = malloc(sizeof(struct thread))) == NULL) {
		perror("allocating thread");
		exit(EXIT_FAILURE);
	}
	thread->tid = nextTID++;
	thread->state = SETUP;
	thread->start = startFunc;
	if ((threadStack.ss_sp = malloc(SIGSTKSZ)) == NULL) { // space for the stack
		perror("allocating stack");
		exit(EXIT_FAILURE);
	}
	thread->stackAddr = threadStack.ss_sp;
	threadStack.ss_size = SIGSTKSZ; // the size of the stack
	threadStack.ss_flags = 0;
	if (sigaltstack(&threadStack, NULL) < 0) { // signal handled on threadStack
		perror("sigaltstack");
		exit(EXIT_FAILURE);
	}
	newThread = thread; // So that the signal handler can find this thread
	kill(getpid(), SIGUSR1); // Send the signal. After this everything is set.
	return thread;
}

int main(void) {
	struct thread controller;
	mainThread = &controller;
	mainThread->state = RUNNING;
	setUpStackTransfer();
	// create the threads
	for (int t = 0; t < NUMTHREADS; t++) {
		threads[t] = createThread(threadFuncs[t]);
	}
	// put threads into doubly circular linked list
	for (int t = 0; t < NUMTHREADS; t++){
		threads[t]->next = threads[(t+1)%NUMTHREADS];
		threads[t]->prev = threads[(t+NUMTHREADS-1)%NUMTHREADS];
	}
	// print states after thread creation but before any have started
	printThreadStates();

	// keep track of curernt thread; initially first created thread
	currentThread = threads[0];

	// switch to first thread; this will lead to scheduler() being called
	puts("switching to first thread");
	switcher(mainThread, currentThread);

	puts("\nback to the main thread");

	// print states one last time before process finishes
	printThreadStates();
	return EXIT_SUCCESS;
}
