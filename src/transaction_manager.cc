//
// Created by 梁俊华 on 11/21/22.
//

#include "transaction_manager.h"

TransactionManager::TransactionManager() : site_manager_(std::make_shared<SiteManager>()), lock_manager_(std::make_shared<LockManager>()) {}

void TransactionManager::add_instruction(const std::shared_ptr<Instruction> &insr, repcrec::timestamp_t timestamp) {
    switch (insr->get_type()) {
        case InstructType::BEGIN: {
            request_queue_.push(std::make_shared<CreateTransactionRequest>(timestamp, insr->get_tran_id(), false));
            break;
        }
        case InstructType::BEGINO: {
            request_queue_.push(std::make_shared<CreateTransactionRequest>(timestamp, insr->get_tran_id(), true));
            break;
        }
        case InstructType::READ: {
            request_queue_.push(std::make_shared<ReadRequest>(timestamp, insr->get_tran_id(), insr->get_var_id()));
            break;
        }
        case InstructType::WRITE: {
            request_queue_.push(std::make_shared<WriteRequest>(timestamp, insr->get_tran_id(), insr->get_site_id(), insr->get_var_id(), insr->get_var_value()));
            break;
        }
        case InstructType::RECOVER: {
            request_queue_.push(std::make_shared<RecoveryRequest>(timestamp, insr->get_site_id()));
            break;
        }
        case InstructType::FAIL: {
            request_queue_.push(std::make_shared<FailRequest>(timestamp, insr->get_site_id()));
            break;
        }
        case InstructType::DUMP: {
            request_queue_.push(std::make_shared<DumpRequest>(timestamp));
            break;
        }
        case InstructType::END: {
            request_queue_.push(std::make_shared<EndTransactionRequest>(timestamp, insr->get_tran_id()));
            break;
        }
        default: {
            throw std::invalid_argument("Unknown instruction type.");
        }
    }
}

void TransactionManager::execute_instructions(repcrec::timestamp_t timestamp) {
    while (!request_queue_.empty()) {
        std::shared_ptr<Request> request = request_queue_.front();
        request_queue_.pop();
        request->exec();
    }
}

void TransactionManager::add_transaction(repcrec::tran_id_t tran_id, const std::shared_ptr<Transaction>& transaction) {
    transactions_[tran_id] = transaction;
}

void TransactionManager::evict_transaction(int tran_id) {
    transactions_.erase(tran_id);
}

std::shared_ptr<SiteManager> TransactionManager::get_site_manager() const {
    return site_manager_;
}

std::shared_ptr<Transaction> TransactionManager::get_transaction(repcrec::tran_id_t tran_id) const {
    return transactions_.at(tran_id);
}

std::shared_ptr<LockManager> TransactionManager::get_lock_manager() const {
    return lock_manager_;
}