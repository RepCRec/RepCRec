/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#ifndef REPCREC_VARIABLE_H
#define REPCREC_VARIABLE_H

#include <map>
#include <memory>
#include <unordered_map>

#include "../global.h"
#include "../transaction/transaction.h"

namespace repcrec {
    namespace variable {
        class Variable {
        public:
            explicit Variable(repcrec::var_id_t var_id);
            ~Variable() = default;

            std::shared_ptr<repcrec::transaction::Transaction> get_exclusive_lock_owner();
            std::unordered_map<repcrec::var_id_t, std::shared_ptr<repcrec::transaction::Transaction>> get_shared_lock_owners();
            bool add_shared_transaction(const std::shared_ptr<repcrec::transaction::Transaction> &transaction);
            bool set_exclusive_transaction(const std::shared_ptr<repcrec::transaction::Transaction> &transaction);
            void remove_exclusive_owner();
            void set_value(repcrec::var_t value, repcrec::timestamp_t timestamp);
            bool remove_shared_owner(repcrec::tran_id_t tran_id);

            [[nodiscard]] bool has_shared_lock(repcrec::tran_id_t tran_id) const;
            [[nodiscard]] bool has_exclusive_lock(repcrec::tran_id_t tran_id) const;
            [[nodiscard]] bool has_shared_lock_exclude_self(repcrec::tran_id_t tran_id) const;
            [[nodiscard]] bool has_exclusive_lock_exclude_self(repcrec::tran_id_t tran_id) const;
            [[nodiscard]] repcrec::var_id_t get_id() const;
            [[nodiscard]] repcrec::var_t get_value(repcrec::timestamp_t timestamp = -1) const;

        private:
            repcrec::var_id_t id_;
            repcrec::var_t value_;

            std::map<repcrec::timestamp_t, repcrec::var_t> versions_;
            std::shared_ptr<repcrec::transaction::Transaction> exclusive_lock_owner_;
            std::unordered_map<repcrec::tran_id_t, std::shared_ptr<repcrec::transaction::Transaction>> shared_lock_owners_;
        };
    }// namespace variable
}// namespace repcrec

#endif//REPCREC_VARIABLE_H
