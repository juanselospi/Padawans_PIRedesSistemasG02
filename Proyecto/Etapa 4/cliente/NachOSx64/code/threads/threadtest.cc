// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
//

#include "copyright.h"
#include "diningph.h"
#include "h2o.h"
#include "system.h"
#include <unistd.h>

DiningPh *dp;
H2O *water;

void HThread(void *p) {
  long who = (long)p;
  water->hReady(who);
}

void OThread(void *p) {
  long who = (long)p;
  water->oReady(who);
}

void Philo(void *p) {

  int eats, thinks;
  long who = (long)p;

  currentThread->Yield();

  for (int i = 0; i < 10; i++) {

    printf(" Philosopher %ld will try to pickup sticks\n", who + 1);

    dp->pickup(who);
    dp->print();
    eats = Random() % 6;

    currentThread->Yield();
    sleep(eats);

    dp->putdown(who);

    thinks = Random() % 6;
    currentThread->Yield();
    sleep(thinks);
  }
}

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

void SimpleThread(void *name) {
  // Reinterpret arg "name" as a string
  char *threadName = (char *)name;

  // If the lines dealing with interrupts are commented,
  // the code will behave incorrectly, because
  // printf execution may cause race conditions.
  for (int num = 0; num < 10; num++) {
    // IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf("*** thread %s looped %d times\n", threadName, num);
    // interrupt->SetLevel(oldLevel);
    // currentThread->Yield();
  }
  // IntStatus oldLevel = interrupt->SetLevel(IntOff);
  printf(">>> Thread %s has finished\n", threadName);
  // interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling
//	SimpleThread ourselves.
//----------------------------------------------------------------------

void TestDiningPh() {
  dp = new DiningPh();
  for (long k = 0; k < 5; k++) {
    Thread *Ph = new Thread("dp");
    Ph->Fork(Philo, (void *)k);
  }
}

void TestH2O() {
  water = new H2O();
  for (long k = 0; k < 4; k++) {
    Thread *h = new Thread("h");
    h->Fork(HThread, (void *)k);
  }
  for (long k = 0; k < 2; k++) {
    Thread *o = new Thread("o");
    o->Fork(OThread, (void *)k);
  }
}

void ThreadTest() {
  DEBUG('t', "Entering SimpleTest");

  // TestDiningPh();
  TestH2O();
}
