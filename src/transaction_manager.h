//
// Created by 梁俊华 on 11/21/22.
//

#ifndef REPCREC_TRANSACTION_MANAGER_H
#define REPCREC_TRANSACTION_MANAGER_H

#include "site_manager.h"
#include <memory>

class TransactionManager {
public:
    TransactionManager();
    ~TransactionManager();

    void dump() const;

private:
    std::unique_ptr<SiteManager> site_manager_;
};


#endif//REPCREC_TRANSACTION_MANAGER_H
