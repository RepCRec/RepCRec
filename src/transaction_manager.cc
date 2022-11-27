//
// Created by 梁俊华 on 11/21/22.
//

#include "transaction_manager.h"

TransactionManager::TransactionManager() : site_manager_(std::make_shared<SiteManager>()), lock_manager_(std::make_shared<LockManager>()) {}

void TransactionManager::add_instruction(const std::shared_ptr<Instruction> &insr, repcrec::timestamp_t timestamp) {
    switch (insr->get_type()) {
        case InstructType::BEGIN: {
            request_ = std::make_shared<CreateTransactionRequest>(timestamp, insr->get_tran_id(), false);
            request_->set_type(InstructType::BEGIN);
            break;
        }
        case InstructType::BEGINO: {
            request_ = std::make_shared<CreateTransactionRequest>(timestamp, insr->get_tran_id(), true);
            request_->set_type(InstructType::BEGINO);
            break;
        }
        case InstructType::READ: {
            request_ = std::make_shared<ReadRequest>(timestamp, insr->get_tran_id(), insr->get_var_id());
            request_->set_type(InstructType::READ);
            break;
        }
        case InstructType::WRITE: {
            request_ = std::make_shared<WriteRequest>(timestamp, insr->get_tran_id(), insr->get_site_id(), insr->get_var_id(), insr->get_var_value());
            request_->set_type(InstructType::WRITE);
            break;
        }
        case InstructType::RECOVER: {
            request_ = std::make_shared<RecoveryRequest>(timestamp, insr->get_site_id());
            request_->set_type(InstructType::RECOVER);
            break;
        }
        case InstructType::FAIL: {
            request_ = std::make_shared<FailRequest>(timestamp, insr->get_site_id());
            request_->set_type(InstructType::FAIL);
            break;
        }
        case InstructType::DUMP: {
            request_ = std::make_shared<DumpRequest>(timestamp);
            request_->set_type(InstructType::DUMP);
            break;
        }
        case InstructType::END: {
            request_ = std::make_shared<EndTransactionRequest>(timestamp, insr->get_tran_id());
            request_->set_type(InstructType::END);
            break;
        }
        default: {
            throw std::invalid_argument("Unknown instruction type.");
        }
    }
}

void TransactionManager::execute_instructions(repcrec::timestamp_t timestamp) {
    if (lock_manager_->detect_deadlock()) {
        printf("INFO: Deadlock!!!\n");
        std::shared_ptr<Transaction> transaction = *transactions_set_.begin();
        abort_transaction(transaction->get_transaction_id());
    }

    if (request_ != nullptr and request_->get_transaction_id() != -1 and !transactions_.contains(request_->get_transaction_id())
        and !(request_->get_type() == InstructType::BEGIN or request_->get_type() == InstructType::BEGINO)) {
        return;
    }

    if (request_ != nullptr and (lock_manager_->is_waiting_for_lock(request_->get_transaction_id()) or blocked_transactions_queue_.contains(request_->get_transaction_id()))) {
        printf("INFO: T%d instruction blocked!!!\n", request_->get_transaction_id());
        if (request_->get_type() == InstructType::WRITE or request_->get_type() == InstructType::READ) {
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
        if (!lock_manager_->is_waiting_for_lock(tran_id)) {
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

void TransactionManager::add_transaction(repcrec::tran_id_t tran_id, const std::shared_ptr<Transaction>& transaction) {
    transactions_[tran_id] = transaction;
    transactions_set_.insert(transaction);
}

void TransactionManager::add_request_to_blocked_queue(const std::shared_ptr<Request>& request) {
    blocked_transactions_queue_[request->get_transaction_id()].insert(request);
}

void TransactionManager::evict_transaction(int tran_id) {
    transactions_set_.erase(transactions_[tran_id]);
    transactions_.erase(tran_id);
}

std::shared_ptr<Transaction> TransactionManager::get_transaction(repcrec::tran_id_t tran_id) const {
    return transactions_.at(tran_id);
}

std::shared_ptr<SiteManager> TransactionManager::get_site_manager() const {
    return site_manager_;
}

std::shared_ptr<LockManager> TransactionManager::get_lock_manager() const {
    return lock_manager_;
}

bool TransactionManager::is_all_instructions_finished() const {
    return transactions_.empty();
}

void TransactionManager::abort_transaction(repcrec::tran_id_t tran_id) {
    transactions_set_.erase(transactions_[tran_id]);
    transactions_.erase(tran_id);
    blocked_transactions_queue_.erase(tran_id);
    lock_manager_->release_locks(tran_id);
    printf("INFO: Abort transaction T%d\n", tran_id);
}
