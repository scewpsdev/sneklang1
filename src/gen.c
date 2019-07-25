#include "gen.h"

#include <string.h>

#include <llvm-c/TargetMachine.h>
#include <llvm-c/Linker.h>

SCOPE* scope_new(SCOPE* parent) {
	SCOPE* scope = malloc(sizeof(SCOPE));
	*scope = (SCOPE){ parent };

	scope->break_dest = NULL;
	scope->continue_dest = NULL;

	scope->locals_k = strvec_new(8);
	scope->locals_v = valvec_new(8);

	return scope;
}

void scope_delete(SCOPE* scope) {
	strvec_delete(&scope->locals_k);
	valvec_delete(&scope->locals_v);
	free(scope);
}

CODEGEN gen_new() {
	CODEGEN g;

	g.llvm_context = LLVMContextCreate();
	g.llvm_builder = LLVMCreateBuilderInContext(g.llvm_context);
	g.module_vec = mdvec_new(2);
	g.module_name_vec = strvec_new(2);

	g.ast = NULL;
	g.current_scope = NULL;
	g.llvm_module = NULL;
	g.llvm_func = NULL;
	g.has_branched = false;

	g.globals_k = strvec_new(8);
	g.globals_v = valvec_new(8);

	LLVMInitializeX86TargetInfo();
	LLVMInitializeX86Target();
	LLVMInitializeX86TargetMC();
	LLVMInitializeX86AsmParser();
	LLVMInitializeX86AsmPrinter();

	return g;
}

void gen_delete(CODEGEN* codegen) {
	mdvec_delete(&codegen->module_vec);
	for (int i = 0; i < codegen->module_name_vec.size; i++) free(codegen->module_name_vec.buffer[i]);
	strvec_delete(&codegen->module_name_vec);
	strvec_delete(&codegen->globals_k);
	valvec_delete(&codegen->globals_v);
}

LLVMValueRef find_local_value(SCOPE* s, char* name) {
	for (int i = 0; i < s->locals_k.size; i++) {
		if (strcmp(s->locals_k.buffer[i], name) == 0) return s->locals_v.buffer[i];
	}
	return NULL;
}

LLVMValueRef find_global_value(CODEGEN* g, char* name) {
	for (int i = 0; i < g->globals_k.size; i++) {
		if (strcmp(g->globals_k.buffer[i], name) == 0) return g->globals_v.buffer[i];
	}
	return NULL;
}

LLVMValueRef find_named_value(CODEGEN* g, SCOPE* s, char* name) {
	if (!s) return NULL;
	LLVMValueRef result = NULL;
	if (result = find_local_value(s, name)) return result;
	if (result = find_named_value(g, s->parent, name)) return result;
	if (result = find_global_value(g, name)) return result;
	return NULL;
}

LLVMTypeRef get_llvm_type_from_str(char* name, bool cpy) {
	LLVMTypeRef val_type = NULL;
	if (strlen(name) >= 2 && name[0] == 'i') {
		int bitsize = strtol(name + 1, NULL, 10);
		val_type = LLVMIntType(bitsize);
	}
	if (!val_type) return NULL;
	return cpy ? val_type : LLVMPointerType(val_type, 0);
}

LLVMTypeRef get_llvm_type(TYPE* type) {
	return get_llvm_type_from_str(type->name, type->cpy);
}

LLVMValueRef cast_value(CODEGEN* g, LLVMValueRef val, LLVMTypeRef type) {
	LLVMTypeRef val_type = LLVMTypeOf(val);
	if (LLVMTypeOf(val) == type) return val;
	// Int-int cast
	if (LLVMGetTypeKind(val_type) == LLVMIntegerTypeKind && LLVMGetTypeKind(type) == LLVMIntegerTypeKind) {
		return LLVMGetIntTypeWidth(val_type) > LLVMGetIntTypeWidth(type)
			? LLVMBuildTrunc(g->llvm_builder, val, type, "")
			: LLVMBuildSExt(g->llvm_builder, val, type, "");
	}
	// Float-int cast
	if ((LLVMGetTypeKind(val_type) == LLVMFloatTypeKind || LLVMGetTypeKind(val_type) == LLVMDoubleTypeKind)
		&& LLVMGetTypeKind(type) == LLVMIntegerTypeKind) {
		return LLVMBuildFPToSI(g->llvm_builder, val, type, "");
	}
	// Int-float cast
	if (LLVMGetTypeKind(val_type) == LLVMIntegerTypeKind
		&& (LLVMGetTypeKind(val_type) == LLVMFloatTypeKind || LLVMGetTypeKind(val_type) == LLVMDoubleTypeKind)) {
		return LLVMBuildSIToFP(g->llvm_builder, val, type, "");
	}
	return NULL;
}

LLVMValueRef alloc_value(CODEGEN* g, char* name, LLVMTypeRef type, SCOPE* scope) {
	LLVMBasicBlockRef entry_block = LLVMGetEntryBasicBlock(g->llvm_func);
	LLVMBuilderRef new_builder = LLVMCreateBuilderInContext(g->llvm_context);
	LLVMValueRef first_inst = NULL;
	if (first_inst = LLVMGetFirstInstruction(entry_block)) {
		LLVMPositionBuilderBefore(new_builder, first_inst);
	} else {
		LLVMPositionBuilderAtEnd(new_builder, entry_block);
	}
	LLVMValueRef ptr = LLVMBuildAlloca(new_builder, type, name);

	return ptr;
}

LLVMValueRef alloc_value_with_content(CODEGEN* g, char* name, LLVMValueRef value, SCOPE* scope) {
	LLVMValueRef ptr = alloc_value(g, name, LLVMTypeOf(value), scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);
	return ptr;
}

LLVMValueRef gen_expr(CODEGEN*, EXPRESSION*);
LLVMValueRef gen_ast(CODEGEN*, AST*);

LLVMValueRef gen_int_literal(CODEGEN* g, INT* i) {
	LLVMValueRef value = LLVMConstInt(LLVMInt64Type(), i->value, true);
	LLVMValueRef ptr = alloc_value(g, "", LLVMTypeOf(value), g->current_scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);
	return ptr;
}

LLVMValueRef gen_char_literal(CODEGEN* g, CHAR* ch) {
	LLVMValueRef value = LLVMConstInt(LLVMInt8Type(), ch->value, false);
	LLVMValueRef ptr = alloc_value(g, "", LLVMTypeOf(value), g->current_scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);
	return ptr;
}

LLVMValueRef gen_bool_literal(CODEGEN* g, BOOL* b) {
	LLVMValueRef value = LLVMConstInt(LLVMInt1Type(), b->value, false);
	LLVMValueRef ptr = alloc_value(g, "", LLVMTypeOf(value), g->current_scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);
	return ptr;
}

LLVMValueRef gen_float_literal(CODEGEN* g, FLOAT* f) {
	LLVMValueRef value = LLVMConstReal(LLVMDoubleType(), f->value);
	LLVMValueRef ptr = alloc_value(g, "", LLVMTypeOf(value), g->current_scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);
	return ptr;
}

LLVMValueRef gen_string_literal(CODEGEN* g, STRING* str) {
	LLVMValueRef value = LLVMConstString(str->value, (unsigned int)strlen(str->value), false);
	LLVMValueRef ptr = alloc_value(g, "", LLVMArrayType(LLVMInt8Type(), (unsigned int)strlen(str->value) + 1), g->current_scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);
	return LLVMBuildBitCast(g->llvm_builder, ptr, LLVMPointerType(LLVMInt8Type(), 0), "");
}

LLVMValueRef gen_identifier(CODEGEN* g, IDENTIFIER* i) {
	return find_named_value(g, g->current_scope, i->name);
}

LLVMValueRef gen_compound_expr(CODEGEN* g, COMPOUND_EXPR* compound_expr) {
	return gen_expr(g, compound_expr->expr);
}

LLVMValueRef create_binary_op(CODEGEN* g, const char* op, LLVMValueRef left, LLVMValueRef right) {
	LLVMTypeRef ltype = LLVMTypeOf(left);
	LLVMTypeRef rtype = LLVMTypeOf(right);
	if (LLVMGetTypeKind(ltype) == LLVMIntegerTypeKind && LLVMGetTypeKind(rtype) == LLVMIntegerTypeKind) {
		LLVMTypeRef type = LLVMIntType(max(LLVMGetIntTypeWidth(ltype), LLVMGetIntTypeWidth(rtype)));
		left = cast_value(g, left, type);
		right = cast_value(g, right, type);
		if (strcmp(op, "+") == 0) return LLVMBuildAdd(g->llvm_builder, left, right, "");
		if (strcmp(op, "-") == 0) return LLVMBuildSub(g->llvm_builder, left, right, "");
		if (strcmp(op, "*") == 0) return LLVMBuildMul(g->llvm_builder, left, right, "");
		if (strcmp(op, "/") == 0) return LLVMBuildSDiv(g->llvm_builder, left, right, "");
		if (strcmp(op, "%") == 0) return LLVMBuildSRem(g->llvm_builder, left, right, "");
		if (strcmp(op, "==") == 0) return LLVMBuildICmp(g->llvm_builder, LLVMIntEQ, left, right, "");
		if (strcmp(op, "!=") == 0) return LLVMBuildICmp(g->llvm_builder, LLVMIntNE, left, right, "");
		if (strcmp(op, "<") == 0) return LLVMBuildICmp(g->llvm_builder, LLVMIntSLT, left, right, "");
		if (strcmp(op, ">") == 0) return LLVMBuildICmp(g->llvm_builder, LLVMIntSGT, left, right, "");
		if (strcmp(op, "<=") == 0) return LLVMBuildICmp(g->llvm_builder, LLVMIntSLE, left, right, "");
		if (strcmp(op, ">=") == 0) return LLVMBuildICmp(g->llvm_builder, LLVMIntSGE, left, right, "");
		if (strcmp(op, "&&") == 0) return LLVMBuildAnd(g->llvm_builder, left, right, "");
		if (strcmp(op, "||") == 0) return LLVMBuildOr(g->llvm_builder, left, right, "");
	}
	return NULL;
}

LLVMValueRef gen_assign(CODEGEN* g, ASSIGN* assign) {
	LLVMValueRef left = gen_expr(g, assign->left);
	LLVMValueRef right = gen_expr(g, assign->right);
	if (strcmp(assign->op, "=") == 0 && !left) {
		if (assign->left->type != EXPR_TYPE_IDENTIFIER) {
			// TODO ERROR
		}
		strvec_push(&g->current_scope->locals_k, assign->left->identifier.name);
		valvec_push(&g->current_scope->locals_v, right);
	} else if (assign->op[strlen(assign->op) - 1] == '=') {
		LLVMValueRef value = NULL;
		switch (assign->op[0]) {
		case '=': value = LLVMBuildLoad(g->llvm_builder, right, ""); break;
		case '+': value = create_binary_op(g, "+", LLVMBuildLoad(g->llvm_builder, left, ""), LLVMBuildLoad(g->llvm_builder, right, "")); break;
		case '-': value = create_binary_op(g, "-", LLVMBuildLoad(g->llvm_builder, left, ""), LLVMBuildLoad(g->llvm_builder, right, "")); break;
		case '*': value = create_binary_op(g, "*", LLVMBuildLoad(g->llvm_builder, left, ""), LLVMBuildLoad(g->llvm_builder, right, "")); break;
		case '/': value = create_binary_op(g, "/", LLVMBuildLoad(g->llvm_builder, left, ""), LLVMBuildLoad(g->llvm_builder, right, "")); break;
		case '%': value = create_binary_op(g, "%", LLVMBuildLoad(g->llvm_builder, left, ""), LLVMBuildLoad(g->llvm_builder, right, "")); break;
		}
		LLVMBuildStore(g->llvm_builder, cast_value(g, value, LLVMGetElementType(LLVMTypeOf(left))), left);
	}

	return right;
}

LLVMValueRef gen_binary_op(CODEGEN* g, BINARY_OP* binary_op) {
	LLVMValueRef value = create_binary_op(g, binary_op->op, LLVMBuildLoad(g->llvm_builder, gen_expr(g, binary_op->left), ""), LLVMBuildLoad(g->llvm_builder, gen_expr(g, binary_op->right), ""));
	LLVMValueRef ptr = alloc_value(g, "", LLVMTypeOf(value), g->current_scope);
	LLVMBuildStore(g->llvm_builder, value, ptr);

	return ptr;
}

LLVMValueRef gen_unary_op(CODEGEN* g, UNARY_OP* unary_op) {
	LLVMValueRef expr = gen_expr(g, unary_op->expr);

	if (strcmp(unary_op->op, "*") == 0) return LLVMBuildLoad(g->llvm_builder, expr, "abc");
	if (strcmp(unary_op->op, "++") == 0 || strcmp(unary_op->op, "--") == 0) {
		LLVMValueRef initial_value = LLVMBuildLoad(g->llvm_builder, expr, "");
		char* op = strcmp(unary_op->op, "++") == 0 ? "+" : "-";
		LLVMValueRef result = create_binary_op(g, op, initial_value, LLVMConstInt(LLVMInt64Type(), 1, true));
		LLVMBuildStore(g->llvm_builder, result, expr);
		return unary_op->position ? alloc_value_with_content(g, "", initial_value, g->current_scope) : expr;
	}

	return NULL;
}

LLVMValueRef gen_compound(CODEGEN* g, COMPOUND* compound) {
	return gen_ast(g, compound->ast);
}

LLVMValueRef gen_return(CODEGEN* g, RETURN* ret_statement) {
	return NULL;
}

LLVMValueRef gen_if_statement(CODEGEN* g, IF* if_statement) {
	LLVMValueRef condition = cast_value(g, LLVMBuildLoad(g->llvm_builder, gen_expr(g, if_statement->condition), ""), LLVMInt1Type());
	LLVMBasicBlockRef before_block = LLVMGetInsertBlock(g->llvm_builder);

	LLVMBasicBlockRef then_block = LLVMAppendBasicBlockInContext(g->llvm_context, g->llvm_func, "then");
	LLVMBasicBlockRef else_block = LLVMAppendBasicBlockInContext(g->llvm_context, g->llvm_func, "else");
	LLVMBasicBlockRef merge_block = LLVMAppendBasicBlockInContext(g->llvm_context, g->llvm_func, "merge");

	LLVMPositionBuilderAtEnd(g->llvm_builder, before_block);
	LLVMBuildCondBr(g->llvm_builder, condition, then_block, else_block);

	LLVMPositionBuilderAtEnd(g->llvm_builder, then_block);
	LLVMValueRef then_result = gen_expr(g, if_statement->then_block);
	if (!g->has_branched) LLVMBuildBr(g->llvm_builder, merge_block);
	else g->has_branched = false;
	then_block = LLVMGetInsertBlock(g->llvm_builder);

	LLVMPositionBuilderAtEnd(g->llvm_builder, else_block);
	LLVMValueRef else_result = NULL;
	if (if_statement->else_block) {
		else_result = gen_expr(g, if_statement->else_block);
		if (!g->has_branched) LLVMBuildBr(g->llvm_builder, merge_block);
		else g->has_branched = false;
		else_block = LLVMGetInsertBlock(g->llvm_builder);
	} else LLVMBuildBr(g->llvm_builder, merge_block);

	LLVMPositionBuilderAtEnd(g->llvm_builder, merge_block);

	if (then_result && else_result) {
		if (LLVMTypeOf(then_result) != LLVMTypeOf(else_result)) {
			// TODO ERROR
			else_result = cast_value(g, else_result, LLVMTypeOf(then_result));
		}
		LLVMValueRef phi = LLVMBuildPhi(g->llvm_builder, LLVMTypeOf(then_result), "");
		LLVMValueRef incoming_values[] = { then_result, else_result };
		LLVMBasicBlockRef incoming_blocks[] = { then_block, else_block };
		LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);
		return phi;
	}
	if (then_result) return then_result;
	else if (else_result) return else_result;
	else return NULL;
}

LLVMValueRef gen_loop(CODEGEN* g, LOOP* loop) {
	LLVMBasicBlockRef before_block = LLVMGetInsertBlock(g->llvm_builder);

	LLVMBasicBlockRef head_block = LLVMAppendBasicBlockInContext(g->llvm_context, g->llvm_func, "head");
	LLVMBasicBlockRef loop_block = LLVMAppendBasicBlockInContext(g->llvm_context, g->llvm_func, "loop");
	LLVMBasicBlockRef merge_block = LLVMAppendBasicBlockInContext(g->llvm_context, g->llvm_func, "merge");

	g->current_scope->break_dest = merge_block;
	g->current_scope->continue_dest = head_block;

	LLVMPositionBuilderAtEnd(g->llvm_builder, before_block);
	LLVMBuildBr(g->llvm_builder, head_block);

	LLVMPositionBuilderAtEnd(g->llvm_builder, head_block);
	if (loop->condition) {
		LLVMValueRef condition = cast_value(g, LLVMBuildLoad(g->llvm_builder, gen_expr(g, loop->condition), ""), LLVMInt1Type());
		LLVMBuildCondBr(g->llvm_builder, condition, loop_block, merge_block);
	} else LLVMBuildBr(g->llvm_builder, loop_block);

	LLVMPositionBuilderAtEnd(g->llvm_builder, loop_block);
	gen_expr(g, loop->body);
	if (!g->has_branched) LLVMBuildBr(g->llvm_builder, head_block);
	else g->has_branched = false;

	LLVMPositionBuilderAtEnd(g->llvm_builder, merge_block);

	g->current_scope->break_dest = NULL;
	g->current_scope->continue_dest = NULL;

	return NULL;
}

LLVMValueRef gen_break(CODEGEN* g, BREAK* break_statement) {
	SCOPE* scope = g->current_scope;
	int i = 0;
	while (true) {
		if (!scope->break_dest) scope = scope->parent;
		else {
			if (i >= break_statement->idx) break;
			else {
				i++;
			}
		}
	}

	g->has_branched = true;
	LLVMBasicBlockRef dest = scope->break_dest;
	LLVMBuildBr(g->llvm_builder, dest);

	return NULL;
}

LLVMValueRef gen_continue(CODEGEN* g, CONTINUE* continue_statement) {
	SCOPE* scope = g->current_scope;
	int i = 0;
	while (true) {
		if (!scope->continue_dest) scope = scope->parent;
		else {
			if (i >= continue_statement->idx) break;
			else {
				i++;
			}
		}
	}

	g->has_branched = true;
	LLVMBasicBlockRef dest = scope->continue_dest;
	LLVMBuildBr(g->llvm_builder, dest);

	return NULL;
}

LLVMValueRef gen_func_call(CODEGEN* g, FUNC_CALL* func_call) {
	// Cast
	if (func_call->callee->type == EXPR_TYPE_IDENTIFIER && func_call->num_args == 1) {
		LLVMTypeRef llvm_type = NULL;
		if (llvm_type = get_llvm_type_from_str(func_call->callee->identifier.name, true)) {
			LLVMValueRef value = cast_value(g, LLVMBuildLoad(g->llvm_builder, gen_expr(g, &func_call->args[0]), ""), llvm_type);
			LLVMValueRef ptr = alloc_value(g, "", LLVMTypeOf(value), g->current_scope);
			LLVMBuildStore(g->llvm_builder, value, ptr);
			return ptr;
		}
	}

	LLVMValueRef callee = gen_expr(g, func_call->callee);

	LLVMValueRef* args = malloc(func_call->num_args * sizeof(LLVMValueRef));
	LLVMTypeRef* arg_types = malloc(LLVMCountParams(callee) * sizeof(LLVMTypeRef));
	for (int i = 0; i < func_call->num_args; i++) {
		LLVMValueRef arg = gen_expr(g, &func_call->args[i]);
		LLVMTypeRef arg_type = LLVMTypeOf(arg);
		LLVMTypeRef param_type = LLVMTypeOf(LLVMGetParam(callee, i));
		if (LLVMGetTypeKind(arg_type) == LLVMGetTypeKind(param_type)) {
			if (LLVMGetTypeKind(arg_type) == LLVMPointerTypeKind) args[i] = arg;
			else args[i] = cast_value(g, arg, param_type);
		} else {
			if (LLVMGetTypeKind(arg_type) == LLVMPointerTypeKind) {
				LLVMValueRef value = LLVMBuildLoad(g->llvm_builder, arg, "");
				args[i] = cast_value(g, value, param_type);
			} else if (LLVMGetTypeKind(param_type) == LLVMPointerTypeKind) {
				LLVMValueRef ptr = LLVMBuildAlloca(g->llvm_builder, LLVMGetElementType(param_type), "");
				LLVMBuildStore(g->llvm_builder, cast_value(g, arg, LLVMGetElementType(param_type)), ptr);
				args[i] = ptr;
			}
		}
	}
	LLVMValueRef ret_val = LLVMBuildCall(g->llvm_builder, callee, args, func_call->num_args, "");
	free(args);
	return ret_val;
}

LLVMValueRef gen_func_decl(CODEGEN* g, FUNC_DECL* func_decl) {
	LLVMTypeRef* arg_types = malloc(func_decl->num_args * sizeof(LLVMTypeRef));
	for (int i = 0; i < func_decl->num_args; i++) {
		arg_types[i] = get_llvm_type(&func_decl->args[i].type);
	}
	LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32Type(), arg_types, func_decl->num_args, false);
	LLVMValueRef func = LLVMAddFunction(g->llvm_module, func_decl->funcname, func_type);
	strvec_push(&g->globals_k, func_decl->funcname);
	valvec_push(&g->globals_v, func);
	return func;
}

LLVMValueRef gen_import(CODEGEN* g, IMPORT* import) {
	DYNAMIC_STRING init_func_name = string_new(8);
	string_push_s(&init_func_name, "__");
	string_push_s(&init_func_name, import->module_name);
	string_push_s(&init_func_name, "_init");
	LLVMValueRef init_func = LLVMAddFunction(g->llvm_module, init_func_name.buffer, LLVMFunctionType(LLVMInt32Type(), NULL, 0, false));
	return LLVMBuildCall(g->llvm_builder, init_func, NULL, 0, "");
	string_delete(&init_func_name);
}

LLVMValueRef gen_expr(CODEGEN* g, EXPRESSION* expr) {
	switch (expr->type) {
	case EXPR_TYPE_INT_LITERAL: return gen_int_literal(g, &expr->int_literal);
	case EXPR_TYPE_CHAR_LITERAL: return gen_char_literal(g, &expr->char_literal);
	case EXPR_TYPE_BOOL_LITERAL: return gen_bool_literal(g, &expr->bool_literal);
	case EXPR_TYPE_FLOAT_LITERAL: return gen_float_literal(g, &expr->float_literal);
	case EXPR_TYPE_STRING_LITERAL: return gen_string_literal(g, &expr->string_literal);
	case EXPR_TYPE_IDENTIFIER: return gen_identifier(g, &expr->identifier);
	case EXPR_TYPE_COMPOUND_EXPR: return gen_compound_expr(g, &expr->compound_expr);

	case EXPR_TYPE_ASSIGN: return gen_assign(g, &expr->assign);
	case EXPR_TYPE_BINARY_OP: return gen_binary_op(g, &expr->binary_op);
	case EXPR_TYPE_UNARY_OP: return gen_unary_op(g, &expr->unary_op);
	case EXPR_TYPE_COMPOUND: return gen_compound(g, &expr->compound);
	case EXPR_TYPE_RETURN: return gen_return(g, &expr->ret_statement);
	case EXPR_TYPE_IF_STATEMENT: return gen_if_statement(g, &expr->if_statement);
	case EXPR_TYPE_LOOP: return gen_loop(g, &expr->loop);
	case EXPR_TYPE_BREAK: return gen_break(g, &expr->break_statement);
	case EXPR_TYPE_CONTINUE: return gen_continue(g, &expr->continue_statement);
	case EXPR_TYPE_FUNC_CALL: return gen_func_call(g, &expr->func_call);
	case EXPR_TYPE_FUNC_DECL: return gen_func_decl(g, &expr->func_decl);

	case EXPR_TYPE_IMPORT: return gen_import(g, &expr->import);

	default: return NULL;
	}
}

LLVMValueRef gen_ast(CODEGEN* g, AST* ast) {
	SCOPE* parent = g->current_scope;
	g->current_scope = scope_new(parent);

	LLVMValueRef ret_value = NULL;
	for (int i = 0; i < ast->num_expressions; i++) {
		ret_value = gen_expr(g, &ast->expressions[i]);
		if (g->has_branched) break;
	}

	scope_delete(g->current_scope);
	g->current_scope = parent;

	return ret_value;
}

void gen_toplevel(CODEGEN* g, AST* ast, char* module_name) {
	char* init_func_name = malloc(strlen(module_name) + 8);
	memcpy(init_func_name + 2, module_name, strlen(module_name));
	memcpy(init_func_name, "__", 2);
	memcpy(init_func_name + 2 + strlen(module_name), "_init", 5);
	init_func_name[strlen(module_name) + 2 + 5] = 0;

	LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, false);
	LLVMValueRef function = LLVMAddFunction(g->llvm_module, init_func_name, func_type);
	LLVMSetLinkage(function, LLVMExternalLinkage);
	g->llvm_func = function;

	LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function, "entry");
	LLVMPositionBuilderAtEnd(g->llvm_builder, entry_block);

	gen_ast(g, ast);

	LLVMBuildRet(g->llvm_builder, LLVMConstInt(LLVMInt32Type(), 0, true));
	g->llvm_func = NULL;

	free(init_func_name);
}

void gen_create_module(CODEGEN* g, AST* ast, char* module_name) {
	g->llvm_module = LLVMModuleCreateWithNameInContext(module_name, g->llvm_context);
	g->ast = ast;

	gen_toplevel(g, ast, module_name);

	puts(LLVMPrintModuleToString(g->llvm_module));
	printf("-----\n\n");
	LLVMPassManagerRef pass_manager = LLVMCreatePassManager();
	LLVMRunPassManager(pass_manager, g->llvm_module);

	mdvec_push(&g->module_vec, g->llvm_module);
	strvec_push(&g->module_name_vec, module_name);
}

void output_module(LLVMModuleRef module, char* output_file) {
	LLVMSetTarget(module, LLVM_DEFAULT_TARGET_TRIPLE);

	LLVMTargetRef target;
	char* error = NULL;
	const char* target_triple = LLVMGetTarget(module);
	LLVMGetTargetFromTriple(target_triple, &target, &error);
	if (!target) {
		printf(error);
		return;
	}
	char* cpu = "generic";
	char* features = "";
	LLVMCodeGenOptLevel level = LLVMCodeGenLevelDefault;
	LLVMRelocMode reloc = LLVMRelocDefault;
	LLVMCodeModel code_model = LLVMCodeModelDefault;
	LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(target, target_triple, cpu, features, level, reloc, code_model);

	LLVMCodeGenFileType filetype = LLVMObjectFile;
	error = NULL;
	LLVMTargetMachineEmitToFile(target_machine, module, output_file, filetype, &error);
	if (error) {
		printf(error);
		return;
	}
}

void gen_link(CODEGEN* g) {
	LLVMModuleRef root_module = LLVMModuleCreateWithNameInContext("__root", g->llvm_context);
	LLVMTypeRef entry_point_arg_types[] = { LLVMInt32Type(), LLVMPointerType(LLVMPointerType(LLVMInt8Type(), 0), 0) };
	LLVMValueRef entry_point = LLVMAddFunction(root_module, "mainCRTStartup", LLVMFunctionType(LLVMInt32Type(), entry_point_arg_types, 2, false));
	LLVMPositionBuilderAtEnd(g->llvm_builder, LLVMAppendBasicBlock(entry_point, "entry"));
	LLVMBuildRet(g->llvm_builder, LLVMBuildCall(g->llvm_builder, LLVMAddFunction(root_module, "__main_init", LLVMFunctionType(LLVMInt32Type(), NULL, 0, false)), NULL, 0, ""));

	//output_module(root_module, "__root.o");
	for (int i = 0; i < g->module_vec.size; i++) {
		/*
		int len = strlen(g->module_name_vec.buffer[i]);
		char* objfile_name = malloc(len + 3);
		memcpy(objfile_name, g->module_name_vec.buffer[i], len);
		objfile_name[len] = '.';
		objfile_name[len + 1] = 'o';
		objfile_name[len + 2] = 0;
		output_module(g->module_vec.buffer[i], objfile_name);
		free(objfile_name);
		*/
		LLVMLinkModules2(root_module, g->module_vec.buffer[i]);
	}
	output_module(root_module, "out.o");

	DYNAMIC_STRING link_cmd = string_new(16);
	//string_push_s(&link_cmd, "lld-link __root.o ");
	string_push_s(&link_cmd, "lld-link out.o ");
	/*
	for (int i = 0; i < g->module_vec.size; i++) {
		string_push_s(&link_cmd, g->module_name_vec.buffer[i]);
		string_push(&link_cmd, '.');
		string_push(&link_cmd, 'o');
		string_push(&link_cmd, ' ');
	}
	*/
	string_push_s(&link_cmd, "msvcrt.lib /subsystem:console /out:a.exe");
	system(link_cmd.buffer);
	//system("lld-link out.o msvcrt.lib /subsystem:console /out:a.exe");
	string_delete(&link_cmd);

	printf("### TEST ###\n");
	system("a.exe");
}