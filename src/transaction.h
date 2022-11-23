//
// Created by 梁俊华 on 11/21/22.
//

#ifndef REPCREC_TRANSACTION_H
#define REPCREC_TRANSACTION_H

#include <vector>
#include <memory>
#include "instruction.h"

class Transaction {
public:
    Transaction();
    ~Transaction();

    [[nodiscard]] int get_transaction_id() const;

private:
    int transaction_id_;
    std::vector<std::shared_ptr<Instruction>> instructions_;

    static int initial_id_;
};


#endif//REPCREC_TRANSACTION_H
