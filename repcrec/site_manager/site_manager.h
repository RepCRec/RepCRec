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

namespace repcrec {
    namespace site_manager {

        class SiteManager {
        public:
            SiteManager();
            ~SiteManager() = default;

            static void set_file(const std::shared_ptr<std::ofstream> &file) {
                write_to_file_ = file;
                site::Site::set_file(file);
            }
            static bool use_file() {
                return write_to_file_ != nullptr;
            }

            void dump() const;

            std::shared_ptr<repcrec::site::Site> get_site(repcrec::site_id_t site_id);
            void set_unavailable(int site_id);

        private:
            std::map<repcrec::site_id_t, std::shared_ptr<repcrec::site::Site>> sites_;
            static std::shared_ptr<std::ofstream> write_to_file_;
        };
    }// namespace site_manager
}// namespace repcrec

#endif//REPCREC_SITE_MANAGER_H
