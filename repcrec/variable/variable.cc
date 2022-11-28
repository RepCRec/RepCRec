//
// Created by 梁俊华 on 11/23/22.
//

#include "variable.h"

repcrec::variable::Variable::Variable(repcrec::var_id_t var_id) : id_(var_id), value_(var_id * 10), latest_commit_time_(0) {}

std::shared_ptr<repcrec::transaction::Transaction> repcrec::variable::Variable::get_exclusive_lock_owner() {
    return exclusive_lock_owner_;
}

std::unordered_map<int, std::shared_ptr<repcrec::transaction::Transaction>> repcrec::variable::Variable::get_shared_lock_owners() {
    return shared_lock_owners_;
}

bool repcrec::variable::Variable::add_shared_transaction(const std::shared_ptr<repcrec::transaction::Transaction>& transaction) {
    if (shared_lock_owners_.count(transaction->get_transaction_id())) {
        return false;
    }

    shared_lock_owners_.insert({transaction->get_transaction_id(), transaction});
    return true;
}

bool repcrec::variable::Variable::set_exclusive_transaction(const std::shared_ptr<repcrec::transaction::Transaction>& transaction) {
    if (exclusive_lock_owner_ != nullptr) {
        return false;
    }

    exclusive_lock_owner_ = transaction;
    return true;
}

void repcrec::variable::Variable::remove_exclusive_owner() {
    exclusive_lock_owner_ = nullptr;
}

bool repcrec::variable::Variable::remove_shared_owner(repcrec::tran_id_t tid) {
    if (!shared_lock_owners_.count(tid)) {
        return false;
    }

    shared_lock_owners_.erase(tid);
    return true;
}

repcrec::var_id_t repcrec::variable::Variable::get_id() const {
    return id_;
}

repcrec::var_t repcrec::variable::Variable::get_value() const {
    return value_;
}

void repcrec::variable::Variable::set_value(repcrec::var_t value) {
    value_ = value;
}

bool repcrec::variable::Variable::has_shared_lock(repcrec::tran_id_t tran_id) const {
    return shared_lock_owners_.contains(tran_id);
}

bool repcrec::variable::Variable::has_exclusive_lock(repcrec::tran_id_t tran_id) const {
    return exclusive_lock_owner_ != nullptr and exclusive_lock_owner_->get_transaction_id() == tran_id;
}

bool repcrec::variable::Variable::has_shared_lock_exclude_self(repcrec::tran_id_t tran_id) const {
    return !shared_lock_owners_.empty() and !shared_lock_owners_.contains(tran_id);
}

bool repcrec::variable::Variable::has_exclusive_lock_exclude_self(repcrec::tran_id_t tran_id) const {
    return exclusive_lock_owner_ != nullptr and exclusive_lock_owner_->get_transaction_id() != tran_id;
}

repcrec::timestamp_t repcrec::variable::Variable::get_latest_commit_time() const {
    return latest_commit_time_;
}

void repcrec::variable::Variable::set_latest_commit_time(repcrec::timestamp_t timestamp) {
    latest_commit_time_ = timestamp;
}