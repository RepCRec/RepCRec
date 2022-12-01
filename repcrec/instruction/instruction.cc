/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "instruction.h"

int repcrec::instruction::Instruction::initial_id_ = 1;

repcrec::instruction::Instruction::Instruction() : id_(initial_id_++), type_(InstructType::UNKNOWN), var_id_(-1), var_value_(-1), site_id_(-1), tran_id_(-1) {}

repcrec::instruction::Instruction::Instruction(const std::string& insr) : id_(initial_id_++) {
    size_t start = insr.find_first_of('(') + 1;
    size_t end = insr.find_first_of(')');
    if (insr.starts_with("beginRO")) {
        type_ = InstructType::BEGINO;
        tran_id_ = std::stoi(insr.substr(start + 1, end - start - 1));
    } else if (insr.starts_with("begin")) {
        type_ = InstructType::BEGIN;
        tran_id_ = std::stoi(insr.substr(start + 1, end - start - 1));
    } else if (insr.starts_with("end")) {
        type_ = InstructType::END;
        tran_id_ = std::stoi(insr.substr(start + 1, end - start - 1));
    } else if (insr.starts_with("dump")) {
        type_ = InstructType::DUMP;
    } else if (insr.starts_with("recover")) {
        type_ = InstructType::RECOVER;
        site_id_ = stoi(insr.substr(start , end - start));
    } else if (insr.starts_with("fail")) {
        type_ = InstructType::FAIL;
        site_id_ = stoi(insr.substr(start , end - start));
    } else if (insr.starts_with("W")) {
        type_ = InstructType::WRITE;
        std::string token;
        std::vector<std::string> tokens;
        size_t index = start + 1;
        while (insr[index] != ')') {
            if (insr[index] == ',') {
                tokens.push_back(token);
                token.clear();
            } else {
                token.push_back(insr[index]);
            }
            ++index;
        }
        tokens.push_back(token);
        tran_id_ = std::stoi(tokens[0]);
        var_id_ = std::stoi(tokens[1].substr(1));
        var_value_ = std::stoi(tokens[2]);
    } else if (insr.starts_with("R")) {
        type_ = InstructType::READ;
        std::string token;
        std::vector<std::string> tokens;
        size_t index = start + 1;
        while (insr[index] != ')') {
            if (insr[index] == ',') {
                tokens.push_back(token);
                token.clear();
            } else {
                token.push_back(insr[index]);
            }
            ++index;
        }
        tokens.push_back(token);
        tran_id_ = std::stoi(tokens[0]);
        var_id_ = std::stoi(tokens[1].substr(1));
    } else {
        type_ = InstructType::UNKNOWN;
    }
}

repcrec::instruction::InstructType repcrec::instruction::Instruction::get_type() const {
    return type_;
}

int repcrec::instruction::Instruction::get_var_id() const {
    return var_id_;
}

int repcrec::instruction::Instruction::get_var_value() const {
    return var_value_;
}

int repcrec::instruction::Instruction::get_site_id() const {
    return site_id_;
}

int repcrec::instruction::Instruction::get_tran_id() const {
    return tran_id_;
}