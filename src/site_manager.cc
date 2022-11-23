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


