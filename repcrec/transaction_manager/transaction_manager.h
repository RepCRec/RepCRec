/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#ifndef REPCREC_TRANSACTION_MANAGER_H
#define REPCREC_TRANSACTION_MANAGER_H

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <exception>
#include <set>

#include "../global.h"
#include "../lock_manager/lock_manager.h"
#include "../request/request.h"
#include "../site_manager/site_manager.h"

namespace repcrec::transaction_manager {
    struct tm_block_queue_cmp {
        bool operator()(const std::shared_ptr<repcrec::request::Request>& r1, const std::shared_ptr<repcrec::request::Request>& r2) const {
            return r1->get_timestamp() < r2->get_timestamp();
        }
    };

    struct tm_transaction_time_cmp {
        bool operator()(const std::shared_ptr<repcrec::transaction::Transaction>& r1, const std::shared_ptr<repcrec::transaction::Transaction>& r2) const {
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

        void add_instruction(const std::shared_ptr<repcrec::instruction::Instruction>& insr, repcrec::timestamp_t timestamp);
        void add_transaction(repcrec::tran_id_t tran_id, const std::shared_ptr<repcrec::transaction::Transaction>& transaction);
        void add_request_to_blocked_queue(const std::shared_ptr<repcrec::request::Request>& request);
        void add_to_site_waiting_map(repcrec::tran_id_t, repcrec::site_id_t var_id);
        void remove_from_site_waiting_map(repcrec::site_id_t site_id);
        void execute_instructions(repcrec::timestamp_t timestamp);
        void evict_transaction(repcrec::tran_id_t tran_id);
        void abort_transaction(repcrec::tran_id_t tran_id);

        [[nodiscard]] bool is_all_instructions_finished() const;
        [[nodiscard]] bool is_transaction_waiting_for_site(repcrec::tran_id_t tran_id) const;
        [[nodiscard]] std::shared_ptr<repcrec::site_manager::SiteManager> get_site_manager() const;
        [[nodiscard]] std::shared_ptr<repcrec::transaction::Transaction> get_transaction(repcrec::tran_id_t tran_id) const;
        [[nodiscard]] std::shared_ptr<repcrec::lock_manager::LockManager> get_lock_manager() const;
        [[nodiscard]] std::unordered_map<repcrec::tran_id_t, std::shared_ptr<repcrec::transaction::Transaction>> get_transactions() const;

        static repcrec::timestamp_t curr_timestamp;

    private:
        TransactionManager();

        std::shared_ptr<repcrec::request::Request> request_;
        std::shared_ptr<repcrec::lock_manager::LockManager> lock_manager_;
        std::shared_ptr<repcrec::site_manager::SiteManager> site_manager_;
        std::set<std::shared_ptr<repcrec::transaction::Transaction>, tm_transaction_time_cmp> transactions_set_;
        std::unordered_map<repcrec::tran_id_t, repcrec::site_id_t> site_waiting_map_;
        std::unordered_map<repcrec::tran_id_t, std::shared_ptr<repcrec::transaction::Transaction>> transactions_;
        std::unordered_map<repcrec::tran_id_t, std::set<std::shared_ptr<repcrec::request::Request>, tm_block_queue_cmp>> blocked_transactions_queue_;
    };
} // namespace repcrec::transaction_manager.

#endif //REPCREC_TRANSACTION_MANAGER_H
