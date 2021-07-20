

/** NOTE:
                 * Now we can go on handling phi instruction
                 * when we find out that one Phi operand BB is in the deleteSet,
                 * we should remake this phi instr,
                 * turning it into a simple assigning instr
                 * Then we can use ConstPropagation on this instr
                 **/
for (auto bbs : func->get_basic_blocks())
{
    std::vector<Instruction *> wait_delete;
    for (auto instr : bbs->get_instructions())
    {
        if (instr->is_phi())
        {
            std::vector<BasicBlock *> validPreBB;
            std::vector<Value *> validPreVal;
            std::set<int> deleteOperIndex;
            for (int i = 0; i < instr->get_operands().size() / 2; i++)
            {
                auto brFromBB = static_cast<BasicBlock *>(instr->get_operand(2 * i + 1));
                if (!deleteBlockSet.count(brFromBB)) // this source is alive
                {
                    auto valFromBB = instr->get_operand(2 * i);
                    validPreBB.push_back(brFromBB);
                    validPreVal.push_back(valFromBB);
                }
                else
                {
                    deleteOperIndex.insert(2 * i);
                }
            }
            if (deleteOperIndex.size() == instr->get_operands().size() / 2) // this bb is an orphan, so this might be useless code
            {
                wait_delete.push_back(instr);
            }
            else if (deleteOperIndex.size() == (instr->get_operands().size() / 2) - 1)
            {
                if (instr->get_type()->is_integer_type())
                {
                    auto constValue = cast_constantint(validPreVal[0]);
                    if (constValue != nullptr)
                    {
                        const_val_stack.insert(std::pair<Value *, Value *>(instr, constValue));
                        instr->replace_all_use_with(constValue);
                        wait_delete.push_back(instr);
                    }
                    else
                    {
                        instr->replace_all_use_with(validPreVal[0]);
                        wait_delete.push_back(instr);
                    }
                }
                else if (instr->get_type()->is_float_type())
                {
                    auto constValue = cast_constantfp(validPreVal[0]);
                    if (constValue != nullptr)
                    {
                        const_val_stack.insert(std::pair<Value *, Value *>(instr, constValue));
                        instr->replace_all_use_with(constValue);
                        wait_delete.push_back(instr);
                    }
                    else
                    {
                        instr->replace_all_use_with(validPreVal[0]);
                        wait_delete.push_back(instr);
                    }
                }
                else
                {
                    instr->replace_all_use_with(validPreVal[0]);
                    wait_delete.push_back(instr);
                }
            }
            else
            {
                for (auto index : deleteOperIndex)
                {
                    instr->remove_operands(index, index + 1);
                }
            }
        }
    }
    for (auto instr : wait_delete)
    {
        bbs->delete_instr(instr);
    }
}
for (auto bb : deleteBlockSet)
{
    bb->erase_from_parent();
}