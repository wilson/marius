#include "parser_state.hpp"
#include <assert.h>

namespace marius {

  Instruction* ParserState::sequence() {
    size_t sz = context_->buffer.size();
    Instruction* seq = new Instruction[sz];

    for(size_t i = 0; i < sz; i++) {
      seq[i] = context_->buffer[i];
    }

    return seq;
  }

  void ParserState::set_top(ast::Node* b) {
    top_ = new ast::Scope(b, context_->local_names);
  }

  ast::Node* ParserState::seq(ast::Node* l, ast::Node* r) {
    return new ast::Seq(l, r);
  }

  ast::Node* ParserState::ast_class(String& name, ast::Node* body) {
    ast::Node* n = new ast::Class(name,
                     new ast::Scope(body, context_->local_names));

    delete context_;

    context_ = stack_.back();
    stack_.pop_back();

    return n;
  }

  void ParserState::start_def() {
    stack_.push_back(context_);

    context_ = new Context();
  }

  ast::Node* ParserState::ast_def(String& name, ast::Node* b) {
    ast::Node* n = new ast::Def(name, 
                     new ast::Scope(b, context_->local_names),
                     context_->args);
    delete context_;

    context_ = stack_.back();
    stack_.pop_back();

    return n;
  }

  void ParserState::def_arg(String& name) {
    context_->args[name] = context_->args.size();
  }

  ast::Node* ParserState::call(ast::Node* recv, String& n) {
    return new ast::Call(n, recv, ast::Nodes());
  }

  ast::Node* ParserState::named(String& s) {
    return new ast::Named(s);
  }

  void ParserState::start_class() {
    stack_.push_back(context_);

    context_ = new Context();
  }

  ast::Call* ParserState::ast_call(String& name, ast::Node* r, ast::Nodes args) {
    return new ast::Call(name, r, args);
  }

  ast::Call* ParserState::ast_binop(const char* s, ast::Node* a, ast::Node* b) {
    ast::Nodes nodes;
    nodes.push_back(b);

    return new ast::Call(String::internalize(s), a, nodes);
  }

  ast::Node* ParserState::number(int a) {
    return new ast::Number(a);
  }

  ast::Node* ParserState::ret(ast::Node* n) {
    return new ast::Return(n);
  }

  void ParserState::start_cascade(ast::Node* recv) {
    cascades_.push_back(new ast::Cascade(recv));
  }

  void ParserState::cascade(String& name) {
    cascades_.back()->push_message(new ast::CascadeCall(name));
  }

  ast::Node* ParserState::end_cascade() {
    ast::Node* n = cascades_.back();
    cascades_.pop_back();

    return n;
  }

  void ParserState::start_arg_list() {
    arg_infos_.push_back(arg_info_);

    arg_info_ = ArgInfo();
  }

  void ParserState::add_arg(ast::Node* a) {
    arg_info_.nodes.push_back(a);
  }

  void ParserState::add_kw_arg(String& name, ast::Node* a) {
    arg_info_.keywords[name] = arg_info_.nodes.size();
    arg_info_.nodes.push_back(a);
  }

  ast::Node* ParserState::call_args(ast::Node* recv, String& id) {
    ast::Node* n = 0;

    if(arg_info_.keywords.size() == 0) {
      n = new ast::Call(id, recv, arg_info_.nodes);
    } else {
      n = new ast::CallWithKeywords(id, recv, arg_info_.nodes,
                                    arg_info_.keywords);
    }

    arg_info_ = arg_infos_.back();
    arg_infos_.pop_back();

    assert(n);

    return n;
  }

  ast::Node* ParserState::call_kw_args(ast::Node* r, String& id) {
    ast::Node* n = new ast::CallWithKeywords(id, r, arg_info_.nodes,
                                arg_info_.keywords);

    arg_info_ = arg_infos_.back();
    arg_infos_.pop_back();

    return n;
  }

  ast::Node* ParserState::if_cond(ast::Node* cond, ast::Node* body) {
    return new ast::IfCond(cond, body);
  }

  ast::Node* ParserState::ast_nil() {
    return new ast::Nil();
  }

  ast::Node* ParserState::ast_true() {
    return new ast::True();
  }

  ast::Node* ParserState::ast_false() {
    return new ast::False();
  }

  ast::Node* ParserState::import(String& name) {
    return new ast::Import(name);
  }

  ast::Node* ParserState::ast_try(ast::Node* b, ast::Node* h) {
    return new ast::Try(b, h);
  }

  ast::Node* ParserState::assign(String& name, ast::Node* n) {
    int r = new_local();
    context_->local_names[name] = r;
    return new ast::Assign(name, r, n);
  }

  /*
  int ParserState::start_cond(int c) {
    push(GOTO_IF_FALSE);
    push(c);

    int l = buffer_.size();
    push(0);

    return l;
  }

  int ParserState::end_cond(int b, int l) {

  }
  */

}
