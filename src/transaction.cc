//
// Created by 梁俊华 on 11/21/22.
//

#include "transaction.h"


Transaction::Transaction(int transaction_id, bool is_read_only) : transaction_id_(transaction_id), is_read_only_(is_read_only), accessed_sites_(std::vector<int>()) {}

int Transaction::get_transaction_id() const {
    return transaction_id_;
}

bool Transaction::is_read_only() const {
    return is_read_only_;
}