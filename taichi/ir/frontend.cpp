// Frontend constructs

#include "taichi/ir/frontend.h"
#include "taichi/ir/statements.h"

TLANG_NAMESPACE_BEGIN

Expr global_new(Expr id_expr, DataType dt) {
  TI_ASSERT(id_expr.is<IdExpression>());
  auto ret = Expr(std::make_shared<GlobalVariableExpression>(
      dt, id_expr.cast<IdExpression>()->id));
  return ret;
}

Expr global_new(DataType dt, std::string name) {
  auto id_expr = std::make_shared<IdExpression>(name);
  return Expr::make<GlobalVariableExpression>(dt, id_expr->id);
}

Expr copy(const Expr &expr) {
  auto e = expr.eval();
  auto stmt = Stmt::make<ElementShuffleStmt>(
      VectorElement(e.cast<EvalExpression>()->stmt_ptr, 0));
  auto eval_expr = std::make_shared<EvalExpression>(stmt.get());
  current_ast_builder().insert(std::move(stmt));
  return Expr(eval_expr);
}

void insert_snode_access_flag(SNodeAccessFlag v, const Expr &field) {
  dec.mem_access_opt.add_flag(field.snode(), v);
}

void reset_snode_access_flag() {
  dec.reset();
}

// Begin: legacy frontend constructs

If::If(const Expr &cond) {
  auto stmt_tmp = std::make_unique<FrontendIfStmt>(cond);
  stmt = stmt_tmp.get();
  current_ast_builder().insert(std::move(stmt_tmp));
}

If::If(const Expr &cond, const std::function<void()> &func) : If(cond) {
  Then(func);
}

If &If::Then(const std::function<void()> &func) {
  auto _ = current_ast_builder().create_scope(stmt->true_statements);
  func();
  return *this;
}

If &If::Else(const std::function<void()> &func) {
  auto _ = current_ast_builder().create_scope(stmt->false_statements);
  func();
  return *this;
}

For::For(const Expr &s, const Expr &e, const std::function<void(Expr)> &func) {
  auto i = Expr(std::make_shared<IdExpression>());
  auto stmt_unique = std::make_unique<FrontendForStmt>(i, s, e);
  auto stmt = stmt_unique.get();
  current_ast_builder().insert(std::move(stmt_unique));
  auto _ = current_ast_builder().create_scope(stmt->body);
  func(i);
}

For::For(const Expr &i,
         const Expr &s,
         const Expr &e,
         const std::function<void()> &func) {
  auto stmt_unique = std::make_unique<FrontendForStmt>(i, s, e);
  auto stmt = stmt_unique.get();
  current_ast_builder().insert(std::move(stmt_unique));
  auto _ = current_ast_builder().create_scope(stmt->body);
  func();
}

For::For(const ExprGroup &i,
         const Expr &global,
         const std::function<void()> &func) {
  auto stmt_unique = std::make_unique<FrontendForStmt>(i, global);
  auto stmt = stmt_unique.get();
  current_ast_builder().insert(std::move(stmt_unique));
  auto _ = current_ast_builder().create_scope(stmt->body);
  func();
}

While::While(const Expr &cond, const std::function<void()> &func) {
  auto while_stmt = std::make_unique<FrontendWhileStmt>(cond);
  FrontendWhileStmt *ptr = while_stmt.get();
  current_ast_builder().insert(std::move(while_stmt));
  auto _ = current_ast_builder().create_scope(ptr->body);
  func();
}

// End: legacy frontend constructs

TLANG_NAMESPACE_END
