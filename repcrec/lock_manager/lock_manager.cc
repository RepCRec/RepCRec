/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "lock_manager.h"

#include "../transaction_manager/transaction_manager.h"

repcrec::LockStatus repcrec::lock_manager::LockManager::try_acquire_write_lock(repcrec::tran_id_t tran_id, repcrec::var_id_t var_id) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    repcrec::LockStatus lock_status;
    if ((var_id & 0x01) == 1) {
        repcrec::site_id_t site_id = 1 + (var_id) % 10;
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        if (!site->is_write_available()) {
            lock_status.status = repcrec::lock_status::SITE_UNAVAILABLE;
            return lock_status;
        }
        std::shared_ptr<repcrec::variable::Variable> var = site->get_variable(var_id);
        if (var != nullptr
            and var->has_shared_lock(tran_id)
            and !var->has_shared_lock_exclude_self(tran_id)
            and !var->has_shared_lock_exclude_self(tran_id)
            and !is_waiting_for_others(tran_id)) {
            lock_status.site_id_set.insert(site_id);
            lock_status.status = repcrec::lock_status::HAS_READ_LOCK;
            return lock_status;
        }
        if (var != nullptr and
            (var->has_exclusive_lock_exclude_self(tran_id)
             or var->has_shared_lock_exclude_self(tran_id)
             or (var->has_shared_lock(tran_id)))) {
            lock_status.status = repcrec::lock_status::NEED_TO_WAIT;
            if (var->has_exclusive_lock_exclude_self(tran_id)) {
                lock_status.owner_id_set.insert(var->get_exclusive_lock_owner()->get_transaction_id());
            }
            if (var->has_shared_lock_exclude_self(tran_id)) {
                for (const auto& [shared_owner_id, _] : var->get_shared_lock_owners()) {
                    lock_status.owner_id_set.insert(shared_owner_id);
                }
            }
            if (var->has_shared_lock(tran_id)) {
                lock_status.owner_id_set.insert(tran_id);
            }
            return lock_status;
        }
        if (var != nullptr and var->has_exclusive_lock(tran_id)) {
            lock_status.status = repcrec::lock_status::HAS_WRITE_LOCK;
            return lock_status;
        }
        lock_status.site_id_set.insert(site_id);
        lock_status.status = repcrec::lock_status::AVAILABLE_TO_ASSIGN;
        return lock_status;
    }

    lock_status.status = repcrec::lock_status::AVAILABLE_TO_ASSIGN;
    for (repcrec::site_id_t sid = 1; sid <= repcrec::SITE_COUNT; ++sid) {
        if (!site_manager->get_site(sid)->is_write_available()) {
            continue;
        }
        std::shared_ptr<repcrec::variable::Variable> var = site_manager->get_site(sid)->get_variable(var_id);
        if (var != nullptr
            and var->has_shared_lock(tran_id)
            and !var->has_shared_lock_exclude_self(tran_id)
            and !var->has_exclusive_lock_exclude_self(tran_id) and !is_waiting_for_others(tran_id)) {
            lock_status.status = repcrec::lock_status::HAS_READ_LOCK;
            lock_status.site_id_set.insert(sid);
            continue;
        }
        if (var != nullptr and
            (var->has_shared_lock_exclude_self(tran_id)
             or var->has_exclusive_lock_exclude_self(tran_id)
             or var->has_shared_lock(tran_id))) {
            lock_status.status = repcrec::lock_status::NEED_TO_WAIT;
            lock_status.site_id_set.clear();
            if (var->has_exclusive_lock_exclude_self(tran_id)) {
                lock_status.owner_id_set.insert(var->get_exclusive_lock_owner()->get_transaction_id());
            }
            if (var->has_shared_lock_exclude_self(tran_id)) {
                for (const auto& [shared_owner_id, _] : var->get_shared_lock_owners()) {
                    lock_status.owner_id_set.insert(shared_owner_id);
                }
            }
            if (var->has_shared_lock(tran_id)) {
                lock_status.owner_id_set.insert(tran_id);
            }
            return lock_status;
        }
        lock_status.site_id_set.insert(sid);
        if (var != nullptr and var->has_exclusive_lock(tran_id)) {
            lock_status.status = repcrec::lock_status::HAS_WRITE_LOCK;
            lock_status.site_id_set.clear();
            return lock_status;
        }
    }

    return lock_status;
}

repcrec::LockStatus repcrec::lock_manager::LockManager::try_acquire_read_lock(repcrec::tran_id_t tran_id, repcrec::var_id_t var_id) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    repcrec::LockStatus lock_status;
    if ((var_id & 0x01) == 1) {
        repcrec::site_id_t site_id = 1 + (var_id) % 10;
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        if (!site->is_read_available(var_id)) {
            lock_status.status = repcrec::lock_status::SITE_UNAVAILABLE;
            return lock_status;
        }
        std::shared_ptr<repcrec::variable::Variable> var = site->get_variable(var_id);
        if (var->has_exclusive_lock_exclude_self(tran_id)) {
            lock_status.status = repcrec::lock_status::NEED_TO_WAIT;
            return lock_status;
        }
        if (var->has_exclusive_lock(tran_id) or var->has_shared_lock(tran_id)) {
            lock_status.status = repcrec::lock_status::HAS_READ_WRITE_LOCK;
            return lock_status;
        }
        lock_status.site_id_set.insert(site_id);
        lock_status.status = repcrec::lock_status::AVAILABLE_TO_ASSIGN;
        return lock_status;
    }

    for (repcrec::site_id_t sid = 1; sid <= repcrec::SITE_COUNT; ++sid) {
        if (!site_manager->get_site(sid)->is_write_available()) {
            continue;
        }
        std::shared_ptr<repcrec::variable::Variable> var = site_manager->get_site(sid)->get_variable(var_id);
        if (var->has_exclusive_lock_exclude_self(tran_id)) {
            lock_status.status = repcrec::lock_status::NEED_TO_WAIT;
            lock_status.site_id_set.clear();
            return lock_status;
        }
        lock_status.site_id_set.insert(sid);
        if (var->has_exclusive_lock(tran_id) or var->has_shared_lock(tran_id)) {
            lock_status.status = repcrec::lock_status::HAS_READ_WRITE_LOCK;
            lock_status.site_id_set.clear();
            return lock_status;
        }
    }
    lock_status.status = repcrec::lock_status::AVAILABLE_TO_ASSIGN;
    return lock_status;
}

void repcrec::lock_manager::LockManager::release_locks(repcrec::tran_id_t tran_id) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    for (const repcrec::var_id_t& vid : lock_table_[tran_id]) {
        if ((vid & 0x01) == 1) {
            repcrec::site_id_t site_index = 1 + (vid % 10);
            std::shared_ptr<repcrec::variable::Variable> var = site_manager->get_site(site_index)->get_variable(vid);
            if (var->has_exclusive_lock(tran_id)) {
                var->remove_exclusive_owner();
            }
            if (var->has_shared_lock(tran_id)) {
                var->remove_shared_owner(tran_id);
            }
        } else {
            for (repcrec::site_id_t sid = 1; sid <= repcrec::SITE_COUNT; ++sid) {
                std::shared_ptr<repcrec::variable::Variable> var = site_manager->get_site(sid)->get_variable(vid);
                if (var->has_exclusive_lock(tran_id)) {
                    var->remove_exclusive_owner();
                }
                if (var->has_shared_lock(tran_id)) {
                    var->remove_shared_owner(tran_id);
                }
            }
        }
    }
    std::unordered_set<repcrec::tran_id_t> empty_ids;
    for (auto& [tid, tids] : wait_for_graph_) {
        if (tids.count(tran_id)) {
            tids.erase(tran_id);
        }
        if (tids.empty()) {
            empty_ids.insert(tid);
        }
    }
    for (const repcrec::tran_id_t& tid : empty_ids) {
        wait_for_graph_.erase(tid);
    }
    wait_for_graph_.erase(tran_id);
    lock_table_.erase(tran_id);
    // printf("INFO: T%d releases all its locks.\n", tran_id);
}

void repcrec::lock_manager::LockManager::assign_write_lock(repcrec::tran_id_t tran_id, std::unordered_set<repcrec::site_id_t>& site_id_set, repcrec::var_id_t var_id) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    for (const repcrec::site_id_t site_id : site_id_set) {
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        if (site->is_write_available()) {
            std::shared_ptr<repcrec::variable::Variable> var = site->get_variable(var_id);
            if (var->has_shared_lock(tran_id)) {
                var->remove_shared_owner(tran_id);
                // remove_self_from_wait_for_graph(tran_id);
            }
            var->set_exclusive_transaction(repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id));
            lock_table_[tran_id].insert(var_id);
        }
    }
}

void repcrec::lock_manager::LockManager::assign_share_lock(repcrec::tran_id_t tran_id, std::unordered_set<repcrec::site_id_t>& site_id_set, repcrec::var_id_t var_id) {
    std::shared_ptr<repcrec::site_manager::SiteManager> site_manager = repcrec::transaction_manager::TransactionManager::get_instance().get_site_manager();
    for (const repcrec::site_id_t site_id : site_id_set) {
        std::shared_ptr<repcrec::site::Site> site = site_manager->get_site(site_id);
        if (site->is_read_available(var_id)) {
            lock_table_[tran_id].insert(var_id);
            std::shared_ptr<repcrec::variable::Variable> var = site->get_variable(var_id);
            var->add_shared_transaction(repcrec::transaction_manager::TransactionManager::get_instance().get_transaction(tran_id));
        }
    }
}

void repcrec::lock_manager::LockManager::assign_wait_for_graph(repcrec::tran_id_t tran_id, std::unordered_set<repcrec::site_id_t>& owner_ids) {
    for (const repcrec::tran_id_t& owner_id : owner_ids) {
        wait_for_graph_[tran_id].insert(owner_id);
    }
}

bool repcrec::lock_manager::LockManager::is_waiting_for_lock(repcrec::tran_id_t tran_id) const {
    return wait_for_graph_.count(tran_id) and !wait_for_graph_.at(tran_id).empty();
}

bool repcrec::lock_manager::LockManager::detect_deadlock() {
    std::unordered_map<repcrec::tran_id_t, dfs_status> visited;
    std::vector<repcrec::tran_id_t> tran_set;
    for (const auto& [curr_tran_id, _] : wait_for_graph_) {
        tran_set.push_back(curr_tran_id);
        visited[curr_tran_id] = dfs_status::NOT_VISIT;
    }
    bool has_cycle = false;
    for (const repcrec::tran_id_t& tid : tran_set) {
        if (visited[tid] == dfs_status::NOT_VISIT) {
            wait_for_graph_dfs(tid, has_cycle, visited);
            if (has_cycle) {
                return true;
            }
        }
    }
    return false;
}

void repcrec::lock_manager::LockManager::wait_for_graph_dfs(repcrec::tran_id_t curr_tran_id, bool& has_cycle, std::unordered_map<repcrec::tran_id_t, dfs_status>& visited) {
    if (visited[curr_tran_id] == dfs_status::VISITED or has_cycle) {
        return;
    }

    visited[curr_tran_id] = dfs_status::ON_PATH;
    for (const repcrec::tran_id_t next_tran_id : wait_for_graph_[curr_tran_id]) {
        if (visited[next_tran_id] == dfs_status::ON_PATH) {
            has_cycle = true;
            return;
        }
        if (visited[next_tran_id] == dfs_status::NOT_VISIT) {
            wait_for_graph_dfs(next_tran_id, has_cycle, visited);
        }
        if (has_cycle) {
            return;
        }
    }
    visited[curr_tran_id] = dfs_status::VISITED;
}

void repcrec::lock_manager::LockManager::remove_self_from_wait_for_graph(repcrec::tran_id_t tran_id) {
    if (wait_for_graph_.count(tran_id) and wait_for_graph_[tran_id].count(tran_id)) {
        wait_for_graph_[tran_id].erase(tran_id);
        if (wait_for_graph_[tran_id].empty()) {
            wait_for_graph_.erase(tran_id);
        }
    }
}

bool repcrec::lock_manager::LockManager::is_waiting_for_others(repcrec::tran_id_t tran_id) const {
    for (const auto& [tid, tids] : wait_for_graph_) {
        if (tid != tran_id and tids.count(tran_id)) {
            return true;
        }
    }
    return false;
}