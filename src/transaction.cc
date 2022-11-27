//
// Created by 梁俊华 on 11/21/22.
//

#include "transaction.h"
#include "transaction_manager.h"

Transaction::Transaction(repcrec::tran_id_t transaction_id, repcrec::timestamp_t timestamp, bool is_read_only)
    : transaction_id_(transaction_id), timestamp_(timestamp), is_read_only_(is_read_only) {}

repcrec::tran_id_t Transaction::get_transaction_id() const {
    return transaction_id_;
}

bool Transaction::is_read_only() const {
    return is_read_only_;
}

repcrec::timestamp_t Transaction::get_timestamp() const {
    return timestamp_;
}

bool Transaction::commit() {
    std::shared_ptr<SiteManager> site_manager = TransactionManager::get_instance().get_site_manager();
    for (const auto& [site_id, _] :accessed_sites_) {
        if (!site_manager->get_site(site_id)->is_available()) {
            TransactionManager::get_instance().abort_transaction(transaction_id_);
            return false;
        }
    }
    for (const auto& [site_id, var_set] : write_records_) {
        std::shared_ptr<Site> site = site_manager->get_site(site_id);
        for (const auto& [var_id, var] : var_set) {
            site->assign_var(var_id, var);
        }
    }
    return true;
}

void Transaction::update_values(repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t var) {
    std::shared_ptr<SiteManager> site_manager = TransactionManager::get_instance().get_site_manager();
    std::shared_ptr<Variable> var_ptr = site_manager->get_site(site_id)->get_variable(var_id);
    write_records_[site_id][var_id] = var;
    accessed_sites_[site_id][var_id] = var_ptr;
}