/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "request.h"

#include "../transaction_manager/transaction_manager.h"

repcrec::request::Request::Request(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : timestamp_(timestamp), tran_id_(tran_id), site_id_(site_id), var_id_(var_id), type_(repcrec::instruction::InstructType::UNKNOWN) {}

repcrec::timestamp_t repcrec::request::Request::get_timestamp() const {
    return timestamp_;
}

repcrec::timestamp_t repcrec::request::Request::get_transaction_id() const {
    return tran_id_;
}

void repcrec::request::Request::set_type(repcrec::instruction::InstructType type) {
    type_ = type;
}

repcrec::instruction::InstructType repcrec::request::Request::get_type() const {
    return type_;
}

// Write
repcrec::request::WriteRequest::WriteRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t value)
    : Request(timestamp, tran_id, site_id, var_id), value_(value) {}

void repcrec::request::WriteRequest::exec() {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<repcrec::lock_manager::LockManager> lock_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_lock_manager();
    auto [site_id_set, status, owner_ids] = lock_manager->try_acquire_write_lock(tran_id_, var_id_);
    if ((var_id_ & 0x01) == 1) {
        int site_id = 1 + var_id_ % repcrec::SITE_COUNT;
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        switch (status) {
            case repcrec::lock_status::SITE_UNAVAILABLE: {
                printf("INFO (%d): T%d is unavailable to write value %d to site%d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, value_, site_id);
                break;
            }
            case repcrec::lock_status::NEED_TO_WAIT: {
                printf("INFO (%d): T%d is waiting for the locks of x%d at site%d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site_id);
                lock_manager->assign_wait_for_graph(tran_id_, owner_ids);
                repcrec::transaction_manager::TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<WriteRequest>(timestamp_, tran_id_, -1, var_id_, value_));
                break;
            }
            case repcrec::lock_status::HAS_WRITE_LOCK: {
                printf("INFO (%d): T%d already has write lock on x%d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                break;
            }
            case repcrec::lock_status::HAS_READ_LOCK: {
                printf("INFO (%d): T%d promotes the shared lock to exclusive lock.",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_);
                repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                lock_manager->assign_write_lock(tran_id_, site_id_set, var_id_);
                printf("INFO (%d): T%d updates x%d to %d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, value_);
                break;
            }
            case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                printf("INFO (%d): T%d gets write lock on x%d on site%d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site_id);
                repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                lock_manager->assign_write_lock(tran_id_, site_id_set, var_id_);
                printf("INFO (%d): T%d updates x%d to %d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, value_);
                break;
            }
            default: {
                throw std::invalid_argument("error status!");
            }
        }
    } else {
        switch (status) {
            case repcrec::lock_status::NEED_TO_WAIT: {
                printf("INFO (%d): T%d is waiting for the locks of x%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                lock_manager->assign_wait_for_graph(tran_id_, owner_ids);
                repcrec::transaction_manager::TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<WriteRequest>(timestamp_, tran_id_, -1, var_id_, value_));
                break;
            }
            case repcrec::lock_status::HAS_WRITE_LOCK: {
                printf("INFO (%d): T%d already has exclusive lock on x%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
                    repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                }
                break;
            }
            case repcrec::lock_status::HAS_READ_LOCK: {
                printf("INFO (%d): T%d promotes the shared lock to exclusive lock.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                }
                printf("INFO (%d): T%d updates x%d to %d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, value_);
                lock_manager->assign_write_lock(tran_id_, site_id_set, var_id_);
                break;
            }
            case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                printf("INFO (%d): T%d gets write lock on x%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->update_values(site_id, var_id_, value_);
                }
                printf("INFO (%d): T%d updates x%d to %d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, value_);
                lock_manager->assign_write_lock(tran_id_, site_id_set, var_id_);
                break;
            }
            default: {
                printf("%d\n", status);
                throw std::invalid_argument("error status!");
            }
        }
    }
}

// Read
repcrec::request::ReadRequest::ReadRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, -1, var_id) {}

void repcrec::request::ReadRequest::exec() {
    if (repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->is_read_only()) {
        handle_read_only_request();
    } else {
        handle_read_request();
    }
}

void repcrec::request::ReadRequest::handle_read_only_request() {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<repcrec::transaction::Transaction> transaction = repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_);
    if ((var_id_ & 0x01) == 1) {
        repcrec::site_id_t site_id = 1 + var_id_ % 10;
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        if (site->is_read_available(var_id_)) {
            printf("INFO (%d): T%d (RO) reads x%d = %d at site%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site->get_variable(var_id_)->get_value(transaction->get_timestamp()), site_id);
            transaction->add_read_history(site_id, var_id_, site->get_variable(var_id_)->get_value(transaction->get_timestamp()));
        } else {
            // Transaction must wait for the site.
            if (!repcrec::transaction_manager::TransactionManager::get_instance().is_transaction_waiting_for_site(tran_id_)) {
                transaction->set_waiting_var_id(var_id_);
                repcrec::transaction_manager::TransactionManager::get_instance().add_to_site_waiting_map(tran_id_, site_id);
            }
            repcrec::transaction_manager::TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<ReadRequest>(timestamp_, tran_id_, var_id_));
        }
    } else {
        for (repcrec::site_id_t site_id = 1; site_id <= repcrec::SITE_COUNT; ++site_id) {
            std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
            if (site->is_read_available(var_id_)) {
                printf("INFO (%d): T%d (RO) reads x%d = %d.\n",
                       repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site->get_variable(var_id_)->get_value(transaction->get_timestamp()));
                transaction->add_read_history(site_id, var_id_, site->get_variable(var_id_)->get_value(transaction->get_timestamp()));
                return;
            }
        }
        // Transaction abort when all sites are down.
        repcrec::transaction_manager::TransactionManager::get_instance().abort_transaction(tran_id_);
    }
}

void repcrec::request::ReadRequest::handle_read_request() {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<repcrec::lock_manager::LockManager> lock_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_lock_manager();
    std::shared_ptr<repcrec::transaction::Transaction> transaction = repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_);
    if ((var_id_ & 0x01) == 1) {
        repcrec::site_id_t site_id = 1 + var_id_ % 10;
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        if (site->is_read_available(var_id_)) {
            auto [site_id_set, status, owner_ids] = lock_manager->try_acquire_read_lock(tran_id_, var_id_);
            switch (status) {
                case repcrec::lock_status::SITE_UNAVAILABLE: {
                    printf("INFO (%d): T%d is unavailable to read x%d at site%d and blocked.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site_id);
                    break;
                }
                case repcrec::lock_status::NEED_TO_WAIT: {
                    printf("INFO (%d): T%d is waiting for the locks of x%d at site%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site_id);
                    lock_manager->assign_wait_for_graph(tran_id_, owner_ids);
                    repcrec::transaction_manager::TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<ReadRequest>(timestamp_, tran_id_, var_id_));
                    break;
                }
                case repcrec::lock_status::HAS_READ_WRITE_LOCK: {
                    transaction->add_read_history(site_id, var_id_, site->get_variable(var_id_)->get_value());
                    printf("INFO (%d): T%d reads x%d=%d at site%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site->get_variable(var_id_)->get_value(), site_id);
                    break;
                }
                case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                    lock_manager->assign_share_lock(tran_id_, site_id_set, var_id_);
                    transaction->add_read_history(site_id, var_id_, site->get_variable(var_id_)->get_value());
                    printf("INFO (%d): T%d reads x%d=%d at site%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site->get_variable(var_id_)->get_value(), site_id);
                    break;
                }
                default: {
                    throw std::invalid_argument("error status!");
                }
            }
        } else {
            if (!repcrec::transaction_manager::TransactionManager::get_instance().is_transaction_waiting_for_site(tran_id_)) {
                transaction->set_waiting_var_id(var_id_);
                repcrec::transaction_manager::TransactionManager::get_instance().add_to_site_waiting_map(tran_id_, site_id);
            }
            repcrec::transaction_manager::TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<ReadRequest>(timestamp_, tran_id_, var_id_));
        }
    } else {
        auto [site_id_set, status, owner_ids] = lock_manager->try_acquire_read_lock(tran_id_, var_id_);
        switch (status) {
            case repcrec::lock_status::NEED_TO_WAIT: {
                printf("INFO (%d): T%d is waiting for the locks of x%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                lock_manager->assign_wait_for_graph(tran_id_, owner_ids);
                repcrec::transaction_manager::TransactionManager::get_instance().add_request_to_blocked_queue(std::make_shared<ReadRequest>(timestamp_, tran_id_, var_id_));
                break;
            }
            case repcrec::lock_status::HAS_READ_WRITE_LOCK: {
                printf("INFO (%d): T%d already has read-write lock on x%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
                    printf("INFO (%d): T%d reads x%d=%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site->get_variable(var_id_)->get_value());
                    break;
                }
                break;
            }
            case repcrec::lock_status::AVAILABLE_TO_ASSIGN: {
                printf("INFO (%d): T%d gets shared lock on x%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_);
                for (const repcrec::site_id_t site_id : site_id_set) {
                    std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
                    printf("INFO (%d): T%d reads x%d=%d.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_, var_id_, site->get_variable(var_id_)->get_value());
                    break;
                }
                lock_manager->assign_share_lock(tran_id_, site_id_set, var_id_);
                break;
            }
            default: {
                throw std::invalid_argument("error status!");
            }
        }
    }
}

// Create
repcrec::request::CreateTransactionRequest::CreateTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, bool is_read_only, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, site_id, var_id), is_read_only_(is_read_only) {}

void repcrec::request::CreateTransactionRequest::exec() {
    std::shared_ptr<repcrec::transaction::Transaction> transaction = std::make_shared<repcrec::transaction::Transaction>(tran_id_, timestamp_, is_read_only_);
    repcrec::transaction_manager::TransactionManager::get_instance().add_transaction(tran_id_, transaction);
    printf("INFO (%d): Create T%d.\n", timestamp_, tran_id_);
}

// End
repcrec::request::EndTransactionRequest::EndTransactionRequest(repcrec::timestamp_t timestamp, repcrec::tran_id_t tran_id, repcrec::site_id_t site_id, repcrec::var_id_t var_id)
    : Request(timestamp, tran_id, site_id, var_id) {}

void repcrec::request::EndTransactionRequest::exec() {
    if (repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id_)->commit(timestamp_)) {
        std::shared_ptr<repcrec::lock_manager::LockManager> lock_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_lock_manager();
        lock_manager->release_locks(tran_id_);
        repcrec::transaction_manager::TransactionManager::get_instance().evict_transaction(tran_id_);
        printf("INFO (%d): T%d committed.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_);
    } else {
        printf("INFO (%d): T%d Commit Failed!\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, tran_id_);
    }
}

// Dump
repcrec::request::DumpRequest::DumpRequest(repcrec::timestamp_t timestamp) : Request(timestamp) {}

void repcrec::request::DumpRequest::exec() {
    repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager()->dump();
}

// Fail
repcrec::request::FailRequest::FailRequest(repcrec::timestamp_t timestamp, repcrec::site_id_t site_id) : Request(timestamp, -1, site_id, -1) {}

void repcrec::request::FailRequest::exec() {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<repcrec::lock_manager::LockManager> lock_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_lock_manager();
    std::unordered_map<repcrec::tran_id_t, std::shared_ptr<repcrec::transaction::Transaction>> transactions
            = repcrec::transaction_manager::TransactionManager::get_instance().get_transactions();
    std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id_);
    site->set_unavailable();

    std::unordered_set<repcrec::tran_id_t> remove_transaction_id_set;
    for (const auto& [tid, transaction] : transactions) {
        if (transaction->is_written_into_site(site_id_)) {
            remove_transaction_id_set.insert(tid);
        }
    }
    for (const repcrec::tran_id_t& tid : remove_transaction_id_set) {
        repcrec::transaction_manager::TransactionManager::get_instance().abort_transaction(tid);
    }

    printf("INFO (%d): Site-%d is failed.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, site_id_);
}

// Recovery
repcrec::request::RecoveryRequest::RecoveryRequest(repcrec::timestamp_t timestamp, repcrec::site_id_t site_id) : Request(timestamp, -1, site_id, -1) {}

void repcrec::request::RecoveryRequest::exec() {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager_
            = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    site_manager_->get_site(site_id_)->set_write_available();
    repcrec::var_id_t unique_id_1 = site_id_ - 1;
    repcrec::var_id_t unique_id_2 = unique_id_1 + 10;
    if ((unique_id_1 & 0x01) == 1) {
        site_manager_->get_site(site_id_)->set_read_available(unique_id_1);
        site_manager_->get_site(site_id_)->set_read_available(unique_id_2);
    }
    repcrec::transaction_manager::TransactionManager::get_instance().remove_from_site_waiting_map(site_id_);
    printf("INFO (%d): Site-%d has been recovered.\n", repcrec::transaction_manager::TransactionManager::curr_timestamp, site_id_);
}