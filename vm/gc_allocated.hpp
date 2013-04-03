#ifndef GC_ALLOCATED_HPP
#define GC_ALLOCATED_HPP

#include <stddef.h>

namespace marius {
  class State;

  class GCAllocated {
    void* operator new(size_t size);

  public:
    void* operator new(size_t size, State& S);
  };
}

#endif
