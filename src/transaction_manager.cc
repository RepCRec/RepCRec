//
// Created by 梁俊华 on 11/21/22.
//

#include "transaction_manager.h"

TransactionManager::TransactionManager() : site_manager_(std::make_unique<SiteManager>()) {

}

TransactionManager::~TransactionManager() = default;

void TransactionManager::dump() const {
    site_manager_->dump();
}