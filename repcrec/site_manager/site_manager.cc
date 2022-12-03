/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include "site_manager.h"

std::shared_ptr<std::ofstream> repcrec::site_manager::SiteManager::write_to_file_= nullptr;

repcrec::site_manager::SiteManager::SiteManager() {
    for (int i = 1; i <= 10 ; ++i) {
        sites_[i] = std::make_shared<repcrec::site::Site>();
    }
}

void repcrec::site_manager::SiteManager::dump() const {
    if(use_file()){
        (*write_to_file_)<<"====================================================\n";
    }else{
        printf("====================================================\n");
    }
    for (const auto& [_, site] : sites_) {
        site->dump();
    }
    if(use_file()){
        (*write_to_file_)<<"====================================================\n";
    }else {
        printf("====================================================\n");
    }
}

std::shared_ptr<repcrec::site::Site> repcrec::site_manager::SiteManager::get_site(repcrec::site_id_t site_id) {
    return sites_[site_id];
}

void repcrec::site_manager::SiteManager::set_unavailable(int site_id) {
    sites_[site_id]->set_unavailable();
}
