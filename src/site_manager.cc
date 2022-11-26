//
// Created by 梁俊华 on 11/21/22.
//

#include "site_manager.h"

SiteManager::SiteManager() {
    for (int i = 1; i <= 10 ; ++i) {
        sites_[i] = std::make_unique<Site>();
    }
}

SiteManager::~SiteManager() = default;

void SiteManager::dump() const {
    for (const auto& [_, site] : sites_) {
        site->dump();
    }
}

std::shared_ptr<Site> SiteManager::get_site(int site_id) {
    return sites_[site_id];
}

void SiteManager::set_unavailable(int site_id) {
    sites_[site_id]->set_unavailable();
}
