#include <algorithm>
#include <iostream>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"
#include <stack>
#include <queue>

#define IS_GLOBAL_VARIABLE(l_val) dynamic_cast<GlobalVariable *>(l_val)
#define IS_GEP_INSTR(l_val) dynamic_cast<GetElementPtrInst *>(l_val)

// #define DEBUG 1

void LoopInvHoist::run()
{
#ifdef DEBUG
    std::cout << "-----循环不变式外提----\n";
#endif
    // 先通过 LoopSearch 获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();

    auto func_list = m_->get_functions();
    for (auto func : func_list)
    {
    #ifdef DEBUG
        printf("auto func\n");
    #endif
        // 遍历每一个函数
        if (func->get_basic_blocks().size() == 0)
        {
            continue;
        }
        else
        {
            //从函数中的最内层循环开始找可以外提的循环不变式
            #ifdef DEBUG
                printf("the num of bb in func = %d\n",func->get_basic_blocks().size());
            #endif

            std::unordered_set<BBset_t *> loops_in_func = loop_searcher.get_loops_in_func(func);
            std::unordered_map<BasicBlock *, bool> loop2child;

            for (auto cloop : loops_in_func)
            {
                auto base = loop_searcher.get_loop_base(cloop);
                loop2child.insert({base, true});
            }
            #ifdef DEBUG
                printf("get all loops in func\n");
            #endif
            for (auto cloop : loops_in_func)
            {
                auto ploop = loop_searcher.get_parent_loop(cloop);
                if (ploop != nullptr)
                {
                    auto base = loop_searcher.get_loop_base(ploop);
                    loop2child[base] = false;
                } // ploop:外一层循环
            }

            while (loops_in_func.size())
            {
                #ifdef DEBUG
                    printf("the curr num of loop in func = %d\n",loops_in_func.size());
                #endif
                // 函数中存在未分析的循环
                BBset_t *inner_loop = nullptr;
                for (auto cloop : loops_in_func)
                {
                    auto base = loop_searcher.get_loop_base(cloop);
                    if (loop2child[base] == true)
                    {
                        // cloop 为最小循环
                        #ifdef DEBUG
                            printf("get the inner_loop\n");
                        #endif

                        inner_loop = cloop;
                        break;
                    }
                }
                // 此时 inner_loop 是当前未分析的最小循环
                while (inner_loop != nullptr)
                {
                    #ifdef DEBUG
                    printf("traverse the inner_loop\n");
                    #endif
                    std::unordered_set<Value *> define_set;
                    std::queue<Instruction *> wait_move;
                    // std::unordered_map<Value *, Instruction *> l_val2instr_set;
                    std::unordered_map<Instruction *, BasicBlock *> l_val2bb_set;

                    for (auto bb = inner_loop->begin(); bb != inner_loop->end(); bb++)
                    {
                        // 遍历 inner 循环中的每一条语句
                        for (auto instr : (*bb)->get_instructions())
                        {
                            define_set.insert(instr);
                            // l_val2instr_set.insert({instr, instr}); // 构建左值 - 指令
                            l_val2bb_set.insert({instr, (*bb)}); // 指令--指令所在的 basicblock
                        }
                    }
                    // 分析指令参数是否与循环无关, 找出循环不变式
                    #ifdef DEBUG
                    printf("get the define_set\n");
                    #endif
                    bool changed = false;
                    do
                    {
                        changed = false;
                        for (auto bb = inner_loop->begin(); bb != inner_loop->end(); bb++)
                        {
                            // 遍历 inner 循环中的每一条语句
                            for (auto instr : (*bb)->get_instructions())
                            {
                                if(define_set.find(instr)==define_set.end()){
                                    continue;
                                }
                                bool move = true;
                                #ifdef DEBUG
                                printf("traverse the insturction\n");
                                #endif
                                if (instr->is_load() || instr->is_store() || instr->is_br() || instr->is_phi() || instr->is_call()||instr->is_ret())
                                { // call 语句必不能外提
                                    #ifdef DEBUG
                                    printf("is load|store|br\n");
                                    #endif
                                    move = false;
                                    continue;
                                }
                                else
                                {
                                    #ifdef DEBUG
                                    printf("is other instruction\n");
                                    #endif
                                    for (int i = 0; i < instr->get_num_operand(); i++)
                                    {
                                        auto arg = instr->get_operand(i);
                                        #ifdef DEBUG
                                        printf("print arg with other ins\n");
                                        std::cout << arg->print() << std::endl;
                                        #endif
                                        if (define_set.find(arg) != define_set.end()||IS_GEP_INSTR(arg)||IS_GLOBAL_VARIABLE(arg))
                                        {
                                            // arg 在循环中有定值
                                            move = false;
                                            break;
                                        }
                                    }
                                }
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
                            }
                        }
                        // std::cout<<changed<<std::endl;
                    } while (changed);

                    // 循环不变式外提到前驱
                    auto base = loop_searcher.get_loop_base(inner_loop);
                    for (auto prev : base->get_pre_basic_blocks())
                    {
                        if (inner_loop->find(prev) == inner_loop->end())
                        {
                            #ifdef DEBUG
                            printf("find the prev bb and we will insert some instruction\n");
                            #endif
                            auto br = prev->get_terminator();
                            prev->delete_instr(br);
                            while (wait_move.size() > 0)
                            {
                                #ifdef DEBUG
                                printf("insert one instruction\n");
                                #endif
                                auto move_ins = wait_move.front();
                                prev->add_instruction(move_ins);
                                l_val2bb_set[move_ins]->delete_instr(move_ins);
                                wait_move.pop();
                            }
                            prev->add_instruction(br);
                            break;
                        }
                    }

                    auto parent_loop = loop_searcher.get_parent_loop(inner_loop);
                    loops_in_func.erase(inner_loop);
                    #ifdef DEBUG
                        printf("renew the loops_in_func => %d\n",loops_in_func.size());
                    #endif
                    if (parent_loop != nullptr)
                    {
                        #ifdef DEBUG
                        printf("renew the loop2child\n");
                        #endif
                        auto base = loop_searcher.get_loop_base(parent_loop);
                        loop2child[base] = true;
                    }
                    inner_loop = parent_loop;
                }
            }
        }
    }
}


