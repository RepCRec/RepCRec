//
// Created by 梁俊华 on 11/21/22.
//

#ifndef REPCREC_SITE_MANAGER_H
#define REPCREC_SITE_MANAGER_H

#include <map>
#include <memory>
#include "site.h"

class SiteManager {
public:
    SiteManager();
    ~SiteManager();

    void dump() const;

private:
    std::map<int, std::unique_ptr<Site>> sites_;
};


#endif//REPCREC_SITE_MANAGER_H
