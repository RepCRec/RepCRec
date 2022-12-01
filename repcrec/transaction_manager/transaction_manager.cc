/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "transaction_manager.h"

repcrec::timestamp_t repcrec::transaction_manager::TransactionManager::curr_timestamp = 10000;

repcrec::transaction_manager::TransactionManager::TransactionManager()
    : site_manager_(std::make_shared<repcrec::site_manager::SiteManager>()), lock_manager_(std::make_shared<repcrec::lock_manager::LockManager>()) {}

void repcrec::transaction_manager::TransactionManager::add_instruction(const std::shared_ptr<repcrec::instruction::Instruction> &insr, repcrec::timestamp_t timestamp) {
    switch (insr->get_type()) {
        case repcrec::instruction::InstructType::BEGIN: {
            request_ = std::make_shared<repcrec::request::CreateTransactionRequest>(timestamp, insr->get_tran_id(), false);
            request_->set_type(repcrec::instruction::InstructType::BEGIN);
            break;
        }
        case repcrec::instruction::InstructType::BEGINO: {
            request_ = std::make_shared<repcrec::request::CreateTransactionRequest>(timestamp, insr->get_tran_id(), true);
            request_->set_type(repcrec::instruction::InstructType::BEGINO);
            break;
        }
        case repcrec::instruction::InstructType::READ: {
            request_ = std::make_shared<repcrec::request::ReadRequest>(timestamp, insr->get_tran_id(), insr->get_var_id());
            request_->set_type(repcrec::instruction::InstructType::READ);
            break;
        }
        case repcrec::instruction::InstructType::WRITE: {
            request_ = std::make_shared<repcrec::request::WriteRequest>(timestamp, insr->get_tran_id(), insr->get_site_id(), insr->get_var_id(), insr->get_var_value());
            request_->set_type(repcrec::instruction::InstructType::WRITE);
            break;
        }
        case repcrec::instruction::InstructType::RECOVER: {
            request_ = std::make_shared<repcrec::request::RecoveryRequest>(timestamp, insr->get_site_id());
            request_->set_type(repcrec::instruction::InstructType::RECOVER);
            break;
        }
        case repcrec::instruction::InstructType::FAIL: {
            request_ = std::make_shared<repcrec::request::FailRequest>(timestamp, insr->get_site_id());
            request_->set_type(repcrec::instruction::InstructType::FAIL);
            break;
        }
        case repcrec::instruction::InstructType::DUMP: {
            request_ = std::make_shared<repcrec::request::DumpRequest>(timestamp);
            request_->set_type(repcrec::instruction::InstructType::DUMP);
            break;
        }
        case repcrec::instruction::InstructType::END: {
            request_ = std::make_shared<repcrec::request::EndTransactionRequest>(timestamp, insr->get_tran_id());
            request_->set_type(repcrec::instruction::InstructType::END);
            break;
        }
        default: {
            throw std::invalid_argument("Unknown instruction type.");
        }
    }
}

void repcrec::transaction_manager::TransactionManager::execute_instructions(repcrec::timestamp_t timestamp) {
    while (lock_manager_->detect_deadlock()) {
        // printf("INFO: Deadlock!!!\n");
        std::shared_ptr<repcrec::transaction::Transaction> transaction = *transactions_set_.begin();
        abort_transaction(transaction->get_transaction_id());
    }

    // When T was aborted, but received its later request, stop.
    // BEGIN(T1)
    // ==> T1 Abort
    // W(T1,x1,101)
    if (request_ != nullptr
        and request_->get_transaction_id() != -1
        and !transactions_.contains(request_->get_transaction_id())
        and !(request_->get_type() == repcrec::instruction::InstructType::BEGIN or request_->get_type() == repcrec::instruction::InstructType::BEGINO)) {
        return;
    }

    // When waiting, add to blocked queue
    if (request_ != nullptr
        and (lock_manager_->is_waiting_for_lock(request_->get_transaction_id())
             or blocked_transactions_queue_.contains(request_->get_transaction_id())
             or is_transaction_waiting_for_site(request_->get_transaction_id()))) {
        // printf("INFO: T%d instruction blocked!!!\n", request_->get_transaction_id());
        if (request_->get_type() == repcrec::instruction::InstructType::WRITE or request_->get_type() == repcrec::instruction::InstructType::READ) {
            // add_request_to_blocked_queue(request_); inside exec()
            request_->exec();
        } else {
            add_request_to_blocked_queue(request_);
        }
        request_ = nullptr;
        return;
    }

    if (request_ != nullptr) {
        request_->exec();
    }

    std::unordered_set<repcrec::tran_id_t> finished_set;
    for (const auto& [tran_id, request_set] : blocked_transactions_queue_) {
        if (!lock_manager_->is_waiting_for_lock(tran_id) and !is_transaction_waiting_for_site(tran_id)) {
            finished_set.insert(tran_id);
            for (const auto & iter : request_set) {
                iter->exec();
            }
        }
    }
    for (const repcrec::tran_id_t tran_id : finished_set) {
        blocked_transactions_queue_.erase(tran_id);
    }
    request_ = nullptr;
}

void repcrec::transaction_manager::TransactionManager::add_transaction(repcrec::tran_id_t tran_id, const std::shared_ptr<repcrec::transaction::Transaction>& transaction) {
    transactions_[tran_id] = transaction;
    transactions_set_.insert(transaction);
}

void repcrec::transaction_manager::TransactionManager::add_request_to_blocked_queue(const std::shared_ptr<repcrec::request::Request>& request) {
    blocked_transactions_queue_[request->get_transaction_id()].insert(request);
}

void repcrec::transaction_manager::TransactionManager::evict_transaction(int tran_id) {
    transactions_set_.erase(transactions_[tran_id]);
    transactions_.erase(tran_id);
}

std::shared_ptr<repcrec::transaction::Transaction> repcrec::transaction_manager::TransactionManager::get_transaction(repcrec::tran_id_t tran_id) const {
    return transactions_.at(tran_id);
}

std::shared_ptr<repcrec::site_manager::SiteManager> repcrec::transaction_manager::TransactionManager::get_site_manager() const {
    return site_manager_;
}

std::shared_ptr<repcrec::lock_manager::LockManager> repcrec::transaction_manager::TransactionManager::get_lock_manager() const {
    return lock_manager_;
}

bool repcrec::transaction_manager::TransactionManager::is_all_instructions_finished() const {
    return transactions_.empty();
}

void repcrec::transaction_manager::TransactionManager::abort_transaction(repcrec::tran_id_t tran_id) {
    transactions_set_.erase(transactions_[tran_id]);
    transactions_.erase(tran_id);
    blocked_transactions_queue_.erase(tran_id);
    lock_manager_->release_locks(tran_id);
    printf("INFO (%d): Abort transaction T%d\n", curr_timestamp, tran_id);
}

void repcrec::transaction_manager::TransactionManager::add_to_site_waiting_map(repcrec::tran_id_t tran_id, repcrec::site_id_t var_id) {
    site_waiting_map_[tran_id] = var_id;
}

void repcrec::transaction_manager::TransactionManager::remove_from_site_waiting_map(repcrec::site_id_t site_id) {
    std::unordered_set<repcrec::tran_id_t> unblocked_tran_id_set;
    for (const auto& [tid, sid] : site_waiting_map_) {
        if (sid == site_id) {
            unblocked_tran_id_set.insert(tid);
        }
    }
    for (const repcrec::tran_id_t& tid : unblocked_tran_id_set) {
        site_waiting_map_.erase(tid);
    }
}

bool repcrec::transaction_manager::TransactionManager::is_transaction_waiting_for_site(repcrec::tran_id_t tran_id) const {
    return site_waiting_map_.contains(tran_id);
}