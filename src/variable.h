//
// Created by 梁俊华 on 11/23/22.
//

#ifndef REPCREC_VARIABLE_H
#define REPCREC_VARIABLE_H

#include "transaction.h"
#include "global.h"

#include <unordered_map>
#include <memory>

class Variable {
public:
    explicit Variable(repcrec::var_id_t var_id);
    ~Variable() = default;

    std::shared_ptr<Transaction> get_exclusive_lock_owner();
    std::unordered_map<repcrec::var_id_t, std::shared_ptr<Transaction>> get_shared_lock_owners();
    bool add_shared_transaction(const std::shared_ptr<Transaction>& transaction);
    bool set_exclusive_transaction(const std::shared_ptr<Transaction>& transaction);
    void remove_exclusive_owner();
    bool remove_shared_owner(repcrec::tran_id_t tran_id);
    [[nodiscard]] bool has_shared_lock(repcrec::tran_id_t tran_id) const;
    [[nodiscard]] bool has_exclusive_lock(repcrec::tran_id_t tran_id) const;
    [[nodiscard]] bool has_shared_lock_exclude_self(repcrec::tran_id_t tran_id) const;
    [[nodiscard]] bool has_exclusive_lock_exclude_self(repcrec::tran_id_t tran_id) const;

    [[nodiscard]] repcrec::var_id_t get_id() const;
    [[nodiscard]] repcrec::var_t get_value() const;
    void set_value(int value);

private:
    repcrec::var_id_t id_;
    repcrec::var_t value_;
    repcrec::timestamp_t latest_commit_time_;

    std::shared_ptr<Transaction> exclusive_lock_owner_;
    std::unordered_map<repcrec::tran_id_t, std::shared_ptr<Transaction>> shared_lock_owners_;
};

#endif//REPCREC_VARIABLE_H
