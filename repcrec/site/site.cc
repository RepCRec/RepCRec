/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "site.h"

repcrec::site_id_t repcrec::site::Site::increase_id_ = 1;

repcrec::site::Site::Site() : id_(increase_id_++), read_available_(true), write_available_(true) {
    for (repcrec::var_id_t var_id = 1; var_id <= repcrec::VAR_COUNT; ++var_id) {
        variables_[var_id] = std::make_shared<repcrec::variable::Variable>(var_id);
    }
}

bool repcrec::site::Site::is_read_available() const {
    return read_available_;
}

bool repcrec::site::Site::is_write_available() const {
    return write_available_;
}

repcrec::site_id_t repcrec::site::Site::get_site_id() const {
    return id_;
}

void repcrec::site::Site::dump() const {
    printf("site %2d - ", id_);
    for (auto iter = variables_.begin(); iter != variables_.end(); ++iter) {
        if (iter == prev(end(variables_))) {
            printf("x%d: %d\n", iter->second->get_id(), iter->second->get_value());
        } else {
            printf("x%d: %d\t", iter->second->get_id(), iter->second->get_value());
        }
    }
}

std::shared_ptr<repcrec::variable::Variable> repcrec::site::Site::get_variable(repcrec::var_id_t var_id) {
    return variables_[var_id];
}

void repcrec::site::Site::assign_var(repcrec::var_id_t var_id, repcrec::var_t value, repcrec::timestamp_t timestamp) {
    variables_[var_id]->set_value(value);
    variables_[var_id]->set_latest_commit_time(timestamp);
}

void repcrec::site::Site::set_unavailable() {
    read_available_ = false;
    write_available_ = false;
}

void repcrec::site::Site::set_read_available() {
    read_available_ = true;
}

void repcrec::site::Site::set_write_available() {
    write_available_ = true;
}
