#include "h2o.h"
#include <stdio.h>

H2O::H2O() {
    lock = new Lock("H2O Lock");
    hCond = new Condition("H Condition");
    oCond = new Condition("O Condition");
    hCount = 0;
    oCount = 0;
}

H2O::~H2O() {
    delete lock;
    delete hCond;
    delete oCond;
}

void H2O::hReady(long who) {
    lock->Acquire();
    hCount++;
    if (hCount >= 2 && oCount >= 1) {
        hCount -= 2;
        oCount -= 1;
        printf("Hidrógeno %ld formó una molécula de agua\n", who);
        hCond->Signal(lock);
        oCond->Signal(lock);
    } else {
        hCond->Wait(lock);
        printf("Hidrógeno %ld formó una molécula de agua\n", who);
    }
    lock->Release();
}

void H2O::oReady(long who) {
    lock->Acquire();
    oCount++;
    if (hCount >= 2 && oCount >= 1) {
        hCount -= 2;
        oCount -= 1;
        printf("Oxígeno %ld formó una molécula de agua\n", who);
        hCond->Signal(lock);
        hCond->Signal(lock);
    } else {
        oCond->Wait(lock);
        printf("Oxígeno %ld formó una molécula de agua\n", who);
    }
    lock->Release();
}
