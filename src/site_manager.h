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

    std::shared_ptr<Site> get_site(int site_id);
    void set_unavailable(int site_id);

private:
    std::map<int, std::shared_ptr<Site>> sites_;
};


#endif//REPCREC_SITE_MANAGER_H
