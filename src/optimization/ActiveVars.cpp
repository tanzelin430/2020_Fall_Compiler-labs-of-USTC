#include "ActiveVars.hpp"
#include "set"
#include <map>
#include"vector"
#include"iostream"
#include <unordered_map>

/*
    bool judge_set:
    return value:true false
    true: equal
    false:not equal
*/
bool judge_set(std::set<Value*>set1,std::set<Value*>set2)
{
    //firstly we judge size
    if (set1.size() != set2.size())
        return false;
    //based on same size,we compare each element
    std::set<Value *>::iterator it1, it2;
    for (it1 = set1.begin(), it2 = set2.begin(); it1 != set1.end(); it1++,it2++)
    {
        if (*it1 != *it2)
        {
            return false;
        }
    }
    return true;
}
void union_set_reserve(std::set<Value *> &set1, std::set<Value *> set2,std::set<Value*> set3)
{
    for(auto item:set3)
    {

        set2.insert(item);

    }
    set1 = set2;
}
void intersection_set(std::set<Value *> &set1, std::set<Value *> set2)
{
    for(auto item : set2){
        if (set1.find(item) != set1.end())
        {
            set1.erase(item);
        }
    }
}
// std::string ActiveVars::use_def_print()
// {
//     std::string use_def_set;
//     func_->set_instr_name();
//     use_def_set += "{\n";
//     use_def_set += "\"function\": \"";
//     use_def_set += func_->get_name();
//     use_def_set += "\",\n";

//     use_def_set += "\"map_use\": {\n";
//     for (auto &p : map_use)
//     {
//         use_def_set += "  \"";
//         use_def_set += p.first->get_name();
//         use_def_set += "\": [";
//         for (auto &v : p.second)
//         {
//             use_def_set += "\"%";
//             use_def_set += v->get_name();
//             use_def_set += "\",";
//         }
//         use_def_set += "]";
//         use_def_set += ",\n";
//     }
//     use_def_set += "\n";
//     use_def_set += "    },\n";

//     use_def_set += "\"map_def\": {\n";
//     for (auto &p : map_def)
//     {
//         use_def_set += "  \"";
//         use_def_set += p.first->get_name();
//         use_def_set += "\": [";
//         for (auto &v : p.second)
//         {
//             use_def_set += "\"%";
//             use_def_set += v->get_name();
//             use_def_set += "\",";
//         }
//         use_def_set += "]";
//         use_def_set += ",\n";
//     }
//     use_def_set += "\n";
//     use_def_set += "    }\n";

//     use_def_set += "}\n";
//     use_def_set += "\n";
//     return use_def_set;
// }
void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    // std::ofstream output_active_vars_use_def;
    // output_active_vars_use_def.open("active_vars_ud.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        }
        else
        {
            func_ = func;

            func_->set_instr_name();
            live_in.clear();
            live_out.clear();
            // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内
            //假设CFG已经建好了

            std::set<Value *> _use;
            std::set<Value *> _def;
            for (auto bb : func_->get_basic_blocks())
            {
                for (auto instr: bb->get_instructions())
                {
                    if(instr->isBinary() || instr->is_fcmp() || instr->is_cmp() )//双目运算符
                    {
                        //无论set里面是否存在这个元素，都可以插入。它会自动处理的
                        //judge:if it is const
                        // std::cout << "entering" << bb->get_name() << std::endl;
                        if (dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr && _def.find(instr->get_operand(0)) == _def.end()) //first value is a var
                        {
                            // std::cout << "approaching target in " << bb->get_name() << "get ready to insert " << instr->get_operand(0)->get_name() << std::endl;

                            _use.insert(instr->get_operand(0));
                            // std::set<Value *>::iterator iter = _use.end();
                            // iter--;
                            // std::cout << "last item: "<<(*iter)->get_name()<<std::endl;
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
                    else if (instr->is_gep())
                    {
                        for (int i = 0; i < instr->get_num_operand(); i++)
                        {
                            if (dynamic_cast<ConstantInt *>(instr->get_operand(i)) == nullptr && dynamic_cast<ConstantFP *>(instr->get_operand(i)) == nullptr && _def.find(instr->get_operand(i)) == _def.end()) //second value is a var
                            {
                                _use.insert(instr->get_operand(i));
                                // std::cout << "insert gep op ver1: " << instr->get_operand(i)->print() << "for bb: "<< bb->get_name()<<std::endl;
                                // std::cout << "insert gep op ver2: " << instr->get_operand(i)->get_name() << "for bb: " << bb->get_name() << std::endl;
                            }
                            if (_use.find(instr) == _use.end()) //not find
                            {
                                _def.insert(instr);
                            }
                        }

                    }
                    //mem op
                    else if (instr->is_alloca())
                    {
                        if (_use.find(instr) == _use.end()) //not find
                        {
                            _def.insert(instr);
                        }
                    }
                    else if (instr->is_load())
                    {
                        auto l_val = instr->get_operand(0);
                        if (_def.find(l_val) == _def.end())
                        {
                            _use.insert(l_val);
                        }
                        if (_use.find(instr) == _use.end()) //not find
                        {
                            _def.insert(instr);
                        }
                    }
                    else if(instr->is_store())
                    {
                        //r_val:use
                        //l_val:def
                        auto l_val = static_cast<StoreInst *>(instr)->get_lval();
                        auto r_val = static_cast<StoreInst *>(instr)->get_rval();
                        if ((_def.find(r_val) == _def.end()) && (dynamic_cast<ConstantInt *>(r_val) == nullptr) && (dynamic_cast<ConstantFP *>(r_val) == nullptr))
                        {
                            // std::cout << "insert str op ver1: " << r_val->print() << "for bb: " << bb->get_name() << std::endl;
                            // std::cout << "insert str op ver2: " << r_val->get_name() << "for bb: " << bb->get_name() << std::endl;
                            _use.insert(r_val);
                        }
                        if ((_def.find(l_val) == _def.end()) && (dynamic_cast<ConstantInt *>(l_val) == nullptr)) //not find
                        {
                            // std::cout << "insert str op ver1: " << l_val->print() << "for bb: " << bb->get_name() << std::endl;
                            // std::cout << "insert str op ver2: " << l_val->get_name() << "for bb: " << bb->get_name() << std::endl;
                            _use.insert(l_val);
                        }
                    }
                    //other operations
                    else if (instr->is_call())
                    {
                        for (int i = 1; i < instr->get_num_operand(); i++)
                        {
                            if ((_def.find(instr->get_operand(i)) == _def.end()) && (dynamic_cast<ConstantInt *>(instr->get_operand(i)) == nullptr) && (dynamic_cast<ConstantFP *>(instr->get_operand(i)) == nullptr))
                            {
                                // std::cout << "insert call op ver1: " << instr->get_operand(i)->print() << "for bb: " << bb->get_name() << std::endl;
                                // std::cout << "insert call op ver2: " << instr->get_operand(i)->get_name() << "for bb: " << bb->get_name() << std::endl;
                                _use.insert(instr->get_operand(i));
                            }

                        }
                        if ((_use.find(instr) == _use.end()) && !instr->is_void()) //not find
                        {
                            _def.insert(instr);
                        }
                    }
                    else if (instr->is_zext() || instr->is_si2fp() || instr->is_fp2si())
                    {
                        if (_def.find(instr->get_operand(0)) == _def.end())
                        {
                            _use.insert(instr->get_operand(0));
                        }
                        if (_use.find(instr) == _use.end())
                        {
                            _def.insert(instr);
                        }
                    }
                    else if (instr->is_ret())
                    {
                        if ((static_cast<ReturnInst *>(instr)->is_void_ret() == false) && (_def.find(instr->get_operand(0)) == _def.end()) && (dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr) && (dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr))
                        {
                            _use.insert(instr->get_operand(0));
                        }
                    }
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
                    else if (instr->is_br())
                    {
                        if (dynamic_cast<ConstantFP *>(instr->get_operand(0)) == nullptr && dynamic_cast<ConstantInt *>(instr->get_operand(0)) == nullptr && _def.find(instr->get_operand(0)) == _def.end())
                        {
                            if (static_cast<BranchInst*>(instr)->is_cond_br())
                            {
                                // std::cout << "insert br op ver1: " << instr->get_operand(0)->print() << "for bb: " << bb->get_name() << std::endl;
                                // std::cout << "insert br op ver2: " << instr->get_operand(0)->get_name() << "for bb: " << bb->get_name() << std::endl;
                                _use.insert(instr->get_operand(0));
                            }
                        }
                    }
                }
                // std::cout << "now at bb " << bb->get_name() <<std::endl;
                // std::cout << "use size:" << _use.size() << std::endl;
                map_def.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, _def));
                // std::cout << "bb and insert: " << bb->get_name() << std::endl;
                map_use.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, _use));
                _def.clear();
                _use.clear();
            }

            //end building use and def
            //now,map_def,map_use store all the def and use with their bb attached
            //begin building IN and OUT for each bb
            bool is_change = true;//is_change is used to indicate whether IN[bb] is changed
            std::set<Value *> union_ins;
            std::set<Value *> temp_outb;
            std::set<Value *> in_reserve;
            //initialization

            for(auto bb : func->get_basic_blocks())
            {
                std::set<Value *> initial_set;
                live_in.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, initial_set));
            }
            // std::cout << "segfault1" << std::endl;
            while (is_change)
            {
                is_change = false;
                for (auto bb : func->get_basic_blocks())
                {
                    for (auto succ_bb : bb->get_succ_basic_blocks())
                    {

                        auto INS = live_in.find(succ_bb);
                        std::set<Value *> old_ins;
                        while (judge_set(INS->second, old_ins) == false)
                        {
                            old_ins = INS->second;
                            for (auto item : INS->second)
                            {
                                // std::cout << "now i am working for bb: " << bb->get_name() << std::endl;
                                // std::cout << "now i am judging element: " << item->get_name() << std::endl;
                                if (big_phi.find(succ_bb) != big_phi.end())//phi instr exist
                                {
                                    if (big_phi.find(succ_bb)->second.find(item) != big_phi.find(succ_bb)->second.end()) //there is a phi instr containing item
                                    {
                                        //now check if bb matches
                                        if (big_phi.find(succ_bb)->second.find(item)->second->get_name() == bb->get_name())
                                        {
                                            union_ins.insert(item);
                                            // std::cout << "i am inserting " << item->get_name() << " for father label : " << bb->get_name() << " from succ_bb: " << succ_bb->get_name() << std::endl;
                                            // if (bb->get_name() == "label25")
                                            // {
                                            //     for (auto test : INS->second)
                                            //         std::cout << "遍历：" << test->get_name() << std::endl;
                                            // }
                                        }
                                    }
                                    else //there is a phi but not containing item
                                    {
                                            union_ins.insert(item);
                                            // std::cout << "i am inserting " << item->get_name() << " for father label : " << bb->get_name() << " from succ_bb: " << succ_bb->get_name() << std::endl;
                                            // if (bb->get_name() == "label25")
                                            // {
                                            //     for (auto test : INS->second)
                                            //         std::cout << "遍历：" << test->get_name() << std::endl;
                                            // }
                                    }
                                }
                                else//no phi instr
                                {
                                            union_ins.insert(item);
                                            // std::cout << "i am inserting " << item->get_name() << " for father label : " << bb->get_name() << " from succ_bb: " << succ_bb->get_name() << std::endl;
                                            // if (bb->get_name() == "label25")
                                            // {
                                            //     for (auto test : INS->second)
                                            //         std::cout << "遍历：" << test->get_name() << std::endl;
                                            // }

                                }
                            }
                        }
                    }
                    if(live_out.find(bb) == live_out.end())
                    {
                        live_out.insert(std::pair<BasicBlock *, std::set<Value *>>(bb, union_ins));
                    }
                    else live_out.find(bb)->second = union_ins;
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

                    union_ins.clear();
                    in_reserve.clear();
                    temp_outb.clear();
                }
            }
            // output_active_vars_use_def << use_def_print();
            output_active_vars
                << print();
            output_active_vars << ",";
        }
        map_def.clear();
        map_use.clear();
        big_phi.clear();
    }
    output_active_vars << "]";
    output_active_vars.close();
    return ;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars +=  "{\n";
    active_vars +=  "\"function\": \"";
    active_vars +=  func_->get_name();
    active_vars +=  "\",\n";

    active_vars +=  "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]" ;
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars +=  "    },\n";

    active_vars +=  "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}
