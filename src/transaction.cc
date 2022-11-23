//
// Created by 梁俊华 on 11/21/22.
//

#include "transaction.h"

int Transaction::initial_id_ = 1;

Transaction::Transaction() : transaction_id_(initial_id_++) {}

Transaction::~Transaction() = default;

int Transaction::get_transaction_id() const {
    return transaction_id_;
}