#ifndef H2O_H
#define H2O_H

#include "synch.h"

class H2O {
  public:
    H2O();
    ~H2O();
    void hReady(long who);
    void oReady(long who);
  private:
    Lock *lock;
    Condition *hCond;
    Condition *oCond;
    int hCount;
    int oCount;
};

#endif // H2O_H
