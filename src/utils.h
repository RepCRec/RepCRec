//
// Created by 梁俊华 on 11/26/22.
//

#ifndef REPCREC_UTILS_H
#define REPCREC_UTILS_H

template<typename Base, typename T>
inline bool instanceof(const T * ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

#endif//REPCREC_UTILS_H
