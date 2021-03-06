#include "compiler.hpp"
#include "parser.hpp"
#include "ast.hpp"

#include "scope_tracker.hpp"
#include "local.hpp"

namespace marius {
  bool Compiler::compile(State& S, FILE* f) {
    Parser parser(S, f);

    if(!parser.parse(debug_)) return false;

    ast::Scope* top = parser.top();

    if(!top) return false;

    ArgMap globals;

    globals[String::internalize(S, "Object")] = 0;
    globals[String::internalize(S, "io")] = 1;
    globals[String::internalize(S, "Class")] = 2;
    globals[String::internalize(S, "Importer")] = 3;

    LocalMap locals;

    calculate_locals(S, top, globals, locals);

    ast::State AS(S, locals);

    top->drive(AS, top->locals().size());

    ArgMap args;

    code_ = AS.to_code(String::internalize(S, "__main__"), args, top->cov());

    if(debug_) code_->print();

    return true;
  }
}
