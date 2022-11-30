//
// Created by 梁俊华 on 11/21/22.
//

#ifndef REPCREC_TRANSACTION_H
#define REPCREC_TRANSACTION_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "../global.h"
#include "../instruction/instruction.h"

namespace repcrec::transaction {
    class Transaction {
    public:
        explicit Transaction(repcrec::tran_id_t transaction_id, repcrec::timestamp_t timestamp, bool is_read_only = false);
        ~Transaction() = default;

        [[nodiscard]] repcrec::tran_id_t get_transaction_id() const;
        [[nodiscard]] bool is_read_only() const;
        [[nodiscard]] repcrec::timestamp_t get_timestamp() const;
        [[nodiscard]] repcrec::var_id_t get_waiting_var_id() const;

        bool commit(repcrec::timestamp_t commit_time);
        void update_values(repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t var);
        void add_read_history(repcrec::site_id_t site_id, repcrec::var_id_t var_id, repcrec::var_t var);
        void set_waiting_var_id(repcrec::var_id_t var_id);

    private:
        repcrec::timestamp_t timestamp_;
        repcrec::tran_id_t transaction_id_;
        repcrec::var_id_t waiting_var_id_;
        bool is_read_only_;
        std::unordered_map<repcrec::site_id_t, std::unordered_map<repcrec::var_id_t, repcrec::var_t>> write_records_;
        std::unordered_map<repcrec::site_id_t, std::unordered_map<repcrec::var_id_t, repcrec::var_t>> read_accessed_sites_;
        std::unordered_map<repcrec::site_id_t, std::unordered_map<repcrec::var_id_t, repcrec::var_t>> write_accessed_sites_;
    };
} // namespace repcrec

#endif//REPCREC_TRANSACTION_H
