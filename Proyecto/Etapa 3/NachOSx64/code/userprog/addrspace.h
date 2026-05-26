// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Load a new user program
    AddrSpace(AddrSpace *parent);	// Fork: share code/data, new stack
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initial registers for main thread
    void InitRegistersForFork(int funcAddr);	// Registers for Fork child

    void SaveState();			// Save address space on context switch
    void RestoreState();		// Restore address space on context switch

    void AddThread() { threadCount++; }
    void RemoveThread() { threadCount--; }
    bool CanDelete() { return threadCount <= 0; }

    static void ReserveTestPhysPages();	// Mark frames 0,2,4,6,8,10 (lab test)
    static void ReleaseTestPhysPages();	// Release reserved test frames

  private:
    void AllocatePageTable(unsigned int numPages);
    void LoadSegment(OpenFile *executable, int segmentSize,
                     unsigned int virtualAddr, int inFileAddr);
    int  AllocatePhysicalPage();
    void DeallocatePhysicalPage(int physPage);

    int threadCount;
    TranslationEntry *pageTable;
    unsigned int numPages;
    unsigned int numStackPages;
    unsigned int stackVirtualTop;
};

#endif // ADDRSPACE_H
