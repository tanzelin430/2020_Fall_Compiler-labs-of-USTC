# Lab5 实验报告

<p align=right>吕瑞 PB18111707 谭泽霖 PB18010454 艾语晨 PB18000227</p>



## 实验要求

开发基本优化 Pass

1. **常量传播**

    能够实现在编译优化阶段，能够计算出结果的变量，就直接替换为常量；

    a. 只需要考虑过程内的常量传播，可以不用考虑数组，**全局变量只需要考虑块内的常量传播**，同时删除无用的赋值和计算语句。

    b. 整形浮点型都需要考虑， 对于`a=1/0`的情形，可以不考虑，即可以做处理也可以不处理。

    c. 做到**删除无用的分支**将条件跳转变换为强制性跳转。

    d. 同时对于分支嵌套的情况都能够删除掉无用的分支，这一步之后对于可能出现的无法到达的块都需要进行删除，而至于只含有跳转指令的冗余块在本次实验中不要求实现。

2. **循环不变式外提**

    要能够实现将与循环无关的表达式提取到循环的外面。不用考虑数组，与全局变量。

3. **活跃变量分析**

    能够实现分析bb块的入口和出口的活跃变量，参考资料见附件(紫书9.2.4节)，在`ActiveVars.hpp`中定义了两个成员`live_in`, `live_out`，你需要将`pair<bb, IN[bb]>`插入`live_in`的map 结构中，将`pair<bb, OUT[bb]>`插入`live_out` 的map 结构中，并调用ActiveVars类中的print()方法输出bb活跃变量情况到json文件。

    材料中没有phi节点的设计，因此数据流方程：$OUT[B] =\cup_{s是B的后继}IN[S]$ 的定义蕴含着S入口处活跃的变量在它所有前驱的出口处都是活跃的，由于`phi`指令的特殊性，例如`%0 = phi [%op1, %bb1], [%op2, %bb2]`如果使用如上数据流方程，则默认此`phi`指令同时产生了`op1`与`op2`的活跃性，而事实上只有控制流从`%bb1`传过来才有`%op1`的活跃性，从`%bb2`传过来才有`%op2`的活跃性。因此对此数据流方程需要做一些修改。

## 实验难点

### 常量传播

- 由于采用的是SSA，所以每一条指令都是独一无二的，它的左值就是这个`Instruction::instr`本身

- 指令的右值、类型、返回值类型都可以通过成员方法调用得到

- 由于`Instruction`类继承了`User`类，于是所有instruction实例都有一个`replace_all_use_with`方法。这个方法可以将所有的用到这个左值的地方都替换为参数里面的值

    - 然而，在处理全局变量时，由于要求全局变量的替换只能在一个bb内进行，就不能用如上方法，而是要将此instr的使用列表`use_list`进行遍历

- 对于死代码删除，可以维护一个待删除列表，

- 对于运算指令（比如比较指令），可以仿照给出来的binary指令，定义一个单独的方法来做

- 所有需要做常数折叠的指令都需要在常数折叠之后接一句常量传播

    #### 删除无用分支

    目标：删除 cfg 中，必然不会经过的分支；

    ##### 思路

    #####  1. 为什么会产生无用分支？

    ANS：在常量传播优化之后，可能会出现条件语句的结果为常数的情况，例如 `if(1>0){...}else{...}`，此时 else 分支中的所有 bb 块都不会被经过，分支跳转实质上变成了无条件跳转语句。

    ##### 2. 如何判断分支无用？

    ANS：在三参数跳转指令中，如果第一个参数（比较指令的执行结果）为定值（true/false），则说明必有一个分支无用。

    - true：`if(true){...}else{...}`，else 分支中的所有块都应该被删除。

        ​			`if(true){...} then ...` ，then 这条分支不会经过，虽然不能删除 then 			之后的语句块，但是可以删除块之间的前驱和后继关系。

    - false: `if(flase){...} else/then {...}`， `true` 分支必不会经过，其中所有块都应该被删除。

    ##### 3. 如何删除无用分支？

    ANS：递归遍历删除。

    - 维护一个 `std::set<BiscBlock *>deleteBlockSet` ，记录一个 function 中必不会经过的 bb 块。

    - 找到每个 bb 块的三参数跳转指令，判断是否有分支需要被删除。

        若有，分 `true` 和 `false/next` 三种情况讨论。



### 循环不变式外提

- 循环不变式一定是自内向外提取；

- 思考如何判断语句与循环无关，且外提没有副作用；

    > 利用到达-定值分析，判断语句与循环无关：
    >
    > 如果循环中含有赋值 x=y+z，而 y 和 z **所有可能定值都在循环外部**，那么 y+z 就是循环不变计算；
    >
    > `s: x = y+z` 外提条件：
    >
    > 1. 【 s 所在的基本块**是循环所有出口节点**（有后继节点在循环外的结点）的支配结点】 或  【该不变运算所定值的不变量在循环出口之后是不活跃的】以上两种说法等价。
    > 2. 循环中没有其他语句对 x 赋值；
    > 3. 循环中对 x 的引用仅由 s 到达；

    在 phi 函数的加持下，以上这些条件在实际实现的过程中都不需要特别考虑。具体实现方式见如下第二点。

- 循环的条件块（base）最多只有两个前驱，思考不变式应该外提到哪个前驱；

    > 当然是提前到属于大循环的那个前驱；
    >
    > 具体：选择前驱中不在该循环里的那个语句块；

### 活跃变量分析

**根据use和def计算每个块的块头活跃变量集合和块尾活跃变量集合**

我们对每一个Block进行遍历，每一次遍历到一个block，便执行：

$OUT[BB] = \bigcup_{s是BB的直接后继}IN[S]$

$IN[BB] = use_{BB} \bigcup (OUT[BB] - def_{BB})$

需要重点注意的其实是第一条语句，因为一旦**涉及到phi函数**，事情会变得复杂起来，phi指令的两个右操作数分别来源于两个不同的label（包含在phi指令中），而这两个操作数又可能都存在于$IN[S]$中，在执行$OUT[BB] = \bigcup_{s是BB的直接后继}IN[S]$时，我们不能简单地将这两个存在于$IN[S]$中的操作数都插入$OUT[BB]$,而是要判断一下BB是不是跟对应待插入变量label相等，相等才能插入。

我们的判断条件如下，条件从前到后依次嵌套：

1.通过big_phi查找当前访问的后继块中是否存在phi指令

2.如果存在phi指令，判断当前遍历到的该后继块中的元素（即IN[S]中的元素）是否在phi指令中

3.如果存在于phi指令中，则检查该元素对应label跟该后继块的前驱块是否相等

以上三条满足，才能插入OUT[BB]这个存在于phi指令中的元素

如果1.2满足3不满足，则无法插入OUT[BB]

如果1满足2不满足，或者1不满足，则都可以插入OUT[BB]

注意，代码中的`INS`变量其实就对应于算法中的`IN[S]`,union_ins是个中间变量，用来存放IN[S]的并集结果，最后更新对应的live_out[bb]即可



## 实验设计

###  常量传播

目标：发掘代码中可能存在的常量，尽量用对常量的引用替代对虚拟寄存器的引用(虚拟寄存器和变量是同一个概念，以下都使用变量)，并尽量计算出可以计算的常量表达式

常量传播分为常量传播、常数折叠和死代码删除三部分，前两部分交替执行最终得到最佳结果。

常量传播需要对每一条指令中，Value已知为常数的参量都替换为其值，而对于整条指令的操作数都不包含变量的时候，这条指令就可以通过常数折叠来计算出它的常值。

>设计两个标志变量 `changed & delete_bb` （其实可以优化成一个），在遍历一个 function 中的所有语句时，若在任意一处地方出现了常量传播的优化修改（`changed` 表示语句有修改，`delete_bb` 表示有基本块需要删除），标志变量都会置位 true；
>
>采用 `do{...}while(changed|delete_bb)` 循环来遍历 function 中的所有语句，直到整个 function 中没有可以优化的地方为止。

#### 1. 处理由于删除无用分支导致的死块

1. 这一步需要放在进入一个bb之前来做。如果这个块是一个死块，那么不对他进行访问，而如果这个块不是起始块且它没有前驱结点，那么它就是被删除的死块。将这个块与他的后继都切开，并将它放入删除块列表中，然后去访问下一个块
2. 这里需要注意的是，在操作之前，function就已经维护了一个在function中所有的基本块的列表，而这个表是不受块之间互相指向的前驱/后继指针的存在与否的影响的。在删除无用分支的过程中，只是改变了块之间的偏序关系

```cpp
if (deleteBlockSet.find(bbs) != deleteBlockSet.end())
{
  // bbs in deleteBlockset
  continue;
}
if ((bbs->get_pre_basic_blocks().size() == 0) && (bbs->get_name() != static_cast<std::string>("label_entry")))
{
  // bbs's prev is null
  delete_bb = true;
  deleteBlockSet.insert(bbs);
  changed = true;
  for (auto succ : bbs->get_succ_basic_blocks())
  {
    bbs->remove_succ_basic_block(succ);
    succ->remove_pre_basic_block(bbs);
  }
  continue;
}
```

#### 2. 处理binary指令

`instr = ins_type type op1, op2`

1. binary指令在两个操作数都为constant时可以进行常数折叠。由lab4的强制转换可知，无须判断两个操作数的类型，而是直接取出来整条指令的类型即可。
2. 将`instr->get_operand(i)`取出来的Value类型的操作数进行`cast_constant`操作，以判断它是否是变量
3. 计算出来最终的结果之后对这条指令做常量传播

```cpp
if (instr->isBinary())
  // only do the const work on binary instructions
{
  auto lVal = instr;
  auto rOperA = static_cast<BinaryInst *>(instr)->get_operand(0);
  auto rOperB = static_cast<BinaryInst *>(instr)->get_operand(1);
  if (const_val_stack.count(rOperA))
  {
    rOperA = (Value *)const_val_stack[rOperA];
  }
  if (const_val_stack.count(rOperB))
  {
    rOperB = (Value *)const_val_stack[rOperB];
  }

  Instruction::OpID opType = instr->get_instr_type();
  if ((instr->is_add()) || (instr->is_sub()) || (instr->is_mul()) || (instr->is_div()))
    // operation is int operation
  {
    auto ifLeftConstInt = cast_constantint(rOperA);
    auto ifRightConstInt = cast_constantint(rOperB);
    ConstantInt *constValue;
    if ((ifLeftConstInt != nullptr) && (ifRightConstInt != nullptr))
    {
      constValue = CF.compute(opType, ifLeftConstInt, ifRightConstInt);
      const_val_stack.insert(std::pair<Value *, Value *>(instr, constValue));
      instr->replace_all_use_with(constValue);
      wait_delete.push_back(instr);
      changed = true;
    }
  }
  else
  {
    auto ifLeftConstFp = cast_constantfp(rOperA);
    auto ifRightConstFp = cast_constantfp(rOperB);
    /* the same */
    ConstantFP *constValue;
    if ((ifLeftConstFp != nullptr) && (ifRightConstFp != nullptr))
    {
      constValue = CF.computefp(opType, ifLeftConstFp, ifRightConstFp);
      const_val_stack.insert(std::pair<Value *, Value *>(instr, constValue));
      instr->replace_all_use_with(constValue);
      wait_delete.push_back(instr);
      changed = true;
    }
  }
}
```

#### 3. 处理全局变量

1. 由于全局变量的生命是在所有function之外的，在bb中对全局变量的常数折叠操作就仅限于mem2reg中特意区分开而没有删除的`load`和`store`指令
2. 全局变量也是一个虚拟寄存器，那么`store`指令时，变量`@global`就已经得到了这个值，没有必要再load一次了
3. 于是在检测到store指令的操作数目的地址是全局变量时，便将这个全局变量和他的值存下来，在检测到同一个bb内的load指令源地址在全局变量常量集合中存在时，便将其做常数折叠处理
4. 注意这个全局变量的常量集合需定义在进入到bb之后，因为它的生存周期只能在这个bb内才有效
5. 而常量传播时也不能直接用`replace_all_use_with`，因为这个方法会将整个文件中出现instr的地方都无脑替换，所以应当将这个instr的use列表取出并遍历，如果一处使用是在当前块中就替换

```cpp
std::map<Value *, Value *> globalConstVar;

for (auto instr : bbs->get_instructions())
{
  if (instr->is_store())
  {
    auto toStore = static_cast<StoreInst *>(instr)->get_lval();
    auto storeValue = static_cast<StoreInst *>(instr)->get_rval();
    if (IS_GLOBAL_VARIABLE(toStore))
    {
      if (globalConstVar.find(toStore) == globalConstVar.end())
      {
        globalConstVar.insert(std::pair<Value *, Value *>(toStore, storeValue));
      }
      else
      {
        globalConstVar[toStore] = storeValue;
      }
    }
  }
  else if (instr->is_load())
  {
    auto loadFromWhere = instr->get_operand(0);
    if (globalConstVar.count(loadFromWhere))
    {
      auto thisGlobalValue = globalConstVar[loadFromWhere];
      // instr->replace_all_use_with(thisGlobalValue);
      for (auto use : instr->get_use_list())
      {
        if (static_cast<Instruction *>(use.val_)->get_parent() == bbs)
        {
          auto val = dynamic_cast<User *>(use.val_);
          assert(val && "new_val is not a user");
          val->set_operand(use.arg_no_, thisGlobalValue);
        }

      }
      wait_delete.push_back(instr);
    }
  }
```

#### 4. 处理$\Phi$指令

1. 对phi指令本身的处理很简单，只需要判断其参数所来自的bb是否是死块，如果是的话就将这个bb的标记和传参一起从phi指令中删去
2. phi指令的处理是无用分支删除的一部分
3. 如果 phi 函数中只剩一个操作数，可以删除 phi 函数，对并对左值进行常量传播。

```cpp
if (instr->is_phi())
{
    if (instr->get_num_operand() == 2)
	{
        // phi 只剩一个参数
        auto value = instr->get_operand(0);
        instr->replace_all_use_with(value);
        wait_delete.push_back(instr);
        changed = true;
	}
  else
	{
        for (int i = 0; i < instr->get_num_operand() / 2; i++)
        {
            auto brFromBB = static_cast<BasicBlock *>(instr->get_operand(2 * i + 1));
            if (deleteBlockSet.find(brFromBB) != deleteBlockSet.end())
            {
                // brFromBB is dead
                instr->remove_operands(2 * i, 2 * i + 1);
                changed = true;
            }
        }
	}
}
```

#### 5. 处理类型转换指令

1. 总共有三种类型转换指令，`int32->float, float->int32, int1->int32`
2. 由于没有适用于这三种类型的强制转换，只能将数据结构中存储的值取出来，然后用`static_cast`强制转换，再构造出目标结构类型

```cpp
if (instr->is_fp2si())
{
  auto fpValue = cast_constantfp(instr->get_operand(0));
  if (fpValue != nullptr)
  {
    auto constFpValue = fpValue->get_value();
    auto intValue = static_cast<int>(constFpValue);
    auto constInt = ConstantInt::get(intValue, m_);
    const_val_stack.insert(std::pair<Value *, Value *>(instr, constInt));
    instr->replace_all_use_with(constInt);
    wait_delete.push_back(instr);
    changed = true;
  }
}
else if (instr->is_si2fp())
{
 ...
}
```

#### 6. 处理比较指令

1. 比较指令是服务于br跳转指令的，若比较指令得到的是常数，那么这个跳转就一定会有死分支
2. 比较指令的操作类似于binary指令，也是用预先定义的成员方法来求出表达式的值，然后做常量传播处理

```cpp
else if ((instr->is_cmp()) || (instr->is_fcmp()))
{
  auto cmpA = instr->get_operand(0);
  auto cmpB = instr->get_operand(1);
  ConstantInt *cmpResult;
  if (instr->is_cmp())
  {
    auto constCmpA = cast_constantint(cmpA);
    auto constCmpB = cast_constantint(cmpB);
    if ((constCmpA != nullptr) && (constCmpB != nullptr))
    {
      auto cmpop = dynamic_cast<CmpInst *>(instr)->get_cmp_op();
      cmpResult = CF.compute_comp(cmpop, constCmpA, constCmpB);
      const_val_stack.insert({instr, cmpResult});
      instr->replace_all_use_with(cmpResult);
      wait_delete.push_back(instr);
      changed = true;
      // auto constInt = ConstantInt::add_use
    }
  }
  else if (instr->is_fcmp())
  {
   ...
  }
}
```

#### 7. 如何删除无用分支？

ANS：递归遍历删除。

- 维护一个 `std::set<BiscBlock *>deleteBlockSet` ，记录一个 function 中必不会经过的 bb 块。

- 找到每个 bb 块的三参数跳转指令，判断是否有分支需要被删除。

    若有，分 `true` 和 `false/next` 三种情况讨论。

    ```cpp
    if (cond > 0)
    {
        // false/next label will be deleted.
        auto loop = loop_searcher.get_inner_loop(secBrBB);
        auto base = loop_searcher.get_loop_base(loop);
        auto pre_num = secBrBB->get_pre_basic_blocks().size();
        bbs->remove_succ_basic_block(secBrBB);
        secBrBB->remove_pre_basic_block(bbs);

        if (pre_num == 1 || (pre_num == 2 && base == secBrBB))
        {
            delete_bb = true;
            // secBB is falseBB, not the nextBB, delete.
            deleteBlockSet.insert(secBrBB);
            for (auto succ : secBrBB->get_succ_basic_blocks())
            {
                succ->remove_pre_basic_block(secBrBB);
            }
        }
        // whatever the secBB is falseBB or nextBB, we need to delete the chain of the succ and pre.
        static_cast<BranchInst *>(terminator)->create_br(firBrBB, bbs); // 这条语句已经加过 br firstbb 了，不需要再加
        bbs->delete_instr(terminator);
    }
    else
    {
        // trueBB will be deleted.
        delete_bb = true;
        deleteBlockSet.insert(firBrBB);
        bbs->remove_succ_basic_block(firBrBB);
        firBrBB->remove_pre_basic_block(bbs);
        for (auto succ : firBrBB->get_succ_basic_blocks())
        {
            succ->remove_pre_basic_block(firBrBB);
        }
        auto newBrInstr = static_cast<BranchInst *>(terminator)->create_br(secBrBB, bbs);
        bbs->delete_instr(terminator);
        // bbs->add_instruction(newBrInstr);
    }
    ```

- 在下一次遍历 func 中的 bb 块时，跳过在 deleteBlockSet 中的块。

- 注意考虑无用分支中嵌套循环的情况：

如果一个 bb 块没有前驱，或者有一个前驱，但这个前驱来自循环内部，即，该 bb 块为一个循环的入口块。这样的块必然在无用分支内部，同样需要删除。

  ```cpp
  if ((bbs->get_pre_basic_blocks().size() == 0) && (bbs->get_name() != static_cast<std::string>("label_entry")))
  {
      // bbs's prev is null
      delete_bb = true;
      deleteBlockSet.insert(bbs);
      changed = true;
      for (auto succ : bbs->get_succ_basic_blocks())
      {
          succ->remove_pre_basic_block(bbs);
      }
      continue;
  }
  ```



  ```cpp
  if ((bbs->get_pre_basic_blocks().size() == 1))
  {
      // 判断这个前驱是不是在循环里的
      auto loop = loop_searcher.get_inner_loop(bbs);
      auto base = loop_searcher.get_loop_base(loop);
      if (base == bbs)
      {
          // bbs 是循环的入口块，且循环入口只有一个前驱，这个前驱必然是属于循环内部的
          for (auto bb = loop->begin(); bb != loop->end(); bb++)
          {
              deleteBlockSet.insert((*bb));
              for (auto succ : (*bb)->get_succ_basic_blocks())
              {
                  succ->remove_pre_basic_block((*bb));
              }
          }
          continue;
      }
  }
  ```



#### 优化结果

源代码：

```cpp
int sub(int a, int b)
{
    return a - b;
}

void main(void)
{
    int i;
    int j;
    int temp;
    i = 1;
    j = 2;
    if (i < 0)
    {
        j = 7;
    }

    else{
        while (j > 1)
        {
            j = 0;
        }
    }
    temp = sub(i, j);
    output(temp);
    return;
}
```

优化前：

```
define i32 @sub(i32 %arg0, i32 %arg1) {
label_entry:
  %op6 = sub i32 %arg0, %arg1
  ret i32 %op6
}
define void @main() {
label_entry:
  %op4 = icmp slt i32 1, 0
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label8
label7:                                                ; preds = %label_entry
  br label %label9
label8:                                                ; preds = %label_entry
  br label %label14
label9:                                                ; preds = %label7, %label20
  %op21 = phi i32 [ 7, %label7 ], [ %op22, %label20 ]
  %op12 = call i32 @sub(i32 1, i32 %op21)
  call void @output(i32 %op12)
  ret void
label14:                                                ; preds = %label8, %label19
  %op22 = phi i32 [ 2, %label8 ], [ 0, %label19 ]
  %op16 = icmp sgt i32 %op22, 1
  %op17 = zext i1 %op16 to i32
  %op18 = icmp ne i32 %op17, 0
  br i1 %op18, label %label19, label %label20
label19:                                                ; preds = %label14
  br label %label14
label20:                                                ; preds = %label14
  br label %label9
}
```

优化后：

```
define i32 @sub(i32 %arg0, i32 %arg1) {
label_entry:
  %op6 = sub i32 %arg0, %arg1
  ret i32 %op6
}
define void @main() {
label_entry:
  br label %label8
label8:                                                ; preds = %label_entry%label_entry
  br label %label14
label9:                                                ; preds = %label20
  %op21 = phi i32 [ %op22, %label20 ]
  %op12 = call i32 @sub(i32 1, i32 %op21)
  call void @output(i32 %op12)
  ret void
label14:                                                ; preds = %label8, %label19
  %op22 = phi i32 [ 2, %label8 ], [ 0, %label19 ]
  %op16 = icmp sgt i32 %op22, 1
  %op17 = zext i1 %op16 to i32
  %op18 = icmp ne i32 %op17, 0
  br i1 %op18, label %label19, label %label20
label19:                                                ; preds = %label14
  br label %label14
label20:                                                ; preds = %label14
  br label %label9
}
```

可以看出，经过复写传播和常量合并以及删除无用分支之后，label_entry 中只剩一条直接跳转语句，原先的 label7 对应 trueBB 已经被删除。

同样被删除的还有 label9 的前驱节点，由 `preds = %label7, %label20`，变为 `preds = %label20`。





### 循环不变式外提

目标：实现将与循环无关的表达式提取到循环外；

注意：不用考虑数组，也不需要考虑全局变量；

#### 1. 如何寻找最内层循环？

从循环的嵌套关系入手，如果当前循环 `loop` 的上层循环  `parent_loop` 不为空，则说明，`parent_loop` 不是最内层循环。

我们可以建立一个 bool 类型的数组（map）来维护上述关系。

```cpp
for (auto cloop : loops_in_func)
{
    auto ploop = loop_searcher.get_parent_loop(cloop);
    if (ploop != nullptr)
    {
        auto base = loop_searcher.get_loop_base(ploop);
        loop2child[base] = false;
    } // ploop:外一层循环
}
```



#### 2. 判断语句是否与循环无关

`s:x=y+z`

y 和 z 的定值都在循环外部或者为常数；

具体实现思路：考虑 y ，z 在循环内部语句块中是否出现在左值部分；

1. 构建` std::unordered_set<Instruction *> define_set`: 遍历循环内部的每一条 instruction，将循环左值插入到 define_set 集合中；

2. 构建`std::queue<Instruction *> wait_move` ：再次遍历 instruction（跳过不在 define_set 中的指令，因为这些指令已经不再存在于该循环中），判断右侧参数是否出现在 define_set 中，若两个都不在，则该 instruction 与循环无关，将这条与循环无关的指令插入队列 wait_move 中，表示该语句需要外提，标志变量 `move = true;changed = ture;`

3. 同时，在 define_set 中删除这条与循环无关的语句，转 2；若遍历过循环中现存的所有语句之后，发现 `changed = false;` 转 4；

    ```cpp
    // 在循环内部删掉不变式
    if (move)
    {
    #ifdef DEBUG
        printf("move this insturction\n");
    #endif
        changed = true;
        wait_move.push(instr);
        define_set.erase(instr);
        // (*bb)->delete_instr(instr);
    }
    ```

4. 遍历 外一层 / 并行的下一个 循环，转 1；

#### 3. 判断语句符合代码外提条件

有一些语句是必然不能外提的，他们是：load,store（全局变量不用考虑）branch,ret,phi,call（分支，跳转，phi 选择函数和调用函数的语句都不能外提，否则会引起程序控制流混乱）；

```cpp
if (instr->is_load() || instr->is_store() || instr->is_br() || instr->is_phi() || instr->is_call() || instr->is_ret())
{ // 这些语句必不能外提
#ifdef DEBUG
    printf("is load|store|br\n");
#endif
    move = false;
    continue;
}
```

#### 4. 不变式应该外提到哪个前驱？如何插入到这个前驱中？

1. 循环的条件块（base）最多只有两个前驱，需要把不变式提前到不属于本循环的那个前驱块中。

    ```cpp
    // 循环不变式外提到前驱
    auto base = loop_searcher.get_loop_base(inner_loop);
    for (auto prev : base->get_pre_basic_blocks())
    {
        if (inner_loop->find(prev) == inner_loop->end())
        {
            // 将 wait_move 中的语句逐行插入 prev 中
        }
    }

    ```



2. 每个 bb 块的最后一条语句都是跳转指令 `bb->get_terminator() 获取`，我们应该在这条跳转指令之前按序插入 wait_move 中的指令。

    ```cpp
    auto br = prev -> get_terminator(); // 首先获取 br
    prev->delete_instr(br); // 先删除这条 br，别着急，我们后面会再加上他
    while (wait_move.size() > 0) // 现在可以插入 wait_move 中的语句啦
    {
    #ifdef DEBUG
        printf("insert one instruction\n");
    #endif
        auto move_ins = wait_move.front();
        prev->add_instruction(move_ins);
        l_val2bb_set[move_ins]->delete_instr(move_ins);
        wait_move.pop();
    }
    prev->add_instruction(br); // 补上之前删掉的 br
    // 代码外提就完成啦~
    ```




#### 优化效果

源代码：

```c
int x;

void main(void)
{
    int i;
    int j;
    int ret;

    i = 1;

    while (i < 10000)
    {
        j = 0;
        x = j;
        while (j < 10000)
        {
            ret = (i*i*i*i)/i/i/i;
            j = j + 1;
        }
        i = i + 1;
    }
    output(ret);
    return;
}

```

优化之前：

```
define void @main() {
label_entry:
  br label %label3
label3:                                                ; preds = %label_entry, %label33
  %op36 = phi i32 [ %op39, %label33 ], [ undef, %label_entry ]
  %op37 = phi i32 [ 1, %label_entry ], [ %op35, %label33 ]
  %op38 = phi i32 [ %op40, %label33 ], [ undef, %label_entry ]
  %op5 = icmp slt i32 %op37, 10000
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label10
label8:                                                ; preds = %label3
  store i32 0, i32* @x
  br label %label12
label10:                                                ; preds = %label3
  call void @output(i32 %op36)
  ret void
label12:                                                ; preds = %label8, %label17
  %op39 = phi i32 [ %op36, %label8 ], [ %op30, %label17 ]
  %op40 = phi i32 [ 0, %label8 ], [ %op32, %label17 ]
  %op14 = icmp slt i32 %op40, 10000
  %op15 = zext i1 %op14 to i32
  %op16 = icmp ne i32 %op15, 0
  br i1 %op16, label %label17, label %label33
label17:                                                ; preds = %label12
  %op20 = mul i32 %op37, %op37
  %op22 = mul i32 %op20, %op37
  %op24 = mul i32 %op22, %op37
  %op26 = sdiv i32 %op24, %op37
  %op28 = sdiv i32 %op26, %op37
  %op30 = sdiv i32 %op28, %op37
  %op32 = add i32 %op40, 1
  br label %label12
label33:                                                ; preds = %label12
  %op35 = add i32 %op37, 1
  br label %label3
}

```

优化之后：

```
define void @main() {
label_entry:
  br label %label3
label3:                                                ; preds = %label_entry, %label33
  %op36 = phi i32 [ %op39, %label33 ], [ undef, %label_entry ]
  %op37 = phi i32 [ 1, %label_entry ], [ %op35, %label33 ]
  %op38 = phi i32 [ %op40, %label33 ], [ undef, %label_entry ]
  %op5 = icmp slt i32 %op37, 10000
  %op6 = zext i1 %op5 to i32
  %op7 = icmp ne i32 %op6, 0
  br i1 %op7, label %label8, label %label10
label8:                                                ; preds = %label3
  store i32 0, i32* @x
  %op20 = mul i32 %op37, %op37
  %op22 = mul i32 %op20, %op37
  %op24 = mul i32 %op22, %op37
  %op26 = sdiv i32 %op24, %op37
  %op28 = sdiv i32 %op26, %op37
  %op30 = sdiv i32 %op28, %op37
  br label %label12
label10:                                                ; preds = %label3
  call void @output(i32 %op36)
  ret void
label12:                                                ; preds = %label8, %label17
  %op39 = phi i32 [ %op36, %label8 ], [ %op30, %label17 ]
  %op40 = phi i32 [ 0, %label8 ], [ %op32, %label17 ]
  %op14 = icmp slt i32 %op40, 10000
  %op15 = zext i1 %op14 to i32
  %op16 = icmp ne i32 %op15, 0
  br i1 %op16, label %label17, label %label33
label17:                                                ; preds = %label12
  %op32 = add i32 %op40, 1
  br label %label12
label33:                                                ; preds = %label12
  %op35 = add i32 %op37, 1
  br label %label3
}
```

可以看出，循环不变式外提优化之后，将原来 labl17 （最内层循环）中对变量 `ret` 的计算语句（ 变量`i` 的值在最内层循环中始终不变）外提到了 label8 （上一层循环），减少了 10000 次冗余的计算。





###  活跃变量分析

#### part1 计算每个Block的`def`和`use`集合，并将其与对应的BasicBlock一起存下来

##### part1.1 选择合适的数据结构

首先我们需要计算每个` BasicBlock`的`def`和`use`集合。由于`def`和`use`里面的元素不能重复，自然想到用`std::set`对其存储。因为每个BasicBlock都有属于自己的`def`和`use`。在这里，我选择使用map结构对其分别进行存储。在`ActiveVars.hpp`文件中，我用声明语句`std::map<BasicBlock *, std::set<Value *>> map_use, map_def;`定义两个map结构。而big_phi则是专门为phi函数准备的，在part1.4中会有详细说明。这样前期工作就完成了

```cpp
class ActiveVars : public Pass
{
public:
    ActiveVars(Module *m) : Pass(m) {}
    void run();
    std::string print();
    std::string use_def_print();

private:
    Function *func_;
    std::map<BasicBlock *, std::set<Value *>> live_in, live_out;
    std::map<BasicBlock *, std::set<Value *>> map_use, map_def;
    std::map<BasicBlock *, std::map<Value *, BasicBlock *>> big_phi;
};
```

##### part1.2 对每个Block的指令进行遍历,并以此计算`def`和`use`

需要注意到的是，我们需要根据指令类型的不同来规定取操作数行为的不同

比如像binary指令，它有两个右操作数和一个左值，两个右操作数可以用`get_operand(i)`来取出，左值直接用指令本身即可。其他不同的指令有不同的操作方式

对于每一个要插入use集合的指令，我们都需要首先看他是否在该块def集合中已经有定义，可以通过`_def.find(instr->get_operand(i)) == _def.end()`来判断

同理，对于每一个要插入def集合的指令，我们都需要首先看他是否在该块use集合中已经有定义，可以通过`_use.find(instr->get_operand(i)) == _use.end()`来判断

还需要注意插入的操作数是否是常数，这个条件可以用dynamic_cast实现

`dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr`

下面以binary指令为例，附上计算def和use的代码

```cpp
 for (auto bb : func_->get_basic_blocks())
            {
                for (auto instr: bb->get_instructions())
                {
                    if(instr->isBinary() || instr->is_fcmp() || instr->is_cmp() )//双目运算符
                    {
                        if (dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr && _def.find(instr->get_operand(0)) == _def.end()) //first value is a var
                        {
							 _use.insert(instr->get_operand(0));
                        }
                        if (dynamic_cast<ConstantInt *>(instr->get_operand(1)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(1)) == nullptr && _def.find(instr->get_operand(1)) == _def.end()) //second value is a var
                        {
                            _use.insert(instr->get_operand(1));
                        }
                        //check left value
                        if (_use.find(instr) == _use.end())//not find
                        {
                            _def.insert(instr);
                        }
                    }
```

##### part1.3 将def和use两个集合与对应block一起存入map

上面代码中使用的_use和\_def只是一个中间变量，作为临时存储的set而存在，最后我们需要将其值和对应block一起存入map，并将\_use和\_def清空，以准备下一个新block的分析

```cpp
map_def.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, _def));
map_use.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, _use));
_def.clear();
_use.clear();
```

##### part 1.4 额外说明，关于phi函数

phi函数需要记录的东西比较多，有：每个操作数和它的来源（即来自phi函数所属块的哪个直接前驱，在phi指令中以label形式出现），以及这个phi函数所属的块（方便后续计算一些块的OUT[BB]，后续part2会有解释)，这些信息都会在big_phi中存储下来

```cpp
else if (instr->is_phi())
                    {
                        for (int i = 0; i < instr->get_num_operand()/2; i++)
                        {
                            if(_def.find(instr->get_operand(2*i)) == _def.end())
                            {
                                if (dynamic_cast<ConstantInt *>(instr->get_operand(2 * i)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(2*i)) == nullptr)
                                {
                                    _use.insert(instr->get_operand(2 * i));

                                    big_phi[bb].insert({instr->get_operand(2 * i), dynamic_cast<BasicBlock *>(instr->get_operand(2 * i + 1))});
                                }
                            }
                        }
                        if (_use.find(instr) == _use.end())
                        {
                            _def.insert(instr);
                        }
                    }
```

#### part2 根据use和def计算每个块的块头活跃变量集合和块尾活跃变量集合

思路在设计难点中以及提到过，对于 phi 函数，需要对数据流方程做一些修改，下面是具体实现部分：

```cpp
 for (auto succ_bb : bb->get_succ_basic_blocks())
                    {

                        auto INS = live_in.find(succ_bb);
                        std::set<Value *> old_ins;
                        while (judge_set(INS->second, old_ins) == false)
                        {
                            old_ins = INS->second;
                            for (auto item : INS->second)
                            {
                                if (big_phi.find(succ_bb) != big_phi.end())//phi instr exist
                                {
                                    if (big_phi.find(succ_bb)->second.find(item) != big_phi.find(succ_bb)->second.end()) //there is a phi instr containing item
                                    {
                                        //now check if bb matches
                                        if (big_phi.find(succ_bb)->second.find(item)->second->get_name() == bb->get_name())
                                        {
                                            union_ins.insert(item);
                                        }
                                    }
                                    else
                                    {
                                            union_ins.insert(item);
                                    }
                                }
                                else//no phi instr
                                {
                                            union_ins.insert(item);
                                }
                            }
                        }
                    }
                    if(live_out.find(bb) == live_out.end())
                    {
                        live_out.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, union_ins));
                    }
                    else live_out.find(bb)->second = union_ins;
```

IN[BB]按照公式计算就好，但是为了检测IN集合是否发生变化，每一次计算IN之前，我们都会用一个中间变量`in_reserve`将其保存起来，然后将新的IN集合跟其比较，如果发生变化，则将bool型变量`is_change`赋值为true

```cpp
temp_outb = live_out.find(bb)->second;
                    // std::cout << "segfault2" << std::endl;
intersection_set(temp_outb, map_def.find(bb)->second);
                    // std::cout << "segfault2.5" << std::endl;
in_reserve = live_in.find(bb)->second;
                    // std::cout << "segfault3" << std::endl;
union_set_reserve(live_in.find(bb)->second, map_use.find(bb)->second, temp_outb);
                    // std::cout << "segfault4" << std::endl;
if (judge_set(in_reserve,live_in.find(bb)->second) == false)//change
                    {
                        is_change = true;
                    }

```

函数`intersection_set(a,b)`的作用是让集合a减去集合b

函数`union_set_reserve(a,b,c)`的作用是让集合b与集合c作并集，存到集合a中

函数`judge_set(a,b)`判断集合a，b是否相等，相等返回true，否则返回false

至此，关于活跃变量分析的工作就完成了。

### 实验总结

tzl：本次实验加深了我对CPP STL的认识，让我更加熟练地去使用诸如set和map等数据结构。还提升了我的debug能力，磨练了我的耐性。同时让我对活跃变量分析这一知识点更加熟悉。

lr：加深对课堂上所学内容的理解，了解到在实际应用当中循环不变式的判定标准和外提的意义。逐渐深入的了解到控制流图的实际跳转方式，并能够在此基础上开发基本的优化。学会了如何使用 `#define DEUBG` 来在程序中增加输出语句调试程序。初步了解了一些 c++ stl 容器的用法。经过两次团队实验，对团队协作的方式和队员间的交流有了一定的认知，总的来说，收获颇丰。

ayc：一开始写的时候觉得常量传播时很简单的，可是真正付诸实践的时候却发现无论是逻辑描述上还是代码实现上，都存在着很多难点。不过在经历了四天的debug和讨论之后，我也对C++的面向对象用法和STL有了更深入的认识。<br/>P.S.感谢同组的两位，在我debug和构建思路的时候给了我很多帮助😊



### 实验反馈 （可选 不会评分）



对本次实验的建议



### 组间交流 （可选）



本次实验和哪些组（记录组长学号）交流了哪一部分信息1