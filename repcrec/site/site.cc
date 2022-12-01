/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "site.h"

repcrec::site_id_t repcrec::site::Site::increase_id_ = 1;
std::shared_ptr<std::ofstream> repcrec::site::Site::write_to_file_= nullptr;

repcrec::site::Site::Site() : id_(increase_id_++), write_available_(true) {
    for (repcrec::var_id_t var_id = 1; var_id <= repcrec::VAR_COUNT; ++var_id) {
        variables_[var_id] = std::make_shared<repcrec::variable::Variable>(var_id);
        read_available_map_[var_id] = true;
    }
}

bool repcrec::site::Site::is_read_available(repcrec::var_id_t var_id) {
    return read_available_map_[var_id];
}

bool repcrec::site::Site::is_write_available() const {
    return write_available_;
}

repcrec::site_id_t repcrec::site::Site::get_site_id() const {
    return id_;
}

void repcrec::site::Site::dump() const {
    if(!use_file()) {
        printf("site %2d - ", id_);
    }else{
        (*write_to_file_)<<"site"<<std::setw(2)<<id_<<" - ";
    };
    for (auto iter = variables_.begin(); iter != variables_.end(); ++iter) {
        if(iter->second->get_id() % 2 == 1 && id_ != iter->second->get_id() % 10 + 1){
            if(!use_file()){
                printf("         \t");
            }else{
                (*write_to_file_)<<"         \t";
            }
            continue;
        }
        if (iter == prev(end(variables_))) {
            if(!use_file()) {
                printf("x%2d:%5d\n", iter->second->get_id(), iter->second->get_value());
            }else{
                (*write_to_file_)<<"x"<<std::setw(2)<<iter->second->get_id()<<":"<<std::setw(5)<<iter->second->get_value()<<"\n";
            }
        } else {
            if(!use_file()){
                printf("x%2d:%5d\t", iter->second->get_id(), iter->second->get_value());
            }else{
                (*write_to_file_)<<"x"<<std::setw(2)<<iter->second->get_id()<<":"<<std::setw(5)<<iter->second->get_value()<<"\t";
            }
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
    for (repcrec::var_id_t var_id = 1; var_id <= repcrec::VAR_COUNT; ++var_id) {
        read_available_map_[var_id] = false;
    }
    write_available_ = false;
}

void repcrec::site::Site::set_read_available(repcrec::var_id_t var_id) {
    read_available_map_[var_id] = true;
}

void repcrec::site::Site::set_write_available() {
    write_available_ = true;
}
