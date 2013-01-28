#include "ast.hpp"
#include "string_map.hpp"
#include "local.hpp"

#include <list>

namespace marius {
  using namespace ast;

  class ScopeTracker : public Visitor {
    typedef StringMap<Local*>::type LocalScope;

    LocalScope* scope_;

    std::list<LocalScope*> stack_;
    ArgMap& globals_;
    LocalMap& locals_;

  public:
    ScopeTracker(ArgMap& globals, LocalMap& locals)
      : scope_(new LocalScope)
      , globals_(globals)
      , locals_(locals)
    {}

    void visit(Import* imp) {
      int depth = stack_.size();

      ArgMap::iterator i = globals_.find(String::internalize("Importer"));

      assert(i != globals_.end());
      Local* l = new Local;
      l->make_global(i->second, depth);

      Local* lv = locals_.add(imp);
      lv->set_extra(l);

      scope_->insert(LocalScope::value_type(imp->name(), lv));
    }

    void before_visit(Class* cls) {
      int depth = stack_.size();

      ArgMap::iterator i = globals_.find(String::internalize("Class"));

      assert(i != globals_.end());
      Local* l = locals_.add(cls);
      l->make_global(i->second, depth);

      scope_->insert(LocalScope::value_type(cls->name(),
                                            locals_.add(cls->body())));
    }

    void before_visit(Scope* s) {
      stack_.push_back(scope_);
      scope_ = new LocalScope;

      // ArgMap& args = s->arguments();

      Arguments& args = s->arg_objs();

      for(Arguments::iterator i = args.begin();
          i != args.end();
          ++i)
      {
        Argument* a = *i;
        Local* l = new Local;
        l->make_arg(a->position());

        locals_.add(a, l);

        scope_->insert(LocalScope::value_type(a->name(), l));
      }
    }

    void visit(Scope* s) {
      int regs = 0;
      int closed = 0;

      for(LocalScope::iterator i = scope_->begin();
          i != scope_->end();
          ++i) {
        Local* l = i->second;
        if(l->reg_p()) {
          int r = regs++;
          i->second->set_reg(r);
          s->add_local(i->first, r);
        } else {
          int c = closed++;
          i->second->set_reg(c);
          s->add_closed_local(i->first, c);
        }
      }

      delete scope_;

      scope_ = stack_.back();

      stack_.pop_back();
    }

    void visit(Assign* a) {
      LocalScope::iterator j = scope_->find(a->name());
      if(j != scope_->end()) return;

      int d = 0;
      for(std::list<LocalScope*>::reverse_iterator i = stack_.rbegin();
          i != stack_.rend();
          ++i, d++)
      {
        LocalScope* s = *i;

        LocalScope::iterator j = s->find(a->name());
        if(j != s->end()) {
          j->second->make_closure();
          Local* l = locals_.add(a);
          l->make_closure_access(j->second, d);
        }
      }

      scope_->insert(LocalScope::value_type(a->name(), locals_.add(a)));
    }

    void visit(Named* n) {
      LocalScope::iterator j = scope_->find(n->name());
      if(j != scope_->end()) {
        locals_.add(n, j->second);
        return;
      }

      int depth = 1;

      for(std::list<LocalScope*>::reverse_iterator i = stack_.rbegin();
          i != stack_.rend();
          ++i, depth++) {
        LocalScope* s = *i;

        LocalScope::iterator j = s->find(n->name());
        if(j != s->end()) {
          j->second->make_closure();
          Local* l = locals_.add(n);
          l->make_closure_access(j->second, depth);
          return;
        }
      }

      ArgMap::iterator i = globals_.find(n->name());
      if(i != globals_.end()) {
        Local* l = locals_.add(n);
        l->make_global(i->second, depth-1);
      }
    }
  };

  void calculate_locals(ast::Node* top, ArgMap& globals, LocalMap& locals) {
    ScopeTracker tracker(globals, locals);
    top->accept(&tracker);
  }
}
