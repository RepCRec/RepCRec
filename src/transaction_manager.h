//
// Created by 梁俊华 on 11/21/22.
//

#ifndef REPCREC_TRANSACTION_MANAGER_H
#define REPCREC_TRANSACTION_MANAGER_H

#include "site_manager.h"
#include "request.h"
#include "global.h"
#include "lock_manager.h"

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <exception>

class TransactionManager {
public:
    ~TransactionManager() = default;
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;
    TransactionManager(const TransactionManager&&) = delete;

    static TransactionManager& get_instance() {
        static TransactionManager instance;
        return instance;
    }

    void add_instruction(const std::shared_ptr<Instruction>& insr, repcrec::timestamp_t timestamp);
    void execute_instructions(repcrec::timestamp_t timestamp);
    void add_transaction(repcrec::tran_id_t tran_id, const std::shared_ptr<Transaction>& transaction);
    void evict_transaction(int tran_id);

    [[nodiscard]] std::shared_ptr<SiteManager> get_site_manager() const;
    [[nodiscard]] std::shared_ptr<Transaction> get_transaction(repcrec::tran_id_t tran_id) const;
    [[nodiscard]] std::shared_ptr<LockManager> get_lock_manager() const;

private:
    TransactionManager();

    std::shared_ptr<LockManager> lock_manager_;
    std::unordered_map<repcrec::tran_id_t, std::shared_ptr<Transaction>> transactions_;
    std::shared_ptr<SiteManager> site_manager_;
    std::queue<std::shared_ptr<Instruction>> blocked_queue_;
    std::queue<std::shared_ptr<Request>> request_queue_;
};


#endif//REPCREC_TRANSACTION_MANAGER_H
