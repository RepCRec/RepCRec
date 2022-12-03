/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "variable.h"

repcrec::variable::Variable::Variable(repcrec::var_id_t var_id) : id_(var_id), value_(var_id * 10) {
    versions_[-1] = value_;
}

std::shared_ptr<repcrec::transaction::Transaction> repcrec::variable::Variable::get_exclusive_lock_owner() {
    return exclusive_lock_owner_;
}

std::unordered_map<repcrec::tran_id_t, std::shared_ptr<repcrec::transaction::Transaction>> repcrec::variable::Variable::get_shared_lock_owners() {
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

repcrec::var_t repcrec::variable::Variable::get_value(repcrec::timestamp_t timestamp) const {
    if (timestamp == -1) {
        return value_;
    }
    auto iter = prev(versions_.upper_bound(timestamp));
    return iter->second;
}

void repcrec::variable::Variable::set_value(repcrec::var_t value, repcrec::timestamp_t timestamp) {
    value_ = value;
    versions_[timestamp] = value;
}

bool repcrec::variable::Variable::has_shared_lock(repcrec::tran_id_t tran_id) const {
    return shared_lock_owners_.count(tran_id);
}

bool repcrec::variable::Variable::has_exclusive_lock(repcrec::tran_id_t tran_id) const {
    return exclusive_lock_owner_ != nullptr and exclusive_lock_owner_->get_transaction_id() == tran_id;
}

bool repcrec::variable::Variable::has_shared_lock_exclude_self(repcrec::tran_id_t tran_id) const {
    return !shared_lock_owners_.empty() and !shared_lock_owners_.count(tran_id);
}

bool repcrec::variable::Variable::has_exclusive_lock_exclude_self(repcrec::tran_id_t tran_id) const {
    return exclusive_lock_owner_ != nullptr and exclusive_lock_owner_->get_transaction_id() != tran_id;
}