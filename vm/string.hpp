#ifndef STRING_HPP
#define STRING_HPP

#include <string>
#include <string.h>

#include "oop.hpp"
#include "utf8.hpp"

#include "util/murmur_hash3.hpp"

#include "gc_allocated.hpp"

namespace marius {
  class State;

  class String : public GCAllocated {
    const char* data_;
    size_t bytelen_;
    size_t charlen_;

  public:
    String(const char* data)
      : data_(data)
      , bytelen_(strlen(data))
      , charlen_(utf8_charlen(data, bytelen_))
    {}

    const char* c_str() {
      return data_;
    }

    size_t bytelen() {
      return bytelen_;
    }

    size_t charlen() {
      return charlen_;
    }

    unsigned hash();
    bool equal(String& other);

    static void init(State& S);
    static String& internalize(State& S, std::string str);

    static String& convert(State& S, OOP obj);
  };
}

#endif
