# lab3 实验报告
<p align=right>PB18000227 艾语晨 </p>

## 问题1: cpp与.ll的对应
请描述你的cpp代码片段和.ll的每个BasicBlock的对应关系。描述中请附上两者代码。

### assign

#### assign_generator.cpp

<details>
	<summary>assign.cpp</summary>

```cpp
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

	// main函数
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
									"main", module);
	auto bb = BasicBlock::create(module, "entry", mainFun);
	// BasicBlock的名字在生成中无所谓,但是可以方便阅读
	builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb

	auto retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置
	builder->create_store(CONST_INT(0), retAlloca); // 默认 ret 0

	auto *arrayType = ArrayType::get(Int32Type,10);
	auto aAlloca = builder->create_alloca(arrayType); // 在内存中分配参a[10]的位置

	auto a0GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)}); // GEP: 这里为什么是{0, 0}呢? (实验报告相关)
	builder->create_store(CONST_INT(10), a0GEP);

	auto a1GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});

	auto a0Load = builder->create_load(a0GEP);
	auto mul = builder->create_imul(a0Load, CONST_INT(2)); // a[0] * 2
	builder->create_store(mul, a1GEP);

	auto retBB = BasicBlock::create(module, "", mainFun); // return分支
	builder->create_store(mul, retAlloca);
	builder->create_br(retBB); // br retBB

	builder->set_insert_point(retBB); // ret分支
	auto retLoad = builder->create_load(retAlloca);
	builder->create_ret(retLoad);

	std::cout << module->print();
	delete module;
	return 0;
}
```

</details>

#### Assign.ll

<details>
	<summary>assign.ll</summary>

```assembly
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca [10 x i32]
  %2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 0
  store i32 10, i32* %2
  %3 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 1
  %4 = load i32, i32* %2
  %5 = mul i32 %4, 2
  store i32 %5, i32* %3
  store i32 %5, i32* %0
  br label %6
6:
  %7 = load i32, i32* %0
  ret i32 %7
}
```

</details>

#### 对应关系

共有两个BasicBlock，分别如下：

1. `auto bb = BasicBlock::create(module, "entry", mainFun);`<br />对应标记`entry`
2. `auto retBB = BasicBlock::create(module, "", mainFun);`<br />对应标记`6`

### fun

#### Fun_generator.cpp

<details>
	<summary>fun_generator.cpp</summary>

```cpp
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

	// callee function
	// 函数参数类型的vector
	std::vector<Type *> Ints(1, Int32Type);

	// 由函数类型得到函数
	auto calleeFun = Function::create(FunctionType::get(Int32Type, Ints), "callee", module);

	// BB的名字在生成中无所谓,但是可以方便阅读
	auto bb = BasicBlock::create(module, "entry", calleeFun);

	builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb

	auto retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置
	builder->create_store(CONST_INT(0), retAlloca);		// 默认 ret 0

	auto aAlloca = builder->create_alloca(Int32Type); // 在内存中分配参数a的位置

	std::vector<Value *> args; // 获取callee函数的形参,通过Function中的iterator
	for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++)
	{
		args.push_back(*arg); // * 号运算符是从迭代器中取出迭代器当前指向的元素
	}

	builder->create_store(args[0], aAlloca); // 将参数a store下来
	auto aLoad = builder->create_load(aAlloca); // 将参数a load上来
	auto mul = builder->create_imul(aLoad, CONST_INT(2)); // a[0] * 2

	auto retBB = BasicBlock::create(module, "", calleeFun); // return分支
	builder->create_store(mul, retAlloca);
	builder->create_br(retBB); // br retBB

	builder->set_insert_point(retBB); // ret分支
	auto retLoad = builder->create_load(retAlloca);
	builder->create_ret(retLoad);

	// main函数
	auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
									"main", module);
	bb = BasicBlock::create(module, "entry", mainFun);
	// BasicBlock的名字在生成中无所谓,但是可以方便阅读
	builder->set_insert_point(bb); // 一个BB的开始,将当前插入指令点的位置设在bb

	retAlloca = builder->create_alloca(Int32Type); // 在内存中分配返回值的位置
	builder->create_store(CONST_INT(0), retAlloca);		// 默认 ret 0

	auto call = builder->create_call(calleeFun, {CONST_INT(110)});
	builder->create_ret(call);

	std::cout << module->print();
	delete module;
	return 0;
}
```

</details>

#### Fun.ll

<details>
	<summary>fun.ll</summary>

```assembly
define i32 @callee(i32 %0) {
entry:
  %1 = alloca i32
  store i32 0, i32* %1
  %2 = alloca i32
  store i32 %0, i32* %2
  %3 = load i32, i32* %2
  %4 = mul i32 %3, 2
  store i32 %4, i32* %1
  br label %5
5:
  %6 = load i32, i32* %1
  ret i32 %6
}
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = call i32 @callee(i32 110)
  ret i32 %1
}
```

</details>

#### 对应关系

共有3个basic block：

1. `auto bb = BasicBlock::create(module, "entry", calleeFun);`<br />对应`callee`函数中的标记`entry`
2. `auto retBB = BasicBlock::create(module, "", calleeFun);`<br />对应`callee`函数中的标记`5`
3. `bb = BasicBlock::create(module, "entry", mainFun);`<br/>对应`main`中的标记`entry`

### if

#### If_generator.cpp

<details>
	<summary>if_generator.cpp</summary>

```cpp
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
```

</details>

#### if.ll

<details>
	<summary>if.ll</summary>

```assembly
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca float
  store float 0x40163851e0000000, float* %1
  %2 = load float, float* %1
  %3 = fcmp ugt float %2,0x3ff0000000000000
  br i1 %3, label %trueBB, label %falseBB
trueBB:
  store i32 233, i32* %0
  br label %4
falseBB:
  store i32 0, i32* %0
  br label %4
4:
  %5 = load i32, i32* %0
  ret i32 %5
}
```

</details>

#### 对应关系

共有4个basic block：

1. `auto bb = BasicBlock::create(module, "entry", mainFun);`<br/>对应标记`entry`
2. `auto trueBB = BasicBlock::create(module, "trueBB", mainFun);`<br />对应标记`trueBB`
3. `auto falseBB = BasicBlock::create(module, "falseBB", mainFun);`<br />对应标记`falseBB`
4. `auto retBB = BasicBlock::create(module, "", mainFun);`<br />对应标记`4`

### while

#### While.cpp

<details>
	<summary>while.cpp</summary>

```cpp
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

	auto aAlloca = builder->create_alloca(Int32Type); // 在内存中分配参a的位置
	builder->create_store(CONST_INT(10), aAlloca);
	auto iAlloca = builder->create_alloca(Int32Type); // 在内存中分配参a的位置
	builder->create_store(CONST_INT(0), iAlloca);

	auto aLoad = builder->create_load(aAlloca);				  // 将参数a load上来
	auto iLoad = builder->create_load(iAlloca);				  // 将参数i load上来

	auto loop = BasicBlock::create(module, "loop", mainFun);
	builder->create_br(loop);		 // br loop
	builder->set_insert_point(loop); // the entry for loop

	iLoad = builder->create_load(iAlloca);				   // 将参数i load上来
	auto icmp = builder->create_icmp_lt(iLoad, CONST_INT(10)); // a和1的比较,注意ICMPEQ

	auto trueBB = BasicBlock::create(module, "trueBB", mainFun);   // inside while
	auto falseBB = BasicBlock::create(module, "falseBB", mainFun); // after while
	auto retBB = BasicBlock::create(module, "", mainFun);		   // return分支

	auto br = builder->create_cond_br(icmp, trueBB, falseBB); // 条件BR
	DEBUG_OUTPUT											  // 我调试的时候故意留下来的,以醒目地提醒你这个调试用的宏定义方法

	builder->set_insert_point(trueBB); // if true; 分支的开始需要SetInsertPoint设置
	iLoad = builder->create_load(iAlloca);	  // 将参数i load上来
	auto addi = builder->create_iadd(iLoad, CONST_INT(1)); // add result for i
	builder->create_store(addi, iAlloca);
	aLoad = builder->create_load(aAlloca); // 将参数a load上来
	iLoad = builder->create_load(iAlloca); // 将参数i load上来
	auto adda = builder->create_iadd(iLoad, aLoad); // the same
	builder->create_store(adda, aAlloca);
	builder->create_br(loop);			   // br loop
	builder->set_insert_point(loop);	   // the entry for loop

	builder->set_insert_point(falseBB); // after while
	aLoad = builder->create_load(aAlloca); // 将参数a load上来
	builder->create_store(aLoad, retAlloca);
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
```

</details>

<details>
  <summary>while.ll</summary>

```assembly
define i32 @main() {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca i32
  store i32 10, i32* %1
  %2 = alloca i32
  store i32 0, i32* %2
  %3 = load i32, i32* %1
  %4 = load i32, i32* %2
  br label %loop
loop:
  %5 = load i32, i32* %2
  %6 = icmp slt i32 %5, 10
  br i1 %6, label %trueBB, label %falseBB
trueBB:
  %7 = load i32, i32* %2
  %8 = add i32 %7, 1
  store i32 %8, i32* %2
  %9 = load i32, i32* %1
  %10 = load i32, i32* %2
  %11 = add i32 %10, %9
  store i32 %11, i32* %1
  br label %loop
falseBB:
  %12 = load i32, i32* %1
  store i32 %12, i32* %0
  br label %13
13:
  %14 = load i32, i32* %0
  ret i32 %14
}
```

</details>

#### 对应关系

共有5个basic block：

1. `auto bb = BasicBlock::create(module, "entry", mainFun);`<br />对应`entry`
2. `auto loop = BasicBlock::create(module, "loop", mainFun);`<br />对应`loop`
3. `auto trueBB = BasicBlock::create(module, "trueBB", mainFun);`<br />对应标记`trueBB`
4. `auto falseBB = BasicBlock::create(module, "falseBB", mainFun);`<br />对应标记`falseBB`
5. `auto retBB = BasicBlock::create(module, "", mainFun);`<br />对应标记`13`

## 问题2: Visitor Pattern

请指出visitor.cpp中，`treeVisitor.visit(exprRoot)`执行时，以下几个Node的遍历序列:numberA、numberB、exprC、exprD、exprE、numberF、exprRoot。
序列请按如下格式指明：
exprRoot->numberF->exprE->numberA->exprD

从三个`visit`方法可以看出，是先遍历右子树，所以是下面这棵树的右序遍历时，途径的结点：

![]( IMG_20201116_104447.jpg)

exprRoot->numberF->exprE->exprD->numberA->exprD->numberB->exprE->exprC->numberB->exprC->numberA->exprC->exprE->exprRoot

## 问题3: getelementptr
请给出`IR.md`中提到的两种getelementptr用法的区别,并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0`
  - `%2 = getelementptr i32, i32* %1 i32 %0`

第一个是[10 x i32]，假设第三个元素是n，第四个元素是m，返回的指针类型是[10 x i32]，但是要前移10n+m个单位

第二个是i32，偏移%0

## 实验难点
描述在实验中遇到的问题、分析和解决方案

llvm文件和cpp的对应关系，以及不会C++。

手动查资料……逐行对照，照葫芦画瓢

## 实验反馈
吐槽?建议?

🈚️
