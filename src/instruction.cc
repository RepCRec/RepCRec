//
// Created by 梁俊华 on 11/21/22.
//

#include "instruction.h"

int Instruction::initial_id_ = 1;

Instruction::Instruction() : id_(initial_id_++), type_(InstructType::UNKNOWN) {}

Instruction::~Instruction() = default;

InstructType Instruction::get_type() const {
    return type_;
}

int Instruction::get_var_id() const {
    return var_id_;
}

int Instruction::get_var_value() const {
    return var_value_;
}