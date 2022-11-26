//
// Created by 梁俊华 on 11/25/22.
//

#include "request.h"
#include "transaction_manager.h"

Request::Request(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : timestamp_(timestamp), tran_id_(tran_id), site_id_(site_id), var_id_(var_id) {}

// Write
WriteRequest::WriteRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t value)
    : Request(timestamp, tran_id, site_id, var_id), value_(value) {

}

void WriteRequest::exec() {
    std::shared_ptr<SiteManager> site_manager_ = TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<LockManager> lock_manager_ = TransactionManager::get_instance().get_lock_manager();
    if ((var_id_ & 0x01) == 1) {
        int site_id = 1 + var_id_ % repcrec::SITE_COUNT;
        std::shared_ptr<Site> site = site_manager_->get_site(site_id);
        auto [site_id_set, status] = lock_manager_->try_acquire_write_lock(tran_id_, var_id_);
        switch (status) {
            case repcrec::lock_status::SITE_UNAVAILABLE: {
                printf("INFO: T%d is unavailable to write value %d to site %d\n", tran_id_, value_, site_id);
                break;
            }
            case repcrec::lock_status::NEED_TO_WAIT: {
                break;
            }
            case repcrec::lock_status::HAS_WRITE_LOCK: {
                printf("INFO: T%d already has write lock on var %d\n", tran_id_, value_);
                site->assign_var(var_id_, value_);
                break;
            }
            case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                printf("INFO: T%d gets write lock on var %d on site %d\n", tran_id_, value_, site_id);
                site->assign_var(var_id_, value_);
                break;
            }
            default: {
                throw std::invalid_argument("error status!");
            }
        }
    } else {
        for (int site_id = 1; site_id <= repcrec::SITE_COUNT; ++site_id) {
            site_manager_->get_site(site_id)->assign_var(var_id_, value_);
        }
    }
}

// Read
ReadRequest::ReadRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::var_id_t var_id) : Request(timestamp, tran_id, -1, var_id) {}

void ReadRequest::exec() {}

// Create
CreateTransactionRequest::CreateTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, bool is_read_only, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, site_id, var_id), is_read_only_(is_read_only) {}

void CreateTransactionRequest::exec() {
    std::shared_ptr<Transaction> transaction = std::make_shared<Transaction>(tran_id_, is_read_only_);
    TransactionManager::get_instance().add_transaction(tran_id_, transaction);
    printf("INFO: Successfully created transaction T%d.\n", tran_id_);
}

// End
EndTransactionRequest::EndTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, site_id, var_id) {}

void EndTransactionRequest::exec() {
    TransactionManager::get_instance().evict_transaction(tran_id_);
}

// Dump
DumpRequest::DumpRequest(repcrec::timestamp_t timestamp) : Request(timestamp) {}

void DumpRequest::exec() {
    TransactionManager::get_instance().get_site_manager()->dump();
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
