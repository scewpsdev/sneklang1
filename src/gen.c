#include "gen.h"

#include <string.h>

#include <llvm-c/TargetMachine.h>
#include <llvm-c/Linker.h>

CODEGEN gen_new() {
	CODEGEN g;

	g.llvm_context = LLVMContextCreate();
	g.llvm_builder = LLVMCreateBuilderInContext(g.llvm_context);
	g.module_vec = mdvec_new(2);

	g.ast = NULL;
	g.llvm_module = NULL;
	g.current_scope = NULL;

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
}

LLVMValueRef find_local_value(SCOPE* s, char* name) {
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

LLVMTypeRef get_llvm_type(TYPE* type) {
	if (strlen(type->name) >= 2 && type->name[0] == 'i') {
		int bitsize = strtol(type->name + 1, NULL, 10);
		return type->ptr ? LLVMPointerType(LLVMIntType(bitsize), 0) : LLVMIntType(bitsize);
	}
	return NULL;
}

LLVMValueRef gen_expr(CODEGEN*, EXPRESSION*);

LLVMValueRef gen_int_literal(CODEGEN* g, INT* i) {
	return LLVMConstInt(LLVMInt32Type(), i->value, true);
}

LLVMValueRef gen_char_literal(CODEGEN* g, CHAR* ch) {
	return LLVMConstInt(LLVMInt8Type(), ch->value, true);
}

LLVMValueRef gen_string_literal(CODEGEN* g, STRING* str) {
	return NULL;
}

LLVMValueRef gen_identifier(CODEGEN* g, IDENTIFIER* i) {
	return find_named_value(g, g->current_scope, i->name);
}

LLVMValueRef gen_func_call(CODEGEN* g, FUNC_CALL* func_call) {
	LLVMValueRef callee = gen_expr(g, func_call->callee);
	LLVMValueRef* args = malloc(func_call->num_args * sizeof(LLVMValueRef));
	for (int i = 0; i < func_call->num_args; i++) {
		args[i] = gen_expr(g, &func_call->args[i]);
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

LLVMValueRef gen_expr(CODEGEN* g, EXPRESSION* expr) {
	switch (expr->type) {
	case EXPR_TYPE_INT_LITERAL: return gen_int_literal(g, &expr->int_literal);
	case EXPR_TYPE_CHAR_LITERAL: return gen_char_literal(g, &expr->char_literal);
	case EXPR_TYPE_STRING_LITERAL: return gen_string_literal(g, &expr->string_literal);
	case EXPR_TYPE_IDENTIFIER: return gen_identifier(g, &expr->identifier);
	case EXPR_TYPE_FUNC_CALL: return gen_func_call(g, &expr->func_call);
	case EXPR_TYPE_FUNC_DECL: return gen_func_decl(g, &expr->func_decl);
	default: return LLVMConstInt(LLVMInt32Type(), 0, true);
	}
}

LLVMValueRef gen_ast(CODEGEN* g, AST* ast) {
	SCOPE* parent = g->current_scope;
	g->current_scope = malloc(sizeof(SCOPE));
	*g->current_scope = (SCOPE){ parent };

	for (int i = 0; i < ast->num_expressions; i++) {
		gen_expr(g, &ast->expressions[i]);
	}

	free(g->current_scope);
	g->current_scope = parent;

	return LLVMConstInt(LLVMInt32Type(), 0, true);
}

void gen_toplevel(CODEGEN* g, AST* ast) {
	LLVMTypeRef func_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, false);
	LLVMValueRef function = LLVMAddFunction(g->llvm_module, "__main_init", func_type);
	LLVMSetLinkage(function, LLVMExternalLinkage);

	LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(function, "entry");
	LLVMPositionBuilderAtEnd(g->llvm_builder, entry_block);

	gen_ast(g, ast);

	LLVMBuildRet(g->llvm_builder, LLVMConstInt(LLVMInt32Type(), 0, true));
}

void gen_create_module(CODEGEN* g, AST* ast) {
	g->llvm_module = LLVMModuleCreateWithNameInContext("main", g->llvm_context);
	g->ast = ast;

	gen_toplevel(g, ast);

	printf(LLVMPrintModuleToString(g->llvm_module));
	printf("-----\n\n");
	LLVMPassManagerRef pass_manager = LLVMCreatePassManager();
	LLVMRunPassManager(pass_manager, g->llvm_module);

	mdvec_push(&g->module_vec, g->llvm_module);
}

void gen_link(CODEGEN* g) {
	LLVMModuleRef output_module = LLVMModuleCreateWithNameInContext("__root_module", g->llvm_context);
	LLVMTypeRef entry_point_arg_types[] = { LLVMInt32Type(), LLVMPointerType(LLVMPointerType(LLVMInt8Type(), 0), 0) };
	LLVMValueRef entry_point = LLVMAddFunction(output_module, "mainCRTStartup", LLVMFunctionType(LLVMInt32Type(), entry_point_arg_types, 2, false));
	LLVMPositionBuilderAtEnd(g->llvm_builder, LLVMAppendBasicBlock(entry_point, "entry"));
	LLVMBuildRet(g->llvm_builder, LLVMBuildCall(g->llvm_builder, LLVMAddFunction(output_module, "__main_init", LLVMFunctionType(LLVMInt32Type(), NULL, 0, false)), NULL, 0, ""));
	printf(LLVMPrintModuleToString(output_module));

	for (int i = 0; i < g->module_vec.size; i++) {
		LLVMLinkModules2(output_module, g->module_vec.buffer[i]);
	}

	LLVMSetTarget(output_module, LLVM_DEFAULT_TARGET_TRIPLE);

	LLVMTargetRef target;
	char* error = NULL;
	const char* target_triple = LLVMGetTarget(output_module);
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

	char* filename = "main.o";
	LLVMCodeGenFileType filetype = LLVMObjectFile;
	error = NULL;
	LLVMTargetMachineEmitToFile(target_machine, output_module, filename, filetype, &error);
	if (error) {
		printf(error);
		return;
	}
	system("lld-link main.o msvcrt.lib /subsystem:console /out:a.exe");

	printf("### TEST ###\n");
	system("a.exe");
}