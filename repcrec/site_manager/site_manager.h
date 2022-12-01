/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

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

        std::shared_ptr<repcrec::site::Site> get_site(repcrec::site_id_t site_id);
        void set_unavailable(int site_id);

    private:
        std::map<repcrec::site_id_t , std::shared_ptr<repcrec::site::Site>> sites_;
    };
} // namespace repcrec::site_manager.

#endif//REPCREC_SITE_MANAGER_H
