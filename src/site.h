//
// Created by 梁俊华 on 11/22/22.
//
#include "variable.h"
#include <map>

class Site {
public:
    explicit Site();

    [[nodiscard]] bool is_available() const;
    [[nodiscard]] int getSiteId() const;
    void dump() const;

private:
    int id_;
    bool is_available_;

    std::map<int, std::shared_ptr<Variable>> variables_;

    static int increase_id_;
};