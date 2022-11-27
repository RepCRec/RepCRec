//
// Created by 梁俊华 on 11/25/22.
//

#include "request.h"
#include "transaction_manager.h"

Request::Request(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : timestamp_(timestamp), tran_id_(tran_id), site_id_(site_id), var_id_(var_id), type_(InstructType::UNKNOWN) {}

repcrec::timestamp_t Request::get_timestamp() const {
    return timestamp_;
}

repcrec::timestamp_t Request::get_transaction_id() const {
    return tran_id_;
}

void Request::set_type(InstructType type) {
    type_ = type;
}

InstructType Request::get_type() const {
    return type_;
}

// Write
WriteRequest::WriteRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t value)
    : Request(timestamp, tran_id, site_id, var_id), value_(value) {}

void WriteRequest::exec() {
    std::shared_ptr<SiteManager> site_manager = TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<LockManager> lock_manager = TransactionManager::get_instance().get_lock_manager();
    auto [site_id_set, status, owner_ids] = lock_manager->try_acquire_write_lock(tran_id_, var_id_);
    if ((var_id_ & 0x01) == 1) {
        int site_id = 1 + var_id_ % repcrec::SITE_COUNT;
        std::shared_ptr<Site> site = site_manager->get_site(site_id);
        switch (status) {
            case repcrec::lock_status::SITE_UNAVAILABLE: {
                printf("INFO: T%d is unavailable to write value %d to site%d\n", tran_id_, value_, site_id);
                break;
            }
            case repcrec::lock_status::NEED_TO_WAIT: {
                printf("INFO: T%d is waiting for the locks of x%d at site%d.\n", tran_id_, var_id_, site_id);
                lock_manager->assign_wait_for_graph(tran_id_, owner_ids);
                TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<WriteRequest>(timestamp_, tran_id_, -1, var_id_, value_));
                break;
            }
            case repcrec::lock_status::HAS_WRITE_LOCK: {
                printf("INFO: T%d already has write lock on x%d\n", tran_id_, var_id_);
                TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                break;
            }
            case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                printf("INFO: T%d gets write lock on x%d on site%d\n", tran_id_, var_id_, site_id);
                TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                lock_manager->assign_write_lock(tran_id_, site_id_set, var_id_);
                break;
            }
            default: {
                throw std::invalid_argument("error status!");
            }
        }
    } else {
        switch (status) {
            case repcrec::lock_status::NEED_TO_WAIT: {
                printf("INFO: T%d is waiting for the locks of x%d.\n", tran_id_, var_id_);
                lock_manager->assign_wait_for_graph(tran_id_, owner_ids);
                TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<WriteRequest>(timestamp_, tran_id_, -1, var_id_, value_));
                break;
            }
            case repcrec::lock_status::HAS_WRITE_LOCK: {
                printf("INFO: T%d already has write lock on x%d\n", tran_id_, var_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    std::shared_ptr<Site> site = site_manager->get_site(site_id);
                    TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                }
                break;
            }
            case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                printf("INFO: T%d gets write lock on x%d\n", tran_id_, var_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    std::shared_ptr<Site> site = site_manager->get_site(site_id);
                    TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                }
                lock_manager->assign_write_lock(tran_id_, site_id_set, var_id_);
                break;
            }
            default: {
                throw std::invalid_argument("error status!");
            }
        }
    }
}

// Read
ReadRequest::ReadRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::var_id_t var_id) : Request(timestamp, tran_id, -1, var_id) {}

void ReadRequest::exec() {
    if (TransactionManager::get_instance().get_transaction(tran_id_)->is_read_only()) {
        handle_read_only_request();
    } else {
        handle_read_request();
    }
}

void ReadRequest::handle_read_only_request() {
    std::shared_ptr<SiteManager> site_manager = TransactionManager::get_instance().get_site_manager();
    if ((var_id_ & 0x01) == 1) {
        int site_id = 1 + var_id_ % 10;
        std::shared_ptr<Site> site = site_manager->get_site(site_id);
        if (site->is_available()) {
            printf("INFO(RO): T%d reads x%d=%d at site%d\n", tran_id_, var_id_, site->get_variable(var_id_)->get_value(), site_id);
        }
    } else {
        // TODO
    }
}

void ReadRequest::handle_read_request() {
    std::shared_ptr<SiteManager> site_manager = TransactionManager::get_instance().get_site_manager();
    if ((var_id_ % 0x01) == 1) {
        printf("INFO\n");
    } else {
        printf("INFO\n");
    }
}

// Create
CreateTransactionRequest::CreateTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, bool is_read_only, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, site_id, var_id), is_read_only_(is_read_only) {}

void CreateTransactionRequest::exec() {
    std::shared_ptr<Transaction> transaction = std::make_shared<Transaction>(tran_id_, timestamp_, is_read_only_);
    TransactionManager::get_instance().add_transaction(tran_id_, transaction);
    printf("INFO: Successfully created transaction T%d.\n", tran_id_);
}

// End
EndTransactionRequest::EndTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, site_id, var_id) {}

void EndTransactionRequest::exec() {
    if (TransactionManager::get_instance().get_transaction(tran_id_)->commit()) {
        std::shared_ptr<LockManager> lock_manager = TransactionManager::get_instance().get_lock_manager();
        lock_manager->release_locks(tran_id_);
        TransactionManager::get_instance().evict_transaction(tran_id_);
    }
}

// Dump
DumpRequest::DumpRequest(repcrec::timestamp_t timestamp) : Request(timestamp) {}

void DumpRequest::exec() {
    printf("INFO: Dump sites' information.\n");
    printf("====================================================\n");
    TransactionManager::get_instance().get_site_manager()->dump();
    printf("====================================================\n");
}

// Fail
FailRequest::FailRequest(repcrec::timestamp_t timestamp, repcrec::site_id_t site_id) : Request(timestamp, -1, site_id, -1) {}

void FailRequest::exec() {
    std::shared_ptr<SiteManager> site_manager_ = TransactionManager::get_instance().get_site_manager();
    site_manager_->get_site(site_id_)->set_unavailable();
}

// Recovery
RecoveryRequest::RecoveryRequest(repcrec::timestamp_t timestamp, repcrec::site_id_t site_id) : Request(timestamp, -1, site_id, -1) {}

void RecoveryRequest::exec() {
    std::shared_ptr<SiteManager> site_manager_ = TransactionManager::get_instance().get_site_manager();
    site_manager_->get_site(site_id_)->set_available();
}