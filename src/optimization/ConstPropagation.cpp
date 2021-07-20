#include "ConstPropagation.hpp"
#include "logging.hpp"
#include <cmath>
#include "LoopSearch.hpp"

#define IS_GLOBAL_VARIABLE(l_val) dynamic_cast<GlobalVariable *>(l_val)
#define IS_GEP_INSTR(l_val) dynamic_cast<GetElementPtrInst *>(l_val)

/**
 * This Hash map stores the variables that can be compressed to a single constant
 * The 'key' region is
 * I would use those values to replace the original instance of that variable
 **/
#define DEBUG 1

std::map<Value *, Value *> const_val_stack;

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2)
{

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::add:
        return ConstantInt::get(c_value1 + c_value2, module_);

        break;
    case Instruction::sub:
        return ConstantInt::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantFP *ConstFolder::computefp(
    Instruction::OpID op,
    ConstantFP *value1,
    ConstantFP *value2)
{

    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op)
    {
    case Instruction::fadd:
        return ConstantFP::get(c_value1 + c_value2, module_);

        break;
    case Instruction::fsub:
        return ConstantFP::get(c_value1 - c_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFP::get(c_value1 * c_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFP::get((float)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 获取整数型比较指令的返回值 ->i32类型
ConstantInt *ConstFolder::compute_comp(
    CmpInst::CmpOp op,
    ConstantInt *value1,
    ConstantInt *value2)
{
    // EQ , ==
    // NE, !=
    // GT, >
    // GE, >=
    // LT, <
    // LE, <=
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
    case CmpInst::EQ:
        return ConstantInt::get(c_value1 == c_value2, module_);
        break;
    case CmpInst::NE:
        return ConstantInt::get(c_value1 != c_value2, module_);
        break;
    case CmpInst::GT:
        return ConstantInt::get(c_value1 > c_value2, module_);
        break;
    case CmpInst::GE:
        return ConstantInt::get(c_value1 >= c_value2, module_);
        break;
    case CmpInst::LT:
        return ConstantInt::get(c_value1 < c_value2, module_);
        break;
    case CmpInst::LE:
        return ConstantInt::get(c_value1 <= c_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 获取浮点类型比较指令的返回值 ->i32类型
ConstantInt *ConstFolder::compute_fcomp(
    FCmpInst::CmpOp op,
    ConstantFP *value1,
    ConstantFP *value2)
{
    // EQ , ==
    // NE, !=
    // GT, >
    // GE, >=
    // LT, <
    // LE, <=
    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op)
    {
    case CmpInst::EQ:
        return ConstantInt::get((abs(c_value1 - c_value2) < 1e-6), module_);
        break;
    case CmpInst::NE:
        return ConstantInt::get((abs(c_value1 - c_value2) >= 1e-6), module_);
        break;
    case CmpInst::GT:
        return ConstantInt::get(c_value1 > c_value2, module_);
        break;
    case CmpInst::GE:
        return ConstantInt::get(c_value1 >= c_value2, module_);
        break;
    case CmpInst::LT:
        return ConstantInt::get(c_value1 < c_value2, module_);
        break;
    case CmpInst::LE:
        return ConstantInt::get(c_value1 <= c_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
}

void ConstPropagation::run()
{
    // 从这里开始吧！
    auto CF = ConstFolder(m_);
    // 先通过 LoopSearch 获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();
    // while (flag)
    // do {
    //     changed = false;
    for (auto func : m_->get_functions())
    {
        if (func->get_basic_blocks().size() == 0)
        {
            continue;
        }
        else
        {
            std::set<BasicBlock *> deleteBlockSet;
            bool changed = false; // to show whether need to take constant action
            bool delete_bb = false;
            do
            {
                changed = false; // to show whether need to take constant action
                delete_bb = false;

                for (auto bbs : func->get_basic_blocks())
                {
#ifdef DEBUG
                    printf("for in bbs\n");
#endif
                    if (deleteBlockSet.find(bbs) != deleteBlockSet.end())
                    {
                        // bbs in deleteBlockset
#ifdef DEBUG
                        printf("bbs in deleteBlockset\n");
#endif
                        continue;
                    }

                    if ((bbs->get_pre_basic_blocks().size() == 1))
                    {
                        // 判断这个前驱是不是在循环里的
                        auto loop = loop_searcher.get_inner_loop(bbs);
                        auto base = loop_searcher.get_loop_base(loop);
                        if (base == bbs)
                        {
#ifdef DEBUG
                            printf("------bbs is the loop base and we need to delete this loop------\n");
#endif
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

                    if ((bbs->get_pre_basic_blocks().size() == 0) && (bbs->get_name() != static_cast<std::string>("label_entry")))
                    {
                        // bbs's prev is null
#ifdef DEBUG
                        printf("bbs's prev is null\n");
#endif
                        delete_bb = true;
                        deleteBlockSet.insert(bbs);
                        changed = true;
                        for (auto succ : bbs->get_succ_basic_blocks())
                        {
                            // bbs->remove_succ_basic_block(succ);
                            succ->remove_pre_basic_block(bbs);
                        }
                        continue;
                    }
                    /** NOTE:
                     * When we dealing with those unused BasicBlocks, we can figure them out by two ways
                     * The 3-operands br instruction:
                     * * If the condition field is a constant,
                     * * we need to cut off the useless branch off and change the br instruction
                     *
                     * The orphan bb:
                     * * We call the bbs who do not have even a single prev block 'orphans'
                     * * Suppose we are looking through the bbs in an function,
                     * * when we find out that a bb's prevNum == 0
                     * * we push this block into deleteSet, while cutting it off from its succs
                     * * Actually, we only need to push into deleteSet when we are visiting them
                     *
                     * Besides, we need to do the ConstPropagation work on the \Phi instruction
                     * * after we have put all blocks that need to delete into the deleteSet,
                     * * we can search the blocks one more time, to find out those phi instrs
                     **/
                    std::vector<Instruction *> wait_delete;
                    /** NOTE:
                     * Global Vars
                     * set up a const_set locally, push when Global Var gets a store instr
                     **/
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

                        else if (instr->is_call())
                        {
                            continue;
                        }
                        else if (instr->is_phi())
                        {
#ifdef DEBUG
                            printf("----------the instruction is phi-----the number of operands:-----\n");
#endif
#ifdef DEBUG
                            std::cout << instr->get_num_operand() << std::endl;
#endif

                            if (instr->get_num_operand() == 2)
                            {
                                // phi 只剩一个参数
#ifdef DEBUG
                                printf("replace phi\n");
#endif
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
#ifdef DEBUG
                                        printf("the brFromBB is dead, we will remove the operands,the new num of operands is:\n");
#endif
                                        instr->remove_operands(2 * i, 2 * i + 1);
                                        changed = true;
#ifdef DEBUG
                                        std::cout << instr->get_num_operand() << std::endl;
#endif
                                    }
                                }
                            }
                        }
                        /** TODO:
                         * Do the const propagation work
                         * get the operand list of that instruction
                         * search through the const_val_stack for each operand
                         * if hit, change it
                         **/
                        /** FIXME:
                         * maybe the instr->replace_all_use_with(constValue) can do this all work?
                         **/
                        else if (instr->isBinary())
                        // only do the const work on binary instructions
                        {
#ifdef DEBUG
                            printf("----------the ins is binary----------\n");
#endif
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
                        else if ((instr->is_fp2si()) || (instr->is_si2fp()) || (instr->is_zext()))
                        /** NOTE:
                         * Besides, we need to deal with Type-Cast instructions
                         * Since there is no static_cast or dynamic_cast between 'Type's,
                         * we need to do that convertion manually.
                         * We pull the real number out and create a new Value with the method .get()
                         **/
                        {
                            if (instr->is_fp2si())
                            {
#ifdef DEBUG
                                printf("----------the ins is float to int----------\n");
#endif
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
#ifdef DEBUG
                                printf("----------the ins is int to float----------\n");
#endif
                                auto intValue = cast_constantint(instr->get_operand(0));
                                if (intValue != nullptr)
                                {
                                    auto constIntValue = intValue->get_value();
                                    auto fpValue = static_cast<float>(constIntValue);
                                    auto constFp = ConstantFP::get(fpValue, m_);
                                    const_val_stack.insert(std::pair<Value *, Value *>(instr, constFp));
                                    instr->replace_all_use_with(constFp);
                                    wait_delete.push_back(instr);
                                    changed = true;
                                }
                            }
                            else
                            {
#ifdef DEBUG
                                printf("----------the ins is zext----------\n");
#endif
                                auto boolValue = cast_constantint(instr->get_operand(0));

                                if (boolValue != nullptr)
                                {
#ifdef DEBUG
                                    printf("get bool value success!\n");
#endif
                                    auto constBoolValue = boolValue->get_value();
                                    auto intValue = static_cast<int>(constBoolValue);
#ifdef DEBUG
                                    printf("the boolValue is %d\n", intValue);
#endif
                                    auto constInt = ConstantInt::get(intValue, m_);
                                    const_val_stack.insert(std::pair<Value *, Value *>(instr, constInt));
                                    instr->replace_all_use_with(constInt);
                                    wait_delete.push_back(instr);
                                    changed = true;
                                }
                            }
                        }
                        /** NOTE:
                         * For the reason of dead block deletion, we need to deal with the constant folding
                         * with the comparation instructions
                         **/
                        else if ((instr->is_cmp()) || (instr->is_fcmp()))
                        {
                            auto cmpA = instr->get_operand(0);
                            auto cmpB = instr->get_operand(1);
                            ConstantInt *cmpResult;
                            if (instr->is_cmp())
                            {
#ifdef DEBUG
                                printf("----------the ins is int compare----------\n");
#endif
                                auto constCmpA = cast_constantint(cmpA);
                                auto constCmpB = cast_constantint(cmpB);
                                if ((constCmpA != nullptr) && (constCmpB != nullptr))
                                {
#ifdef DEBUG
                                    printf("constant compare\n");
#endif
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
#ifdef DEBUG
                                printf("----------the ins is float compare----------\n");
#endif
                                auto constCmpA = cast_constantfp(cmpA);
                                auto constCmpB = cast_constantfp(cmpB);
                                if ((constCmpA != nullptr) && (constCmpB != nullptr))
                                {
#ifdef DEBUG
                                    printf("constant compare\n");
#endif
                                    auto cmpOp = dynamic_cast<FCmpInst *>(instr)->get_cmp_op();
                                    cmpResult = CF.compute_fcomp(cmpOp, constCmpA, constCmpB);
                                    const_val_stack.insert({instr, cmpResult});
                                    instr->replace_all_use_with(cmpResult);
                                    wait_delete.push_back(instr);
                                    changed = true;
                                }
                            }
                        }
                    } // for instructions in bbs

                    for (auto instr : wait_delete)
                    {
                        bbs->delete_instr(instr);
                    }
                    // wait_delete.clear();
                    // #ifdef DEBUG
                    //                     printf("----------now we begin to look branch instruction ! ----------\n");
                    // #endif
                    /** NOTE: delete the unused blocks
                         * A global set, maintaining the unvisited bbs
                         * When we visit the BasicBlocks logically, from the ENTRY bb
                         * we use the rule above to find those unused
                         * When we finding the redundant block, we should find the `br` instruction first
                         * Actually we only need to remove 'prev' pointer
                         **/
                    // auto terminator = dynamic_cast<BranchInst *>(bbs->get_terminator());
                    auto terminator = bbs->get_terminator();
                    if (terminator->is_br())
                    {
#ifdef DEBUG
                        printf("----------get the br----------\n");
#endif
                        if (terminator->get_num_operand() == 3)
                        // equals to terminator == cond_br
                        {
#ifdef DEBUG
                            printf("----- cond br----\n");
#endif
                            auto condition = terminator->get_operand(0);
                            auto condValue = cast_constantint(condition);
                            if (condValue != nullptr)
                            {
                                auto firBrBB = static_cast<BasicBlock *>(terminator->get_operand(1));
                                auto secBrBB = static_cast<BasicBlock *>(terminator->get_operand(2));
                                auto cond = condValue->get_value();
                                if (cond > 0)
                                {

                                    auto loop = loop_searcher.get_inner_loop(secBrBB);
                                    auto base = loop_searcher.get_loop_base(loop);
                                    auto pre_num = secBrBB->get_pre_basic_blocks().size();
#ifdef DEBUG
                                    printf("find the cond is %d, maybe we need delete secBB...\n", pre_num);
#endif
                                    bbs->remove_succ_basic_block(secBrBB);
                                    secBrBB->remove_pre_basic_block(bbs);

                                    if (pre_num == 1 || (pre_num == 2 && base == secBrBB))
                                    {
                                        delete_bb = true;
#ifdef DEBUG
                                        printf("----------find the dead block!----------\n");
#endif
                                        // secBB is falseBB, not the nextBB, delete.
                                        deleteBlockSet.insert(secBrBB);
                                        for (auto succ : secBrBB->get_succ_basic_blocks())
                                        {
#ifdef DEBUG
                                            printf("cut the secBrBB's succ\n");
#endif
                                            // secBrBB->remove_succ_basic_block(succ);
                                            succ->remove_pre_basic_block(secBrBB);
                                        }
#ifdef DEBUG
                                        printf("delete the old secBB branch and insert the new branch\n");
#endif
                                    }
                                    static_cast<BranchInst *>(terminator)->create_br(firBrBB, bbs); // 这条语句已经加过 br firstbb 了，不需要再加
                                    bbs->delete_instr(terminator);
                                }
                                else
                                {
                                    delete_bb = true;
                                    deleteBlockSet.insert(firBrBB);

                                    bbs->remove_succ_basic_block(firBrBB);
                                    firBrBB->remove_pre_basic_block(bbs);
                                    for (auto succ : firBrBB->get_succ_basic_blocks())
                                    {
#ifdef DEBUG
                                        printf("cut the firBrBB's succ\n");
#endif
                                        // firBrBB->remove_succ_basic_block(succ);
                                        succ->remove_pre_basic_block(firBrBB);
                                    }
                                    auto newBrInstr = static_cast<BranchInst *>(terminator)->create_br(secBrBB, bbs);
                                    bbs->delete_instr(terminator);
#ifdef DEBUG
                                    printf("delete the old first branch and insert the new branch\n");
#endif
                                    // bbs->add_instruction(newBrInstr);
                                }
                            }
                        }
                    } // if terminator is cond br

                } // for bbs in function
            } while (changed || delete_bb);

            for (auto delbb : deleteBlockSet)
            {
                func->remove(delbb);
            }
        }
    }
    // } while(changed);
}
