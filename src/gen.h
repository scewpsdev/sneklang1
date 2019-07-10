#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <llvm-c/Core.h>

#include "ast.h"
#include "utils.h"

typedef struct SCOPE_t {
	struct SCOPE_t* parent;
} SCOPE;

typedef struct CODEGEN_t {
	LLVMContextRef llvm_context;
	LLVMBuilderRef llvm_builder;
	MODULE_VEC module_vec;

	AST* ast;
	LLVMModuleRef llvm_module;
	SCOPE* current_scope;

	STRING_VEC globals_k;
	VALUE_VEC globals_v;
} CODEGEN;

CODEGEN gen_new();
void gen_delete(CODEGEN* codegen);

void gen_create_module(CODEGEN* g, AST* ast);
void gen_link(CODEGEN* g);