#include "TailRecursion.hpp"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <list>
#define DEBUG 1

void TailRecurtion::run()
{

    auto func_list = m_->get_functions();
    for (auto func : func_list)
    {
        // 遍历函数列表，寻找尾递归函数
#ifdef DEBUG
        printf("---------- get the function ----------\n");
#endif
        auto entry = func->get_entry_block(); // 存放 label_entry
        std::vector<Instruction *> wait_inset_phi;
        // std::stack<Instruction *> delete_set;
        if (func->get_basic_blocks().size() == 0)
        {
            continue;
        }
        else
        {

            for (auto arg = func->arg_begin(); arg != func->arg_end(); arg++){
                auto use_list = (*arg)->get_use_list();
                std::cout << (*arg)->print() << std::endl;

                for (auto user : use_list)
                {
                    std::cout << user.val_->print() << std::endl;
                }
            }


            for (auto bb : func->get_basic_blocks())
            {
                // 遍历 func 中的每一个基本块
                printf("------get one bb------\n");
                auto end = bb->get_terminator();
                if (end->is_ret() && end->get_operands().size() != 0)
                {
                    // printf("---get ret!---\n");
                    auto ret_val = end->get_operand(0);
                    // std::cout << ret_val->print() << std::endl;
                    // printf("get the op\n");
                    auto ins = dynamic_cast<Instruction *>(ret_val);
                    if(!ins)
                        continue;
                    std::cout<<ins->print()<<std::endl;

                    if (!ins->is_call())
                        continue;
                    // printf("-the ret's op is call-\n");
                    dynamic_cast<Function *>(ins->get_operand(0));
                    if (ins->get_operand(0) != ins->get_function())
                        continue;
                    wait_inset_phi.push_back(ins);

                }
            } // for bb in function
            if (!wait_inset_phi.empty())
            {
                // 存在尾递归
                printf("------there is tail recurtion!------\n");

                auto loop_base = BasicBlock::create(m_,"",func);
                // 增加一个循环入口块，并修改相应前驱后继关系
                loop_base->add_pre_basic_block(entry);//设定 loop_base 的前驱是 entry
                std::stack<BasicBlock*> wait_delete_succ;
                for(auto succ: entry->get_succ_basic_blocks()){
                    // loop_base 的后继是原先 entry 的所有后继
                    loop_base->add_succ_basic_block(succ);
                    succ->add_pre_basic_block(loop_base);
                    succ->remove_pre_basic_block(entry);
                    wait_delete_succ.push(succ);
                }
                // 修改 entry 的后继，使其只能跳转到 loop_base
                while(!wait_delete_succ.empty()){
                    entry->remove_succ_basic_block(wait_delete_succ.top());
                    wait_delete_succ.pop();
                }
                entry->add_succ_basic_block(loop_base);
                // 将 entry 中的所有语句都转移到 loop_base 中
                auto terminator = entry->get_terminator();
                for(auto entry_ins:entry->get_instructions()){
                    loop_base->add_instruction(entry_ins);
                    // if(entry_ins!=entry->get_terminator()) {
                    //     delete_set.push(entry_ins);
                    // }
                }
                entry->get_instructions().clear(); // 不能直接 delete，要保护 uselist

                // printf("print label_entry terminator\n");
                // std::cout<<terminator->print()<<std::endl;

                static_cast<BranchInst *>(terminator)->create_br(loop_base, entry);
                entry->delete_instr(terminator);
                // 现在 label_entry 中就只剩了一条跳转语句
                int n = 1;

                for (auto arg = func->arg_begin(); arg != func->arg_end(); arg++, n++)
                {
                    // 给每个参数都插入 phi 函数
                    // printf("------create phi------\n");

                    auto phi = PhiInst::create_phi((*arg)->get_type(), loop_base); // loop_base
                    // printf("phi is success,then we will add the first pair\n");

                    phi->add_phi_pair_operand((*arg), entry);

                    // std::cout << phi->print() << std::endl;

                    for (int i = 0; i < wait_inset_phi.size(); i++)
                    {
                        // printf("--add args' pair---\n");
                        phi->add_phi_pair_operand(wait_inset_phi[i]->get_operand(n), wait_inset_phi[i]->get_parent());
                    }
                    loop_base->add_instr_begin(phi); // loop_base

                    auto use_list = (*arg)->get_use_list();
                    std::cout << (*arg)->print() << std::endl;

                    for(auto user: use_list){
                        std::cout << user.val_->print() << std::endl;
                        if(user.val_ == phi){
                            continue;
                        }
                        else{
                            auto val = dynamic_cast<User *>(user.val_);
                            assert(val && "new_val is not a user");
                            val->set_operand(user.arg_no_, phi);
                        }
                    }

                }
                for(int i = 0;i<wait_inset_phi.size();i++){
                    auto call_bb = wait_inset_phi[i]->get_parent();
                    call_bb->delete_instr(wait_inset_phi[i]);
                    auto terminator = call_bb->get_terminator();
                    static_cast<BranchInst *>(terminator)->create_br(loop_base, call_bb); // loop_base
                    call_bb->delete_instr(terminator);
                }
            }
        }
    }
}
