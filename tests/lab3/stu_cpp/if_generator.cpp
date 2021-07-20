#include "../../../include/lightir/BasicBlock.h"
#include "../../../include/lightir/Constant.h"
#include "../../../include/lightir/Function.h"
#include "../../../include/lightir/IRBuilder.h"
#include "../../../include/lightir/Module.h"
#include "../../../include/lightir/Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG											 // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl; // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
	ConstantInt::get(num, module)

#define CONST_FP(num) \
	ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main()
{
	auto module = new Module("Cminus code"); // module name是什么无关紧要
	auto builder = new IRBuilder(nullptr, module);
	Type *Int32Type = Type::get_int32_type(module);
	Type *FloatType = Type::get_float_type(module);

	// main函数
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
									"main", module);
	auto bb = BasicBlock::create(module, "entry", mainFun);
	// BasicBlock的名字在生成中无所谓,但是可以方便阅读
	builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb

	auto retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置
	builder->create_store(CONST_INT(0), retAlloca);		// 默认 ret 0

	auto aAlloca = builder->create_alloca(FloatType); // 在内存中分配参a的位置
	builder->create_store(CONST_FP(5.555), aAlloca);

	auto aLoad = builder->create_load(aAlloca);				  // 将参数a load上来
	auto fcmp = builder->create_fcmp_gt(aLoad, CONST_FP(1)); // a和1的比较,注意ICMPEQ

	auto trueBB = BasicBlock::create(module, "trueBB", mainFun); // true分支
	auto falseBB = BasicBlock::create(module, "falseBB", mainFun); // false分支
	auto retBB = BasicBlock::create(module, "", mainFun); // return分支

	auto br = builder->create_cond_br(fcmp, trueBB, falseBB); // 条件BR
	DEBUG_OUTPUT											  // 我调试的时候故意留下来的,以醒目地提醒你这个调试用的宏定义方法

	builder->set_insert_point(trueBB); // if true; 分支的开始需要SetInsertPoint设置
	builder->create_store(CONST_INT(233), retAlloca);
	builder->create_br(retBB); // br retBB

	builder->set_insert_point(falseBB); // if false
	builder->create_store(CONST_INT(0), retAlloca);
	builder->create_br(retBB); // br retBB

	builder->set_insert_point(retBB); // ret分支
	auto retLoad = builder->create_load(retAlloca);
	builder->create_ret(retLoad);
	// 给这么多注释了,但是可能你们还是会弄很多bug
	// 所以强烈建议配置AutoComplete,效率会大大提高!
	// 别人配了AutoComplete,只花1小时coding
	// 你没有配AutoComplete,找method花5小时,debug花5小时,肯定哭唧唧!
	// 最后,如果猜不到某个IR指令对应的C++的函数,建议把指令翻译成英语然后在method列表中搜索一下
	// 最后的最后,这个例子只涉及到了一点基本的指令生成,
	// 对于额外的指令,包括数组,在之后的实验中可能需要大家好好搜索一下思考一下,
	// 还有涉及到的C++语法,可以在gitlab上发issue提问或者向大家提供指导,会有bonus哦!
	// 对于这个例子里的代码风格/用法,如果有好的建议也欢迎提出!
	std::cout << module->print();
	delete module;
	return 0;
}
