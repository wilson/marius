#ifndef OOP_HPP
#define OOP_HPP

#include <stdint.h>

#include "bug.hpp"

namespace marius {

  const static uintptr_t cNil = 2;
  const static uintptr_t cFalse = 4;

  class Class;
  class Module;
  class String;
  class Code;
  class Method;
  class Unwind;
  class User;
  class Attributes;
  class Method;
  class Tuple;
  class State;
  class Closure;

  class GCImpl;

  class OOP {
  public:
    enum Type {
      // Value object types
      eNil, eTrue, eFalse, eInteger, eString, eUnwind, eCode, eMethod,
      eTuple, eRaw,

      // Mutable object types
      eClass, eUser, eModule, eClosure,
      TotalTypes
    };

    const static int cMutableObjects = eClass;

  private:
    friend class GCImpl;

    enum IntConstruct { IntConstruct };
    enum ImmConstruct { ImmConstruct };

    Type type_;

    union {
      int int_;
      String* string_;
      Code* code_;
      Unwind* unwind_;
      Method* method_;
      Tuple* tuple_;

      Class* class_;
      Module* module_;
      User* user_;
      Closure* closure_;
      void* raw_;
    };

  public:
    OOP()
      : type_(eNil)
      , int_(0)
    {}

    OOP(Class* cls)
      : type_(eClass)
      , class_(cls)
    {}

    OOP(Module* mod)
      : type_(eModule)
      , module_(mod)
    {}

    OOP(Code* c)
      : type_(eCode)
      , code_(c)
    {}

    OOP(Closure* c)
      : type_(eClosure)
      , closure_(c)
    {}

    OOP(enum IntConstruct _, int val)
      : type_(eInteger)
      , int_(val)
    {}

    OOP(enum ImmConstruct _, bool v)
      : type_(v ? eTrue : eFalse)
    {}

    OOP(String* str)
      : type_(eString)
      , string_(str)
    {}

    OOP(Code& code)
      : type_(eCode)
      , code_(&code)
    {}

    OOP(User* obj)
      : type_(eUser)
      , user_(obj)
    {}

    OOP(Unwind* u)
      : type_(eUnwind)
      , unwind_(u)
    {}

    OOP(Method* m)
      : type_(eMethod)
      , method_(m)
    {}

    OOP(Tuple* t)
      : type_(eTuple)
      , tuple_(t)
    {}

    Type type() {
      return type_;
    }

    bool value_p() {
      return type_ < cMutableObjects;
    }

    bool mutable_p() {
      return type_ >= cMutableObjects;
    }

    static OOP nil() {
      return OOP();
    }

    static OOP true_() {
      return OOP(ImmConstruct, true);
    }

    static OOP false_() {
      return OOP(ImmConstruct, false);
    }

    static OOP integer(int val) {
      return OOP(IntConstruct, val);
    }

    static OOP raw(void* obj) {
      OOP o;
      o.type_ = eRaw;
      o.raw_ = obj;

      return o;
    }

    static OOP wrap_klass(Class* cls) {
      return OOP(cls);
    }

    static OOP copy_of(OOP orig, void* val) {
      OOP o;
      o.type_ = orig.type_;
      o.raw_ = val;
      return o;
    }

    int int_value() {
      check(type_ == eInteger);
      return int_;
    }

    String* as_string() {
      check(type_ == eString);
      return string_;
    }

    Class* as_class() {
      check(type_ == eClass);
      return class_;
    }

    Code& as_code() {
      check(type_ == eCode);
      return *code_;
    }

    Method* as_method() {
      check(type_ == eMethod);
      return method_;
    }

    Module* as_module() {
      check(type_ == eModule);
      return module_;
    }

    User* as_obj() {
      check(type_ == eUser);
      return user_;
    }

    Tuple* as_tuple() {
      check(type_ == eTuple);
      return tuple_;
    }

    Attributes* as_attributes();

    bool true_condition_p() {
      return type_ != eFalse;
    }

    bool unwind_p() {
      return type_ == eUnwind;
    }

    Unwind* unwind_value() {
      check(type_ == eUnwind);
      return unwind_;
    }

    // template <typename T>
      // class Caster {
        // static T cast(OOP val) {
          // check(false);
        // }
      // };

    // template <>
      // class Caster<int> {
        // static int cast(OOP val) {
          // check(val.type_ == eInteger);
          // return val.int_;
        // }
      // }

    // template <typename T>
      // T as() {
        // return Caster<T>::cast(*this);
      // }

    template <typename T>
      T as() {
        check(false);
      }

    void* heap_address() {
      switch(type_) {
      case eInteger:
      case eNil:
      case eTrue:
      case eFalse:
        return 0;
      default:
        return raw_;
      }
    }

    Class* klass();

    Method* find_method(String* name);
    OOP set_attribute(State& S, String* name, OOP val);
    OOP attribute(String* name, bool* found=0);

    void print();
  };

#define SPEC(T, E, V) template <> inline T OOP::as<T>() { check(type_ == E); return V; }

  SPEC(Class*, eClass, class_);
  SPEC(Module*, eModule, module_);
  SPEC(String*, eString, string_);
  SPEC(Code*, eCode, code_);
  SPEC(User*, eUser, user_);
  SPEC(Unwind*, eUnwind, unwind_);
  SPEC(int, eInteger, int_);

#undef SPEC

}

#endif
