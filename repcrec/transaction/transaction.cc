/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "transaction.h"
#include "../transaction_manager/transaction_manager.h"

repcrec::transaction::Transaction::Transaction(repcrec::tran_id_t transaction_id, repcrec::timestamp_t timestamp, bool is_read_only)
    : transaction_id_(transaction_id), timestamp_(timestamp), is_read_only_(is_read_only), waiting_var_id_(-1) {}

repcrec::tran_id_t repcrec::transaction::Transaction::get_transaction_id() const {
    return transaction_id_;
}

bool repcrec::transaction::Transaction::is_read_only() const {
    return is_read_only_;
}

repcrec::timestamp_t repcrec::transaction::Transaction::get_timestamp() const {
    return timestamp_;
}

bool repcrec::transaction::Transaction::commit(repcrec::timestamp_t commit_time) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    for (const auto& [site_id, _] : write_accessed_sites_) {
        if (!site_manager->get_site(site_id)->is_write_available()) {
            repcrec::transaction_manager::TransactionManager::get_instance().abort_transaction(transaction_id_);
            return false;
        }
    }
    for (const auto& [site_id, var_set] : read_accessed_sites_) {
        for (const auto& [var_id, var] : var_set) {
            if (!site_manager->get_site(site_id)->is_read_available(var_id)) {
                repcrec::transaction_manager::TransactionManager::get_instance().abort_transaction(transaction_id_);
                return false;
            }
        }
    }
    // TODO: Solve waiting
    for (const auto& [site_id, var_set] : write_records_) {
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        for (const auto& [var_id, var] : var_set) {
            site->assign_var(var_id, var, commit_time);
            site->set_read_available(var_id);
        }
    }
    return true;
}

void repcrec::transaction::Transaction::update_values(repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t var) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<repcrec::variable::Variable> var_ptr = site_manager->get_site(site_id)->get_variable(var_id);
    write_records_[site_id][var_id] = var;
    write_accessed_sites_[site_id][var_id] = var;
}

void repcrec::transaction::Transaction::add_read_history(repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t var) {
    read_accessed_sites_[site_id][var_id] = var;
}

repcrec::var_id_t repcrec::transaction::Transaction::get_waiting_var_id() const {
    return waiting_var_id_;
}

void repcrec::transaction::Transaction::set_waiting_var_id(repcrec::var_id_t var_id) {
    waiting_var_id_ = var_id;
}

bool repcrec::transaction::Transaction::is_written_into_site(repcrec::site_id_t site_id) {
    return write_accessed_sites_.contains(site_id);
}