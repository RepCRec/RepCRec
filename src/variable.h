//
// Created by 梁俊华 on 11/23/22.
//

#ifndef REPCREC_VARIABLE_H
#define REPCREC_VARIABLE_H

#include "transaction.h"

#include <unordered_map>
#include <memory>

class Variable {
public:
    Variable();
    ~Variable();

    std::shared_ptr<Transaction> get_exclusive_lock_owner();
    std::unordered_map<int, std::shared_ptr<Transaction>> get_shared_lock_owners();
    bool add_shared_transaction(const std::shared_ptr<Transaction>& transaction);
    bool set_exclusive_transaction(const std::shared_ptr<Transaction>& transaction);
    void remove_exclusive_owner();
    bool remove_shared_owner(int tid);

    int get_id() const;
    int get_value() const;
    void set_value(int value);

private:
    int id_;
    int value_;

    std::shared_ptr<Transaction> exclusive_lock_owner_;
    std::unordered_map<int, std::shared_ptr<Transaction>> shared_lock_owners_;

    static int initial_id_;
};

#endif//REPCREC_VARIABLE_H
