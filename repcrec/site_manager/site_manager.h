//
// Created by 梁俊华 on 11/21/22.
//

#ifndef REPCREC_SITE_MANAGER_H
#define REPCREC_SITE_MANAGER_H

#include <map>
#include <memory>

#include "../site/site.h"

namespace repcrec::site_manager {
    class SiteManager {
    public:
        SiteManager();
        ~SiteManager() = default;

        void dump() const;

        std::shared_ptr<repcrec::site::Site> get_site(int site_id);
        void set_unavailable(int site_id);

    private:
        std::map<int, std::shared_ptr<repcrec::site::Site>> sites_;
    };
}



#endif//REPCREC_SITE_MANAGER_H
