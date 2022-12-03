/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#ifndef REPCREC_SITE_H
#define REPCREC_SITE_H

#include <fstream>
#include <map>
#include <memory>

#include "../global.h"
#include "../variable/variable.h"

namespace repcrec::site {
    class Site {
    public:
        Site();
        ~Site() = default;

        static void set_file(const std::shared_ptr<std::ofstream> &file) {
            write_to_file_ = file;
        }

        static bool use_file() {
            return write_to_file_ != nullptr;
        }

        [[nodiscard]] bool is_read_available(repcrec::var_id_t);
        [[nodiscard]] bool is_write_available() const;
        void dump() const;

        void assign_var(repcrec::var_id_t var_id, repcrec::var_t value, repcrec::timestamp_t timestamp);
        void set_unavailable();
        void set_read_available(repcrec::var_id_t);
        void set_write_available();

        std::shared_ptr<repcrec::variable::Variable> get_variable(repcrec::var_id_t var_id);

    private:
        repcrec::site_id_t id_;
        bool write_available_;
        std::unordered_map<repcrec::var_id_t, bool> read_available_map_;
        std::map<repcrec::var_id_t, std::shared_ptr<repcrec::variable::Variable>> variables_;
        static repcrec::site_id_t increase_id_;
        static std::shared_ptr<std::ofstream> write_to_file_;
    };
}// namespace repcrec::site.

#endif// REPCREC_SITE_H