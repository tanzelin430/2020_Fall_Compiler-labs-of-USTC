/*
 * 声明：本代码为 2020 秋 中国科大编译原理（李诚）课程实验参考实现。
 * 请不要以任何方式，将本代码上传到可以公开访问的站点或仓库
*/

#include "cminusf_builder.hpp"

#define CONST_FP(num) \
	ConstantFP::get((float)num, module.get())
#define CONST_INT(num) \
	ConstantInt::get(num, module.get())

// You can define global variables here
// to store state

// store temporary value
Value *tmp_val = nullptr;
// whether require lvalue
bool require_lvalue = false;
// function that is being built
Function *cur_fun = nullptr;
// detect scope pre-enter (for elegance only)
bool pre_enter_scope = false;

// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *INT32PTR_T;
Type *FLOAT_T;
Type *FLOATPTR_T;

bool promote(IRBuilder *builder, Value **l_val_p, Value **r_val_p)
{
	bool is_int;
	auto &l_val = *l_val_p;
	auto &r_val = *r_val_p;
	if (l_val->get_type() == r_val->get_type())
	{
		is_int = l_val->get_type()->is_integer_type();
	}
	else
	{
		is_int = false;
		if (l_val->get_type()->is_integer_type())
			l_val = builder->create_sitofp(l_val, FLOAT_T);
		else
			r_val = builder->create_sitofp(r_val, FLOAT_T);
	}
	return is_int;
}

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node)
{
	VOID_T = Type::get_void_type(module.get());
	INT1_T = Type::get_int1_type(module.get());
	INT32_T = Type::get_int32_type(module.get());
	INT32PTR_T = Type::get_int32_ptr_type(module.get());
	FLOAT_T = Type::get_float_type(module.get());
	FLOATPTR_T = Type::get_float_ptr_type(module.get());

	for (auto decl : node.declarations)
	{
		decl->accept(*this);
	}
}

void CminusfBuilder::visit(ASTNum &node)
{
	if (node.type == TYPE_INT)
		tmp_val = CONST_INT(node.i_val);
	else
		tmp_val = CONST_FP(node.f_val);
}

void CminusfBuilder::visit(ASTVarDeclaration &node)
{
	Type *var_type;
	if (node.type == TYPE_INT)
		var_type = Type::get_int32_type(module.get());
	else
		var_type = Type::get_float_type(module.get());
	if (node.num == nullptr)
	{
		if (scope.in_global())
		{
			auto initializer = ConstantZero::get(var_type, module.get());
			auto var = GlobalVariable::create(
				node.id,
				module.get(),
				var_type,
				false,
				initializer);
			scope.push(node.id, var);
		}
		else
		{
			auto var = builder->create_alloca(var_type);
			scope.push(node.id, var);
		}
	}
	else
	{
		auto *array_type = ArrayType::get(var_type, node.num->i_val);
		if (scope.in_global())
		{
			auto initializer = ConstantZero::get(array_type, module.get());
			auto var = GlobalVariable::create(
				node.id,
				module.get(),
				array_type,
				false,
				initializer);
			scope.push(node.id, var);
		}
		else
		{
			auto var = builder->create_alloca(array_type);
			scope.push(node.id, var);
		}
	}
}

void CminusfBuilder::visit(ASTFunDeclaration &node)
{
	FunctionType *fun_type;
	Type *ret_type;
	std::vector<Type *> param_types;
	if (node.type == TYPE_INT)
		ret_type = INT32_T;
	else if (node.type == TYPE_FLOAT)
		ret_type = FLOAT_T;
	else
		ret_type = VOID_T;

	for (auto &param : node.params)
	{
		if (param->type == TYPE_INT)
		{
			if (param->isarray)
			{
				param_types.push_back(INT32PTR_T);
			}
			else
			{
				param_types.push_back(INT32_T);
			}
		}
		else
		{
			if (param->isarray)
			{
				param_types.push_back(FLOATPTR_T);
			}
			else
			{
				param_types.push_back(FLOAT_T);
			}
		}
	}

	fun_type = FunctionType::get(ret_type, param_types);
	auto fun =
		Function::create(
			fun_type,
			node.id,
			module.get());
	scope.push(node.id, fun);
	cur_fun = fun;
	auto funBB = BasicBlock::create(module.get(), "entry", fun);
	builder->set_insert_point(funBB);
	scope.enter();
	pre_enter_scope = true;
	std::vector<Value *> args;
	for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++)
	{
		args.push_back(*arg);
	}
	for (int i = 0; i < node.params.size(); ++i)
	{
		if (node.params[i]->isarray)
		{
			Value *array_alloc;
			if (node.params[i]->type == TYPE_INT)
				array_alloc = builder->create_alloca(INT32PTR_T);
			else
				array_alloc = builder->create_alloca(FLOATPTR_T);
			builder->create_store(args[i], array_alloc);
			scope.push(node.params[i]->id, array_alloc);
		}
		else
		{
			Value *alloc;
			if (node.params[i]->type == TYPE_INT)
				alloc = builder->create_alloca(INT32_T);
			else
				alloc = builder->create_alloca(FLOAT_T);
			builder->create_store(args[i], alloc);
			scope.push(node.params[i]->id, alloc);
		}
	}
	node.compound_stmt->accept(*this);
	if (builder->get_insert_block()->get_terminator() == nullptr)
	{
		if (cur_fun->get_return_type()->is_void_type())
			builder->create_void_ret();
		else if (cur_fun->get_return_type()->is_float_type())
			builder->create_ret(CONST_FP(0.));
		else
			builder->create_ret(CONST_INT(0));
	}
	scope.exit();
}

void CminusfBuilder::visit(ASTParam &node) {}

void CminusfBuilder::visit(ASTCompoundStmt &node)
{
	bool need_exit_scope = !pre_enter_scope;
	if (pre_enter_scope)
	{
		pre_enter_scope = false;
	}
	else
	{
		scope.enter();
	}

	for (auto &decl : node.local_declarations)
	{
		decl->accept(*this);
	}

	for (auto &stmt : node.statement_list)
	{
		stmt->accept(*this);
		if (builder->get_insert_block()->get_terminator() != nullptr)
			break;
	}

	if (need_exit_scope)
	{
		scope.exit();
	}
}

void CminusfBuilder::visit(ASTExpressionStmt &node)
{
	if (node.expression != nullptr)
		node.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &node)
{
	node.expression->accept(*this);
	auto ret_val = tmp_val;
	auto trueBB = BasicBlock::create(module.get(), "", cur_fun);
	auto falseBB = BasicBlock::create(module.get(), "", cur_fun);
	auto contBB = BasicBlock::create(module.get(), "", cur_fun);
	Value *cond_val;
	if (ret_val->get_type()->is_integer_type())
		cond_val = builder->create_icmp_ne(ret_val, CONST_INT(0));
	else
		cond_val = builder->create_fcmp_ne(ret_val, CONST_FP(0.));

	if (node.else_statement == nullptr)
	{
		builder->create_cond_br(cond_val, trueBB, contBB);
	}
	else
	{
		builder->create_cond_br(cond_val, trueBB, falseBB);
	}
	builder->set_insert_point(trueBB);
	node.if_statement->accept(*this);

	if (builder->get_insert_block()->get_terminator() == nullptr)
		builder->create_br(contBB);

	if (node.else_statement == nullptr)
	{
		falseBB->erase_from_parent();
	}
	else
	{
		builder->set_insert_point(falseBB);
		node.else_statement->accept(*this);
		if (builder->get_insert_block()->get_terminator() == nullptr)
			builder->create_br(contBB);
	}

	builder->set_insert_point(contBB);
}

void CminusfBuilder::visit(ASTIterationStmt &node)
{
	auto exprBB = BasicBlock::create(module.get(), "", cur_fun);
	if (builder->get_insert_block()->get_terminator() == nullptr)
		builder->create_br(exprBB);
	builder->set_insert_point(exprBB);
	node.expression->accept(*this);
	auto ret_val = tmp_val;
	auto trueBB = BasicBlock::create(module.get(), "", cur_fun);
	auto contBB = BasicBlock::create(module.get(), "", cur_fun);
	Value *cond_val;
	if (ret_val->get_type()->is_integer_type())
		cond_val = builder->create_icmp_ne(ret_val, CONST_INT(0));
	else
		cond_val = builder->create_fcmp_ne(ret_val, CONST_FP(0.));

	builder->create_cond_br(cond_val, trueBB, contBB);
	builder->set_insert_point(trueBB);
	node.statement->accept(*this);
	if (builder->get_insert_block()->get_terminator() == nullptr)
		builder->create_br(exprBB);
	builder->set_insert_point(contBB);
}

void CminusfBuilder::visit(ASTReturnStmt &node)
{
	if (node.expression == nullptr)
	{
		builder->create_void_ret();
	}
	else
	{
		auto fun_ret_type = cur_fun->get_function_type()->get_return_type();
		node.expression->accept(*this);
		if (fun_ret_type != tmp_val->get_type())
		{
			if (fun_ret_type->is_integer_type())
				tmp_val = builder->create_fptosi(tmp_val, INT32_T);
			else
				tmp_val = builder->create_sitofp(tmp_val, FLOAT_T);
		}

		builder->create_ret(tmp_val);
	}
}

void CminusfBuilder::visit(ASTVar &node)
{
	auto var = scope.find(node.id);
	assert(var != nullptr);
	auto is_int = var->get_type()->get_pointer_element_type()->is_integer_type();
	auto is_float = var->get_type()->get_pointer_element_type()->is_float_type();
	auto is_ptr = var->get_type()->get_pointer_element_type()->is_pointer_type();
	bool should_return_lvalue = require_lvalue;
	require_lvalue = false;
	if (node.expression == nullptr)
	{
		if (should_return_lvalue)
		{
			tmp_val = var;
			require_lvalue = false;
		}
		else
		{
			if (is_int || is_float || is_ptr)
			{
				tmp_val = builder->create_load(var);
			}
			else
			{
				tmp_val = builder->create_gep(var, {CONST_INT(0), CONST_INT(0)});
			}
		}
	}
	else
	{
		node.expression->accept(*this);
		auto val = tmp_val;
		Value *is_neg;
		auto exceptBB = BasicBlock::create(module.get(), "", cur_fun);
		auto contBB = BasicBlock::create(module.get(), "", cur_fun);
		if (val->get_type()->is_float_type())
			val = builder->create_fptosi(val, INT32_T);

		is_neg = builder->create_icmp_lt(val, CONST_INT(0));

		builder->create_cond_br(is_neg, exceptBB, contBB);
		builder->set_insert_point(exceptBB);
		auto neg_idx_except_fun = scope.find("neg_idx_except");
		builder->create_call(static_cast<Function *>(neg_idx_except_fun), {});
		if (cur_fun->get_return_type()->is_void_type())
			builder->create_void_ret();
		else if (cur_fun->get_return_type()->is_float_type())
			builder->create_ret(CONST_FP(0.));
		else
			builder->create_ret(CONST_INT(0));

		builder->set_insert_point(contBB);
		Value *tmp_ptr;
		if (is_int || is_float)
			tmp_ptr = builder->create_gep(var, {val});
		else if (is_ptr)
		{
			auto array_load = builder->create_load(var);
			tmp_ptr = builder->create_gep(array_load, {val});
		}
		else
			tmp_ptr = builder->create_gep(var, {CONST_INT(0), val});
		if (should_return_lvalue)
		{
			tmp_val = tmp_ptr;
			require_lvalue = false;
		}
		else
		{
			tmp_val = builder->create_load(tmp_ptr);
		}
	}
}

void CminusfBuilder::visit(ASTAssignExpression &node)
{
	node.expression->accept(*this);
	auto expr_result = tmp_val;
	require_lvalue = true;
	node.var->accept(*this);
	auto var_addr = tmp_val;
	if (var_addr->get_type()->get_pointer_element_type() != expr_result->get_type())
	{
		if (expr_result->get_type() == INT32_T)
			expr_result = builder->create_sitofp(expr_result, FLOAT_T);
		else
			expr_result = builder->create_fptosi(expr_result, INT32_T);
	}
	builder->create_store(expr_result, var_addr);
	tmp_val = expr_result;
}

void CminusfBuilder::visit(ASTSimpleExpression &node)
{
	if (node.additive_expression_r == nullptr)
	{
		node.additive_expression_l->accept(*this);
	}
	else
	{
		node.additive_expression_l->accept(*this);
		auto l_val = tmp_val;
		node.additive_expression_r->accept(*this);
		auto r_val = tmp_val;
		bool is_int = promote(builder, &l_val, &r_val);
		Value *cmp;
		switch (node.op)
		{
		case OP_LT:
			if (is_int)
				cmp = builder->create_icmp_lt(l_val, r_val);
			else
				cmp = builder->create_fcmp_lt(l_val, r_val);
			break;
		case OP_LE:
			if (is_int)
				cmp = builder->create_icmp_le(l_val, r_val);
			else
				cmp = builder->create_fcmp_le(l_val, r_val);
			break;
		case OP_GE:
			if (is_int)
				cmp = builder->create_icmp_ge(l_val, r_val);
			else
				cmp = builder->create_fcmp_ge(l_val, r_val);
			break;
		case OP_GT:
			if (is_int)
				cmp = builder->create_icmp_gt(l_val, r_val);
			else
				cmp = builder->create_fcmp_gt(l_val, r_val);
			break;
		case OP_EQ:
			if (is_int)
				cmp = builder->create_icmp_eq(l_val, r_val);
			else
				cmp = builder->create_fcmp_eq(l_val, r_val);
			break;
		case OP_NEQ:
			if (is_int)
				cmp = builder->create_icmp_ne(l_val, r_val);
			else
				cmp = builder->create_fcmp_ne(l_val, r_val);
			break;
		}

		tmp_val = builder->create_zext(cmp, INT32_T);
	}
}

void CminusfBuilder::visit(ASTAdditiveExpression &node)
{
	if (node.additive_expression == nullptr)
	{
		node.term->accept(*this);
	}
	else
	{
		node.additive_expression->accept(*this);
		auto l_val = tmp_val;
		node.term->accept(*this);
		auto r_val = tmp_val;
		bool is_int = promote(builder, &l_val, &r_val);
		switch (node.op)
		{
		case OP_PLUS:
			if (is_int)
				tmp_val = builder->create_iadd(l_val, r_val);
			else
				tmp_val = builder->create_fadd(l_val, r_val);
			break;
		case OP_MINUS:
			if (is_int)
				tmp_val = builder->create_isub(l_val, r_val);
			else
				tmp_val = builder->create_fsub(l_val, r_val);
			break;
		}
	}
}

void CminusfBuilder::visit(ASTTerm &node)
{
	if (node.term == nullptr)
	{
		node.factor->accept(*this);
	}
	else
	{
		node.term->accept(*this);
		auto l_val = tmp_val;
		node.factor->accept(*this);
		auto r_val = tmp_val;
		bool is_int = promote(builder, &l_val, &r_val);
		switch (node.op)
		{
		case OP_MUL:
			if (is_int)
				tmp_val = builder->create_imul(l_val, r_val);
			else
				tmp_val = builder->create_fmul(l_val, r_val);
			break;
		case OP_DIV:
			if (is_int)
				tmp_val = builder->create_isdiv(l_val, r_val);
			else
				tmp_val = builder->create_fdiv(l_val, r_val);
			break;
		}
	}
}

void CminusfBuilder::visit(ASTCall &node)
{
	auto fun = static_cast<Function *>(scope.find(node.id));
	std::vector<Value *> args;
	auto param_type = fun->get_function_type()->param_begin();
	for (auto &arg : node.args)
	{
		arg->accept(*this);
		if (!tmp_val->get_type()->is_pointer_type() &&
			*param_type != tmp_val->get_type())
		{
			if (tmp_val->get_type()->is_integer_type())
				tmp_val = builder->create_sitofp(tmp_val, FLOAT_T);
			else
				tmp_val = builder->create_fptosi(tmp_val, INT32_T);
		}
		args.push_back(tmp_val);
		param_type++;
	}

	tmp_val = builder->create_call(static_cast<Function *>(fun), args);
}

// #include "../../include/cminusf_builder.hpp"

// // use these macros to get constant value
// #define CONST_FP(num) \
// 	ConstantFP::get((float)num, module.get())
// #define CONST_ZERO(type) \
// 	ConstantZero::get(var_type, module.get())
// #define CONST_INT(num) \
// 	ConstantInt::get(num, module.get())

// Value *ret; // store the expression results
// // Function *curr_func;
// // You can define global variables here
// // to store state
// int isAfterReturn = 0;

// /*
//  * use CMinusfBuilder::Scope to construct scopes
//  * scope.enter: enter a new scope
//  * scope.exit: exit current scope
//  * scope.push: add a new binding to current scope
//  * scope.find: find and return the value bound to the name
//  */

// void CminusfBuilder::visit(ASTProgram &node)
// {
// 	// program -> declaration-list by Rui Lv
// 	if (!isAfterReturn)
// 	{
// 		for (auto n : node.declarations)
// 		{					  // n 获取 declaration-list
// 			n->accept(*this); // visit all declaration-list
// 		}
// 	}
// }

// void CminusfBuilder::visit(ASTNum &node)
// {
// 	// Num by Rui Lv
// 	if (node.type == TYPE_INT)
// 	{
// 		ret = CONST_INT(node.i_val);
// 	}
// 	else if (node.type == TYPE_FLOAT)
// 	{
// 		ret = CONST_FP(node.f_val);
// 	}
// }

// void CminusfBuilder::visit(ASTVarDeclaration &node)
// {
// 	// var-declaration -> type-specifier ID; | type-specifier ID [NUM] by Rui Lv
// 	if (!isAfterReturn)
// 	{
// 		Type *Int32Type = Type::get_int32_type(module.get()); // 获取变量类型
// 		Type *FloatType = Type::get_float_type(module.get());

// 		Type *mainType = node.type == TYPE_INT ? Int32Type : FloatType;

// 		if (!scope.in_global())
// 		{
// 			// not in global
// 			if (node.num)
// 			{
// 				// local array
// 				// create memory for array
// 				if (node.num->type != TYPE_INT || node.num->i_val <= 0)
// 				{
// 					// 数组声明时，下标只能是整数，且必须大于零，否则调用 neg_idx_except 退出
// 					// DEBUG：声明时，数组下标只能是立即数，不会是 expression，所以可以直接通过 >= 的方式检测，而不用设置跳转。
// 					auto neg_idx_except_fun = scope.find("neg_idx_except"); // get neg_idx_except
// 					builder->create_call(neg_idx_except_fun, {});
// 				}
// 				auto *arrayType = ArrayType::get(mainType, node.num->i_val);
// 				auto aAlloca = builder->create_alloca(arrayType);
// 				// push the id into scope stack
// 				scope.push(node.id, aAlloca);
// 			}
// 			else
// 			{
// 				// local id
// 				auto idAlloca = builder->create_alloca(mainType);
// 				// push the id into scope stack
// 				scope.push(node.id, idAlloca);
// 			}
// 		}
// 		else
// 		{
// 			// global declaration
// 			if (node.num)
// 			{
// 				// global array
// 				if (node.num->type != TYPE_INT || node.num->i_val <= 0)
// 				{
// 					// 数组声明时，下标只能是整数，且必须大于零，否则调用 neg_idx_except 退出
// 					auto neg_idx_except_fun = scope.find("neg_idx_except"); // get neg_idx_except
// 					builder->create_call(neg_idx_except_fun, {});
// 				}
// 				auto *arrayType = ArrayType::get(mainType, node.num->i_val);
// 				auto initializer = ConstantZero::get(mainType, module.get());
// 				// initialize the array to zero
// 				auto gaAlloca = GlobalVariable::create(node.id, module.get(), arrayType, false, initializer);
// 				// push the array id into scope stack
// 				scope.push(node.id, gaAlloca);
// 			}
// 			else
// 			{
// 				// global id
// 				auto initializer = ConstantZero::get(mainType, module.get());
// 				auto gidAlloca = GlobalVariable::create(node.id, module.get(), mainType, false, initializer);
// 				// push into scope
// 				scope.push(node.id, gidAlloca);
// 			}
// 		}
// 	}
// }

// void CminusfBuilder::visit(ASTFunDeclaration &node)
// {
// 	// fun-declaration -> type-specifier ID (prams) compound-stmt
// 	// we should push the parameter identifiers into the function scope. by Rui Lv
// 	if (!isAfterReturn)
// 	{
// 		scope.enter();										  // enter a new scope for this function declaration
// 		Type *Int32Type = Type::get_int32_type(module.get()); // 获取变量类型
// 		Type *FloatType = Type::get_float_type(module.get());
// 		Type *VoidType = Type::get_void_type(module.get());
// 		auto Int32PtrType = Type::get_int32_ptr_type(module.get()); // 单个参数的类型,指针
// 		auto FloatPtrType = Type::get_float_ptr_type(module.get());

// 		auto funType = node.type == TYPE_VOID ? VoidType : (node.type == TYPE_INT ? Int32Type : FloatType); // 获取函数的返回值类型 void/int/float

// 		std::vector<Type *> args; // 获取函数中的形参类型
// 		if (node.params.size() > 0)
// 		{
// 			// the params is not void,then get them by iterator in node.params
// 			for (auto arg : node.params)
// 			{
// 				if (arg->isarray)
// 				{
// 					// this arg is an arrayType param
// 					auto argType = arg->type == TYPE_INT ? Int32PtrType : FloatPtrType;
// 					args.push_back(argType);
// 				}
// 				else if (arg->type == TYPE_INT)
// 				{
// 					args.push_back(Int32Type);
// 				}
// 				else
// 				{
// 					args.push_back(FloatType);
// 				}
// 			}
// 		}

// 		auto funcTy = FunctionType::get(funType, args);				   // 通过返回值类型与参数类型列表得到函数类型
// 		auto funcID = Function::create(funcTy, node.id, module.get()); // 由函数类型得到函数

// 		// curr_func = funcID; //全局变量设置，保存当前函数作用域，否则，“获取当前bb所在函数应该builder->get_insert_block()->getparent()” -- by TA chen qingyuan

// 		scope.exit(); // exit a function scope

// 		scope.push(node.id, funcID); // push the function into parent scope [the scope that can call this function]

// 		scope.enter(); // enter into the function scope
// 		auto funBB = BasicBlock::create(module.get(), "entry", funcID);
// 		builder->set_insert_point(funBB); // the begin of funBB

// 		for (auto param : node.params)
// 		{
// 			// the params can be a declaration-list, each of them can be a Tree Node, so call accept() -- by ast.cpp
// 			param.get()->accept(*this);
// 		}

// 		std::vector<Value *> args_value; // get the params value of the function

// 		for (auto arg = funcID->arg_begin(); arg != funcID->arg_end(); arg++)
// 		{
// 			args_value.push_back(*arg); // * 号运算符是从迭代器中取出迭代器当前指向的元素
// 		}

// 		if (node.params.size() > 0 && args_value.size() > 0)
// 		{ // alloca memory for params and store them
// 			int i = 0;
// 			for (auto arg : node.params)
// 			{
// 				auto pAlloc = scope.find(arg->id); // get param's alloc
// 				if (pAlloc == nullptr)
// 				{
// 					exit(0); // the param can't be found
// 				}
// 				else
// 				{
// 					builder->create_store(args_value[i++], pAlloc);
// 				}
// 			}
// 		}

// 		// compound-stmt
// 		if (node.compound_stmt != nullptr)
// 		{
// 			node.compound_stmt->accept(*this);
// 		}
// 		if (builder->get_insert_block()->get_terminator() == nullptr)
// 		{
// 			// function end,set the initialized return value
// 			if (funType == VoidType)
// 			{
// 				builder->create_void_ret(); // void func()
// 			}
// 			else if (funType == Int32Type)
// 			{
// 				builder->create_ret(CONST_INT(0));
// 			}
// 			else if (funType == FloatType)
// 			{
// 				builder->create_ret(CONST_FP(0.));
// 			}
// 		}
// 		// 此处可能需要统一进行错误函数处理（使用全局变量），但目前我们是将错误拆分到每个函数内部识别 & 处理。

// 		scope.exit(); // exit the function scope

// 		isAfterReturn = 0;
// 	}
// }

// void CminusfBuilder::visit(ASTParam &node)
// {
// 	// param ->type-specifier ID | type-specifier ID [] -- by Rui Lv
// 	// this is a function's param declaration, so the params cannot be in global scope
// 	// similar to var-declaration
// 	Type *Int32Type = Type::get_int32_type(module.get()); // 获取变量类型
// 	Type *FloatType = Type::get_float_type(module.get());
// 	Type *VoidType = Type::get_void_type(module.get());
// 	auto Int32PtrType = Type::get_int32_ptr_type(module.get()); // 单个参数的类型,指针
// 	auto FloatPtrType = Type::get_float_ptr_type(module.get());

// 	Value *pAlloc; // alloca some memory for this param
// 	if (node.type == TYPE_VOID)
// 	{
// 		// the param is void
// 		return;
// 	}
// 	else if (node.isarray)
// 	{
// 		// this param is an array
// 		// 此处不需要考虑数组下标，因为根据 cminusf 语法，这里的数组不会有下标
// 		auto arrayType = node.type == TYPE_INT ? Int32PtrType : FloatPtrType;
// 		pAlloc = builder->create_alloca(arrayType);
// 		scope.push(node.id, pAlloc);
// 	}
// 	else
// 	{
// 		// the param is just an id
// 		auto idType = node.type == TYPE_INT ? Int32Type : FloatType;
// 		pAlloc = builder->create_alloca(idType);
// 		scope.push(node.id, pAlloc);
// 	}
// }

// void CminusfBuilder::visit(ASTCompoundStmt &node)
// {
// 	// compound-stmt -> { local-declarations statement-list } -- by Rui Lv
// 	// {} 需要进入一个新域，来维护函数体内部变量的作用域
// 	if (!isAfterReturn)
// 	{
// 		scope.enter(); // enter function body
// 		if (node.local_declarations.size() > 0)
// 		{
// 			for (auto locDec : node.local_declarations)
// 			{
// 				locDec->accept(*this); // 对非终结符，继续向下遍历即可
// 			}
// 		}
// 		for (auto staList : node.statement_list)
// 		{
// 			staList->accept(*this);
// 		}
// 		scope.exit(); // exit function body
// 	}
// }

// void CminusfBuilder::visit(ASTExpressionStmt &node)
// {
// 	if (!isAfterReturn)
// 	{
// 		if (node.expression != nullptr)
// 			node.expression->accept(*this);
// 	}
// }

// void CminusfBuilder::visit(ASTSelectionStmt &node)
// {
// 	/*selection-stmt → ​if ( expression ) statement∣ if ( expression ) statement else statement​*/
// 	//expand expression firstly,because there must be at least one expression
// 	if (!isAfterReturn)
// 	{
// 		int localFlag = 0;
// 		node.expression->accept(*this);
// 		//define some types that will be used later
// 		Type *typeint = Type::get_int32_type(module.get());
// 		Type *typeintptr = Type::get_int32_ptr_type(module.get());
// 		Type *typefloatptr = Type::get_float_ptr_type(module.get());
// 		Type *typefloat = Type::get_float_type(module.get());
// 		Type *typeint1 = Type::get_int1_type(module.get());
// 		//if ret.type == ptr,then we load ret,because we need to use it
// 		if (ret->get_type() == typeintptr || ret->get_type() == typefloatptr)
// 		{
// 			ret = builder->create_load(ret);
// 		}
// 		//now we judge if expression is true
// 		if (ret->get_type() == typeint1)
// 		{
// 			ret = builder->create_zext(ret, typeint);
// 		}
// 		if (ret->get_type() == typeint)
// 		{
// 			ret = builder->create_icmp_ne(ret, CONST_INT(0));
// 		}
// 		if (ret->get_type() == typefloat)
// 		{
// 			ret = builder->create_fcmp_ne(ret, CONST_FP(0));
// 		}
// 		//now we see if there is an else statement
// 		if (node.else_statement != nullptr)
// 		{
// 			//if there is an else
// 			//make branches
// 			auto trueBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());	// true分支
// 			auto falseBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent()); // false分支
// 			auto retBB = BasicBlock::create(
// 				module.get(), "", builder->get_insert_block()->get_parent()); // return分支,提前create,以便true分支可以br
// 			auto br = builder->create_cond_br(ret, trueBB, falseBB);		  // 条件BR
// 			//trueBB
// 			builder->set_insert_point(trueBB);
// 			node.if_statement->accept(*this);

// 			if (isAfterReturn)
// 			{
// 				localFlag = 1;
// 				isAfterReturn = 0;
// 			}

// 			//true,enter if statement
// 			//empty instrlist,not inserted yet
// 			if (!localFlag)
// 			{ // not returned in if-statement
// 				// if (builder->get_insert_block()->get_terminator() == nullptr)
// 				//get_terminator() 返回BB块的终止指令(ret|br)若BB块最后一条指令不是终止指令返回null
// 				// {
// 				//not returned in if-statement
// 				builder->create_br(retBB);
// 				// }
// 			}

// 			//falsebb
// 			//enter else statement
// 			builder->set_insert_point(falseBB);
// 			int localFlag_2 = 0;
// 			node.else_statement->accept(*this);

// 			if (isAfterReturn)
// 			{
// 				localFlag_2 = 1;
// 				if (localFlag)
// 				{
// 					isAfterReturn = 1; // if & else 都有 return ，则 if-else 后的代码块没有必要执行；
// 				}
// 				else
// 				{
// 					isAfterReturn = 0;
// 				}
// 			}

// 			if (!localFlag_2) // && builder->get_insert_block()->get_terminator() == nullptr --- not end in the block
// 			{
// 				/* code */
// 				builder->create_br(retBB);
// 			}

// 			/*if (is_inserted)
//         {
//             builder->set_insert_point(retBB);
//         }*/
// 			builder->set_insert_point(retBB);
// 		}
// 		else //no else statement,only need to see if it is true
// 		{
// 			auto trueBB = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent()); // true分支
// 			auto retBB = BasicBlock::create(
// 				module.get(), "", builder->get_insert_block()->get_parent()); // return分支,提前create,以便true分支可以br
// 			builder->create_cond_br(ret, trueBB, retBB);
// 			//tureBB
// 			builder->set_insert_point(trueBB);
// 			node.if_statement->accept(*this);

// 			if (!isAfterReturn)
// 			{ // builder->get_insert_block()->get_terminator() == nullptr
// 				//not returned,go to out
// 				builder->create_br(retBB);
// 			}
// 			builder->set_insert_point(retBB);
// 		}
// 	}
// }
// void CminusfBuilder::visit(ASTIterationStmt &node) // iteration-stmt→while ( expression ) statement
// {
// 	if (!isAfterReturn)
// 	{
// 		Type *typeint = Type::get_int32_type(module.get());
// 		Type *typeintptr = Type::get_int32_ptr_type(module.get());
// 		Type *typefloatptr = Type::get_float_ptr_type(module.get());
// 		Type *typefloat = Type::get_float_type(module.get());
// 		Type *typeint1 = Type::get_int1_type(module.get());

// 		auto loop_enter = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
// 		auto loop = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
// 		auto loop_out = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
// 		//loop_enter
// 		builder->create_br(loop_enter); // DEBUG: 进入循环
// 		builder->set_insert_point(loop_enter);
// 		//expand expression to get the ret of expression(used to see if it should loop) and insert instruction
// 		node.expression->accept(*this);
// 		if (ret->get_type() == typeintptr || ret->get_type() == typefloatptr)
// 		{
// 			ret = builder->create_load(ret);
// 		}

// 		if (ret->get_type() == typeint1)
// 		{
// 			ret = builder->create_zext(ret, typeint);
// 		}
// 		if (ret->get_type() == typeint)
// 		{
// 			ret = builder->create_icmp_ne(ret, CONST_INT(0));
// 		}
// 		if (ret->get_type() == typefloat)
// 		{
// 			ret = builder->create_fcmp_ne(ret, CONST_FP(0));
// 		}
// 		//create branch
// 		builder->create_cond_br(ret, loop, loop_out);

// 		//loop
// 		builder->set_insert_point(loop);
// 		//expand statement
// 		node.statement->accept(*this);

// 		if (!isAfterReturn)
// 		//builder->get_insert_block()->get_terminator() == nullptr
// 		//not return in statement block
// 		{
// 			//loop again
// 			builder->create_br(loop_enter);
// 		}
// 		isAfterReturn = 0;
// 		//go out(condition or loop terminated from statement inside loop)
// 		builder->set_insert_point(loop_out);
// 	}
// }

// void CminusfBuilder::visit(ASTReturnStmt &node)
// {
// 	//return-stmt→return ; ∣ return expression ;
// 	if (!isAfterReturn)
// 	{
// 		Type *typeint = Type::get_int32_type(module.get());
// 		Type *typeintptr = Type::get_int32_ptr_type(module.get());
// 		Type *typefloatptr = Type::get_float_ptr_type(module.get());
// 		Type *typefloat = Type::get_float_type(module.get());
// 		Type *typeint1 = Type::get_int1_type(module.get());

// 		if (node.expression != nullptr)
// 		//return expression(not a void type)
// 		{
// 			node.expression->accept(*this);
// 			if (ret->get_type() == typeintptr || ret->get_type() == typefloatptr)
// 			{
// 				auto ret_load = builder->create_load(ret);
// 				if (ret_load->get_type() == builder->get_insert_block()->get_parent()->get_return_type())
// 				{
// 					builder->create_ret(ret_load);
// 				}
// 				else if ((ret_load->get_type() == typeint && builder->get_insert_block()->get_parent()->get_return_type() == typefloat))
// 				{
// 					auto ret_trans = builder->create_sitofp(ret_load, typefloat);
// 					builder->create_ret(ret_trans);
// 				}
// 				else if ((ret_load->get_type() == typefloat && builder->get_insert_block()->get_parent()->get_return_type() == typeint))
// 				{
// 					auto ret_trans = builder->create_fptosi(ret_load, typeint);
// 					builder->create_ret(ret_trans);
// 				}
// 			}
// 			else if (ret->get_type() == typeint1)
// 			{
// 				auto ret_load = builder->create_zext(ret, typeint);
// 				if (builder->get_insert_block()->get_parent()->get_return_type() == typefloat)
// 				{
// 					auto ret_trans = builder->create_sitofp(ret_load, typefloat);
// 					builder->create_ret(ret_trans);
// 				}
// 				else
// 					builder->create_ret(ret_load);
// 			}
// 			else if (ret->get_type() == builder->get_insert_block()->get_parent()->get_return_type())
// 			{
// 				builder->create_ret(ret);
// 			}
// 			else if ((ret->get_type() == typeint && builder->get_insert_block()->get_parent()->get_return_type() == typefloat))
// 			{
// 				auto ret_load = builder->create_sitofp(ret, typefloat);
// 				builder->create_ret(ret_load);
// 			}
// 			else if ((ret->get_type() == typefloat && builder->get_insert_block()->get_parent()->get_return_type() == typeint))
// 			{
// 				auto ret_load = builder->create_fptosi(ret, typeint);

// 				builder->create_ret(ret_load);
// 			}
// 		}
// 		else
// 			builder->create_void_ret();
// 		isAfterReturn = 1;
// 	}
// 	else
// 	{
// 		builder->create_void_ret();
// 	}
// }
// void CminusfBuilder::visit(ASTVar &node)
// {
// 	//var→ID ∣ ID [ expression]
// 	if (node.expression == nullptr)
// 	//type is pure ID
// 	{

// 		ret = scope.find(node.id);
// 	}
// 	else
// 	//type is array
// 	{
// 		Type *typeint = Type::get_int32_type(module.get());
// 		Type *typeintptr = Type::get_int32_ptr_type(module.get());
// 		Type *typefloatptr = Type::get_float_ptr_type(module.get());
// 		Type *typefloat = Type::get_float_type(module.get());
// 		Type *typeint1 = Type::get_int1_type(module.get());
// 		//expand expression
// 		node.expression->accept(*this);
// 		//note that:
// 		//the value of expression can be:INTEGER VARIABLE=EXPRESSION AND SOME SIMPLE EXPRESSION
// 		//the following discussion is based on different types of expression
// 		auto temp_ret = ret;
// 		auto id_type = scope.find(node.id);
// 		//id_type: INT OR FLOAT OR POINTER OF ITS TYPE
// 		//Discussion about type of expression
// 		//1.FLOAT
// 		while (temp_ret->get_type() == typefloatptr || temp_ret->get_type() == typeintptr)
// 		{
// 			temp_ret = builder->create_load(temp_ret);
// 		}
// 		if (temp_ret->get_type() == typefloat)
// 		{
// 			temp_ret = builder->create_fptosi(temp_ret, typeint);
// 		}
// 		//2.bool(int1)
// 		else if (temp_ret->get_type() == typeint1)
// 		{
// 			temp_ret = builder->create_zext(temp_ret, typeint);
// 		}
// 		auto comparison = builder->create_icmp_ge(temp_ret, CONST_INT(0));
// 		auto truebb = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
// 		auto falsebb = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
// 		auto outbb = BasicBlock::create(module.get(), "", builder->get_insert_block()->get_parent());
// 		builder->create_cond_br(comparison, truebb, falsebb);
// 		builder->set_insert_point(truebb);
// 		//Disscussion about now there are three possible types of id:int* int** float* float**
// 		//if id_type is int** or float**,after load,it will be int* or float*
// 		//if id_type is int* or float*,after load,it will be int or float
// 		//but what we need int* and float*
// 		Value *id_load;
// 		id_load = builder->create_load(id_type);
// 		//id is int** or float**
// 		/** BUG:
// 		 * we still don't know the exact usage of create_gep
// 		 * also we still don't know if the judgement:id_load->get_type()->is_array_type() is right to judge bidimension array
// 		 **/
// 		if (id_load->get_type()->is_array_type())
// 		{
// 			id_load = builder->create_gep(id_type, {CONST_INT(0), CONST_INT(0)});
// 		}
// 		id_load = builder->create_gep(id_load, {temp_ret});
// 		ret = id_load;
// 		builder->create_br(outbb);
// 		//falsebb
// 		builder->set_insert_point(falsebb);
// 		auto neg_idx_except_fun = scope.find("neg_idx_except");
// 		builder->create_call(neg_idx_except_fun, {});
// 		builder->create_br(outbb);
// 		builder->set_insert_point(outbb);
// 	}
// }

// void CminusfBuilder::visit(ASTAssignExpression &node)
// {
// 	// expression -> var = expression | simple-expression -- by Rui Lv
// 	if (!isAfterReturn)
// 	{
// 		node.var.get()->accept(*this);
// 		// get var => ret => store in var
// 		Value *var = ret;
// 		// get expression => ret
// 		node.expression.get()->accept(*this);
// 		auto Int32PtrType = Type::get_int32_ptr_type(module.get()); // 单个参数的类型,指针
// 		auto FloatPtrType = Type::get_float_ptr_type(module.get());
// 		auto floatType = Type::get_float_type(module.get());
// 		auto Int32Type = Type::get_int32_type(module.get());

// 		// judge the type of expression
// 		// it might be point or just a value
// 		auto expType = ret->get_type();
// 		if (expType == Int32PtrType || expType == FloatPtrType)
// 		{
// 			// if the expression is a point, load the value of this point => ret
// 			ret = builder->create_load(ret);
// 		}

// 		auto varType = var->get_type()->get_pointer_element_type(); // get the var type

// 		auto expValueType = ret->get_type();
// 		auto OneType = Type::get_int1_type(module.get());

// 		// var 和 expression 类型不一致，强制类型转换
// 		if (expValueType == OneType)
// 		{
// 			// if ret is OneType, turn to int32type
// 			ret = builder->create_zext(ret, Int32Type);
// 		}
// 		else if (expValueType != varType)
// 		{
// 			if (expValueType == floatType)
// 			{
// 				ret = builder->create_fptosi(ret, Int32Type);
// 			}
// 			else
// 			{
// 				ret = builder->create_sitofp(ret, floatType);
// 			}
// 		}

// 		// the value of expression (ret) should be store in var
// 		// var is a point of Value,it is a block of memory
// 		builder->create_store(ret, var);
// 	}
// }

// void CminusfBuilder::visit(ASTSimpleExpression &node)
// {
// 	// simple-expression -> additive-expression relop additive-expression | additive-expression -- by Rui Lv
// 	if (!isAfterReturn)
// 	{
// 		node.additive_expression_l.get()->accept(*this);

// 		// simple-expression -> additive-expression relop additive-expression
// 		// 关系运算
// 		if (node.additive_expression_r != nullptr)
// 		{
// 			auto Int32PtrType = Type::get_int32_ptr_type(module.get()); // 单个参数的类型,指针
// 			auto FloatPtrType = Type::get_float_ptr_type(module.get());

// 			Value *lexp; // the value of left expression
// 			Value *rexp; // the value of right expression

// 			auto lexpType = ret->get_type();
// 			if (lexpType == Int32PtrType || lexpType == FloatPtrType)
// 			{
// 				// if the left expression is a point, load the value of this point => ret
// 				lexp = builder->create_load(ret);
// 			}
// 			else
// 			{
// 				lexp = ret;
// 			}

// 			// get the right expression's value
// 			node.additive_expression_r.get()->accept(*this);

// 			auto rexpType = ret->get_type();
// 			if (rexpType == Int32PtrType || rexpType == FloatPtrType)
// 			{
// 				// if the left expression is a point, load the value of this point => ret
// 				rexp = builder->create_load(ret);
// 			}
// 			else
// 			{
// 				rexp = ret;
// 			}

// 			//-------- 两个表达式类型不同，强制类型转换，对 int1type 和 int32type 分别做类型提升 ------
// 			auto OneType = Type::get_int1_type(module.get());
// 			auto floatType = Type::get_float_type(module.get());
// 			auto Int32Type = Type::get_int32_type(module.get());

// 			auto ltype = lexp->get_type();
// 			auto rtype = rexp->get_type();

// 			if (ltype == OneType || rtype == OneType)
// 			{
// 				if (ltype == OneType)
// 				{
// 					lexp = builder->create_zext(lexp, Int32Type);
// 				}
// 				if (rtype == OneType)
// 				{
// 					rexp = builder->create_zext(rexp, Int32Type);
// 				}
// 			}
// 			if ((ltype != rtype) && (ltype == floatType || rtype == floatType))
// 			{
// 				if (ltype == floatType)
// 				{
// 					rexp = builder->create_sitofp(rexp, floatType);
// 				}
// 				else
// 				{
// 					lexp = builder->create_sitofp(lexp, floatType);
// 				}
// 			}

// 			// if(lexp->get_type() != rexp->get_type) exit(-1);
// 			int typeFlag = 0;
// 			if (lexp->get_type() == floatType)
// 			{
// 				typeFlag = 1;
// 			}
// 			//------ relop -----
// 			Value *cmp;
// 			switch (node.op)
// 			{
// 			case OP_EQ:
// 				if (typeFlag)
// 					cmp = builder->create_fcmp_eq(lexp, rexp);
// 				else
// 					cmp = builder->create_icmp_eq(lexp, rexp);
// 				break;
// 			case OP_GE:
// 				if (typeFlag)
// 					cmp = builder->create_fcmp_ge(lexp, rexp);
// 				else
// 					cmp = builder->create_icmp_ge(lexp, rexp);
// 				break;
// 			case OP_GT:
// 				if (typeFlag)
// 					cmp = builder->create_fcmp_gt(lexp, rexp);
// 				else
// 					cmp = builder->create_icmp_gt(lexp, rexp);
// 				break;
// 			case OP_LE:
// 				if (typeFlag)
// 					cmp = builder->create_fcmp_le(lexp, rexp);
// 				else
// 					cmp = builder->create_icmp_le(lexp, rexp);
// 				break;
// 			case OP_LT:
// 				if (typeFlag)
// 					cmp = builder->create_fcmp_lt(lexp, rexp);
// 				else
// 					cmp = builder->create_icmp_lt(lexp, rexp);
// 				break;
// 			case OP_NEQ:
// 				if (typeFlag)
// 					cmp = builder->create_fcmp_ne(lexp, rexp);
// 				else
// 					cmp = builder->create_icmp_ne(lexp, rexp);
// 				break;
// 			default:
// 				break;
// 			}
// 			ret = cmp; // store the relop value into ret
// 		}
// 	}
// }

// void CminusfBuilder::visit(ASTAdditiveExpression &node)
// {
// 	/* additive_expression -> term */
// 	if (!isAfterReturn)
// 	{
// 		if (node.additive_expression == nullptr)
// 		{
// 			/* term1 in (additive_expression -> term1 addop term) is null */
// 			/* means that not this choice */
// 			if (node.term != nullptr)
// 			{
// 				node.term->accept(*this);
// 			}
// 		}

// 		/* additive_expression -> additive_expression1 addop term */
// 		else
// 		{
// 			/* both int or float types can be involved */
// 			Type *Int32Type = Type::get_int32_type(module.get());
// 			Type *FloatType = Type::get_float_type(module.get());
// 			Type *Int32PtrType = Type::get_int32_ptr_type(module.get());
// 			Type *FloatPtrType = Type::get_float_ptr_type(module.get());

// 			node.additive_expression->accept(*this);
// 			Value *lvalue;
// 			/* get type of element additive_expression */
// 			if (ret->get_type() == Int32PtrType)
// 			{
// 				lvalue = builder->create_load(Int32Type, ret);
// 			}
// 			else if (ret->get_type() == Int32Type)
// 			{
// 				lvalue = ret;
// 			}
// 			else if (ret->get_type() == FloatPtrType)
// 			{
// 				lvalue = builder->create_load(FloatType, ret);
// 			}
// 			else if (ret->get_type() == FloatType)
// 			{
// 				lvalue = ret;
// 			}
// 			node.term->accept(*this);

// 			Value *rvalue;
// 			/* get type of element term */
// 			if (ret->get_type() == Int32PtrType)
// 			{
// 				rvalue = builder->create_load(Int32Type, ret);
// 			}
// 			else if (ret->get_type() == Int32Type)
// 			{
// 				rvalue = ret;
// 			}
// 			else if (ret->get_type() == FloatPtrType)
// 			{
// 				rvalue = builder->create_load(FloatType, ret);
// 			}
// 			else if (ret->get_type() == FloatType)
// 			{
// 				rvalue = ret;
// 			}

// 			Value *result;
// 			Type *Int1Type = Type::get_int1_type(module.get());
// 			if (lvalue->get_type() == Int1Type)
// 			{
// 				lvalue = builder->create_zext(lvalue, Int32Type);
// 			}
// 			if (rvalue->get_type() == Int1Type)
// 			{
// 				rvalue = builder->create_zext(rvalue, Int32Type);
// 			}

// 			/* change int to float if there is a float as component */
// 			if ((lvalue->get_type() == FloatType) && (rvalue->get_type() == Int32Type))
// 			{
// 				rvalue = builder->create_sitofp(rvalue, FloatType);
// 			}
// 			if ((rvalue->get_type() == FloatType) && (lvalue->get_type() == Int32Type))
// 			{
// 				lvalue = builder->create_sitofp(lvalue, FloatType);
// 			}

// 			/* mul or div */
// 			if (node.op == OP_PLUS)
// 			{
// 				if (lvalue->get_type() == Int32Type)
// 				{
// 					result = builder->create_iadd(lvalue, rvalue);
// 				}
// 				else if (lvalue->get_type() == FloatType)
// 				{
// 					result = builder->create_fadd(lvalue, rvalue);
// 				}
// 			}
// 			else if (node.op == OP_MINUS)
// 			{
// 				if (lvalue->get_type() == Int32Type)
// 				{
// 					result = builder->create_isub(lvalue, rvalue);
// 				}
// 				else if (lvalue->get_type() == FloatType)
// 				{
// 					result = builder->create_fsub(lvalue, rvalue);
// 				}
// 			}
// 			ret = result;
// 		}
// 	}
// }

// void CminusfBuilder::visit(ASTTerm &node)
// {
// 	/* term -> factor */
// 	if (!isAfterReturn)
// 	{
// 		if (node.term == nullptr)
// 		{
// 			/* term1 in (term -> term1 mulop factor) is null */
// 			/* means that not this choice */
// 			if (node.factor != nullptr)
// 			{
// 				node.factor->accept(*this);
// 			}
// 		}

// 		/* term -> term1 mulop factor */
// 		else
// 		{
// 			/* both int or float types can be involved */
// 			Type *Int32Type = Type::get_int32_type(module.get());
// 			Type *FloatType = Type::get_float_type(module.get());
// 			Type *Int32PtrType = Type::get_int32_ptr_type(module.get());
// 			Type *FloatPtrType = Type::get_float_ptr_type(module.get());

// 			node.term->accept(*this);
// 			Value *lvalue;
// 			/* get type of element term */
// 			if (ret->get_type() == Int32PtrType)
// 			{
// 				lvalue = builder->create_load(Int32Type, ret);
// 			}
// 			else if (ret->get_type() == Int32Type)
// 			{
// 				lvalue = ret;
// 			}
// 			else if (ret->get_type() == FloatPtrType)
// 			{
// 				lvalue = builder->create_load(FloatType, ret);
// 			}
// 			else if (ret->get_type() == FloatType)
// 			{
// 				lvalue = ret;
// 			}
// 			node.factor->accept(*this);

// 			Value *rvalue;
// 			/* get type of element factor */
// 			if (ret->get_type() == Int32PtrType)
// 			{
// 				rvalue = builder->create_load(Int32Type, ret);
// 			}
// 			else if (ret->get_type() == Int32Type)
// 			{
// 				rvalue = ret;
// 			}
// 			else if (ret->get_type() == FloatPtrType)
// 			{
// 				rvalue = builder->create_load(FloatType, ret);
// 			}
// 			else if (ret->get_type() == FloatType)
// 			{
// 				rvalue = ret;
// 			}

// 			Value *result;
// 			Type *Int1Type = Type::get_int1_type(module.get());
// 			if (lvalue->get_type() == Int1Type)
// 			{
// 				lvalue = builder->create_zext(lvalue, Int32Type);
// 			}
// 			if (rvalue->get_type() == Int1Type)
// 			{
// 				rvalue = builder->create_zext(rvalue, Int32Type);
// 			}

// 			/* change int to float if there is a float as component */
// 			if ((lvalue->get_type() == FloatType) && (rvalue->get_type() == Int32Type))
// 			{
// 				rvalue = builder->create_sitofp(rvalue, FloatType);
// 			}
// 			if ((rvalue->get_type() == FloatType) && (lvalue->get_type() == Int32Type))
// 			{
// 				lvalue = builder->create_sitofp(lvalue, FloatType);
// 			}

// 			/* mul or div */
// 			if (node.op == OP_MUL)
// 			{
// 				if (lvalue->get_type() == Int32Type)
// 				{
// 					result = builder->create_imul(lvalue, rvalue);
// 				}
// 				else if (lvalue->get_type() == FloatType)
// 				{
// 					result = builder->create_fmul(lvalue, rvalue);
// 				}
// 			}
// 			else if (node.op == OP_DIV)
// 			{
// 				if (lvalue->get_type() == Int32Type)
// 				{
// 					result = builder->create_isdiv(lvalue, rvalue);
// 				}
// 				else if (lvalue->get_type() == FloatType)
// 				{
// 					result = builder->create_fdiv(lvalue, rvalue);
// 				}
// 			}
// 			ret = result;
// 		}
// 	}
// }

// void CminusfBuilder::visit(ASTCall &node)
// {

// 	/* to get the type specifier, int32 or float or their pointers, or bool, for args */
// 	if (!isAfterReturn)
// 	{
// 		Type *Int32Type = Type::get_int32_type(module.get());
// 		Type *FloatType = Type::get_float_type(module.get());
// 		Type *Int32PtrType = Type::get_int32_ptr_type(module.get());
// 		Type *FloatPtrType = Type::get_float_ptr_type(module.get());
// 		Type *Int1Type = Type::get_int1_type(module.get());

// 		/** TODO:
// 	 * need to check between fun declaration and defination
// 	 **/

// 		/* enter the scope of terminal symbol ID */
// 		auto IDalloc = scope.find(node.id);
// 		auto fun = IDalloc->get_type();
// 		auto callFun = static_cast<FunctionType *>(fun);
// 		std::vector<Type *> params; // get the params value of the function

// 		for (auto arg = callFun->param_begin(); arg != callFun->param_end(); arg++)
// 		{
// 			params.push_back(*arg);
// 		}

// 		// if(params.size() != node.args.size())

// 		if (IDalloc == nullptr)
// 		{
// 			exit(0); /* no need to take action */
// 		}
// 		else
// 		{
// 			int i = 0; // record the id of params
// 			std::vector<Value *> funArgs;
// 			for (auto expr : node.args)
// 			{

// 				expr->accept(*this);

// 				if (ret->get_type() == Int1Type)
// 				{
// 					//whatever, turn the int1Type to Int32Type
// 					ret = builder->create_zext(ret, Int32Type);
// 				}

// 				if (params[i] == Int32Type)
// 				{
// 					if (ret->get_type() == Int32Type)
// 					{
// 						ret = ret;
// 					}
// 					else if (ret->get_type() == FloatType)
// 					{
// 						// DEBUG:
// 						// printf("the ret is float type!\n");
// 						// std::cout << ret->print();

// 						ret = builder->create_fptosi(ret, Int32Type);

// 						// std::cout << ret->print();
// 					}
// 					else if (ret->get_type() == Int32PtrType)
// 					{
// 						ret = builder->create_load(ret);
// 					}
// 					else if (ret->get_type() == FloatPtrType)
// 					{
// 						ret = builder->create_load(ret);
// 						ret = builder->create_fptosi(ret, Int32Type);
// 					}
// 				}
// 				else if (params[i] == FloatType)
// 				{
// 					if (ret->get_type() == FloatType)
// 					{
// 						ret = ret;
// 					}
// 					else if (ret->get_type() == Int32Type)
// 					{
// 						ret = builder->create_sitofp(ret, FloatType);
// 					}
// 					else if (ret->get_type() == Int32PtrType)
// 					{
// 						ret = builder->create_load(ret);
// 						ret = builder->create_sitofp(ret, FloatType);
// 					}
// 					else if (ret->get_type() == FloatPtrType)
// 					{
// 						ret = builder->create_load(ret);
// 					}
// 				}

// 				else if (ret->get_type()->get_pointer_element_type()->is_array_type())
// 				{
// 					// ret is array*,it could be intType or floatType
// 					ret = builder->create_gep(ret, {CONST_INT(0), CONST_INT(0)});
// 					// now we get the first address of the array => ret
// 				}
// 				else if (ret->get_type()->get_pointer_element_type() == Int32PtrType)
// 				{
// 					// ret is i32**
// 					ret = builder->create_load(ret);
// 					// now ret is i32*
// 				}
// 				else if (ret->get_type()->get_pointer_element_type() == FloatPtrType)
// 				{
// 					// ret is float**
// 					ret = builder->create_load(ret);
// 					// now ret is float*
// 				}

// 				// DEBUG：
// 				// ret->print();
// 				funArgs.push_back(ret);
// 				i++; // params++
// 			}
// 			ret = builder->create_call(IDalloc, funArgs);
// 		}
// 	}
// }
// // 编译器真的帮我们干了很多事


