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
#include <set>

struct tm_block_queue_cmp {
    bool operator()(const std::shared_ptr<Request>& r1, const std::shared_ptr<Request>& r2) const {
        return r1->get_timestamp() < r2->get_timestamp();
    }
};

struct tm_transaction_time_cmp {
    bool operator()(const std::shared_ptr<Transaction>& r1, const std::shared_ptr<Transaction>& r2) const {
        return r1->get_timestamp() > r2->get_timestamp();
    }
};

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
    void add_transaction(repcrec::tran_id_t tran_id, const std::shared_ptr<Transaction>& transaction);
    void add_request_to_blocked_queue(const std::shared_ptr<Request>& request);
    void execute_instructions(repcrec::timestamp_t timestamp);
    void evict_transaction(repcrec::tran_id_t tran_id);
    void abort_transaction(repcrec::tran_id_t tran_id);

    [[nodiscard]] bool is_all_instructions_finished() const;
    [[nodiscard]] std::shared_ptr<SiteManager> get_site_manager() const;
    [[nodiscard]] std::shared_ptr<Transaction> get_transaction(repcrec::tran_id_t tran_id) const;
    [[nodiscard]] std::shared_ptr<LockManager> get_lock_manager() const;

private:
    TransactionManager();

    std::shared_ptr<LockManager> lock_manager_;
    std::shared_ptr<SiteManager> site_manager_;
    std::set<std::shared_ptr<Transaction>, tm_transaction_time_cmp> transactions_set_;
    std::unordered_map<repcrec::tran_id_t, std::shared_ptr<Transaction>> transactions_;
    std::unordered_map<repcrec::tran_id_t, std::set<std::shared_ptr<Request>, tm_block_queue_cmp>> blocked_transactions_queue_;
    std::shared_ptr<Request> request_;
};


#endif//REPCREC_TRANSACTION_MANAGER_H
