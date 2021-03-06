#include "environment.hpp"
#include "memory_object.hpp"
#include "state.hpp"

#include "class.hpp"
#include "compiler.hpp"
#include "vm.hpp"

#include "module.hpp"
#include "settings.hpp"
#include "state.hpp"
#include "method.hpp"
#include "closure.hpp"

#include "unwind.hpp"

#include <string.h>

namespace marius {
  namespace {

    const char* find_path(State& S, String* req) {
      std::vector<const char*>& lp = S.settings().load_path();

      char* path = new char[1024];

      for(std::vector<const char*>::iterator i = lp.begin();
          i != lp.end();
          ++i) {
        strcat(path, *i);
        strcat(path, "/");
        strcat(path, req->c_str());
        strcat(path, ".mr");

        if(access(path, R_OK) == 0) return path;
      }

      delete path;
      return 0;
    }

    Handle import(State& S, Handle recv, Arguments& args) {
      String* name = args[0]->as_string();

      const char* path = find_path(S, name);

      if(!path) {
        return handle(S, Unwind::import_error(S, name));
      }

      FILE* file = fopen(path, "r");
      delete path;

      if(!file) {
        return handle(S, Unwind::import_error(S, name));
      }

      Compiler compiler;

      if(!compiler.compile(S, file)) {
        return handle(S, Unwind::import_error(S, name));
      }

      Module* m = new(S) Module(S,
          S.env().lookup(S, "Module").as_class(), name);

      OOP* fp = args.frame() + 1;
      fp[0] = m;

      Code& code = *compiler.code();

      Method* top = new(S) Method(name, code, S.env().globals());

      S.vm().run(S, top, fp + 1);

      return handle(S, m);
    }

    Handle current(State& S, Handle recv, Arguments& args) {
      return handle(S, S.importer());
    }
  }

  Class* init_import(State& S) {
    Class* x = S.env().new_class(S, "Importer");
    x->add_method(S, "import", import, 1);
    x->add_class_method(S, "current", current, 0);

    return x;
  }
}
