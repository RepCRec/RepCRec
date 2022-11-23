//
// Created by 梁俊华 on 11/23/22.
//

#include "site.h"

int Site::increase_id_ = 1;

Site::Site() : id_(increase_id_), is_available_(false) {
    ++increase_id_;
}

bool Site::is_available() const {
    return is_available_;
}

int Site::getSiteId() const {
    return id_;
}

void Site::dump() const {
    printf("site %d - ", id_);
    for (auto iter = variables_.begin(); iter != variables_.end(); ++iter) {
        if (iter == prev(end(variables_))) {
            printf("x%d: %d", iter->second->get_id(), iter->second->get_value());
        } else {
            printf("x%d: %d ", iter->second->get_id(), iter->second->get_value());
        }
    }
    printf("\n");
}
