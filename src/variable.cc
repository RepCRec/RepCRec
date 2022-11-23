//
// Created by 梁俊华 on 11/23/22.
//

#include "variable.h"

int Variable::initial_id_ = 1;

Variable::Variable() : id_(initial_id_) {
    value_ = id_ * 10;
    ++initial_id_;
}

Variable::~Variable() = default;

std::shared_ptr<Transaction> Variable::get_exclusive_lock_owner() {
    return exclusive_lock_owner_;
}

std::unordered_map<int, std::shared_ptr<Transaction>> Variable::get_shared_lock_owners() {
    return shared_lock_owners_;
}

bool Variable::add_shared_transaction(const std::shared_ptr<Transaction>& transaction) {
    if (shared_lock_owners_.count(transaction->get_transaction_id())) {
        return false;
    }

    shared_lock_owners_.insert({transaction->get_transaction_id(), transaction});
    return true;
}

bool Variable::set_exclusive_transaction(const std::shared_ptr<Transaction>& transaction) {
    if (exclusive_lock_owner_ != nullptr) {
        return false;
    }

    exclusive_lock_owner_ = transaction;
    return true;
}

void Variable::remove_exclusive_owner() {
    exclusive_lock_owner_ = nullptr;
}

bool Variable::remove_shared_owner(int tid) {
    if (!shared_lock_owners_.count(tid)) {
        return false;
    }

    shared_lock_owners_.erase(tid);
    return true;
}

int Variable::get_id() const {
    return id_;
}

int Variable::get_value() const {
    return value_;
}