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
    Transaction(int transaction_id, bool is_read_only = false);
    ~Transaction() = default;

    [[nodiscard]] int get_transaction_id() const;
    [[nodiscard]] bool is_read_only() const;

private:
    int transaction_id_;
    bool is_read_only_;
    std::vector<int> accessed_sites_;
};


#endif//REPCREC_TRANSACTION_H
