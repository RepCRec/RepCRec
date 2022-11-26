//
// Created by 梁俊华 on 11/22/22.
//
#include "variable.h"
#include "global.h"
#include <map>

class Site {
public:
    explicit Site();

    [[nodiscard]] bool is_available() const;
    [[nodiscard]] repcrec::site_id_t get_site_id() const;
    void dump() const;

    void assign_var(repcrec::var_id_t var_id, repcrec::var_t value);
    void set_available();
    void set_unavailable();

    std::shared_ptr<Variable> get_variable(repcrec::var_id_t var_id);

private:
    repcrec::site_id_t id_;
    bool is_available_;

    std::map<repcrec::var_id_t, std::shared_ptr<Variable>> variables_;

    static repcrec::site_id_t increase_id_;
};