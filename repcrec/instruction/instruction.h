/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#ifndef REPCREC_INSTRUCTION_H
#define REPCREC_INSTRUCTION_H

#include <string>
#include <vector>

namespace repcrec {
    namespace instruction {
        enum struct InstructType {
            UNKNOWN = 0,
            BEGIN = 1,
            BEGINO = 2,
            READ = 3,
            WRITE = 4,
            RECOVER = 5,
            FAIL = 6,
            DUMP = 7,
            END = 8
        };

        class Instruction {
        public:
            Instruction();
            explicit Instruction(const std::string &insr);
            ~Instruction() = default;

            [[nodiscard]] InstructType get_type() const;
            [[nodiscard]] int get_var_id() const;
            [[nodiscard]] int get_var_value() const;
            [[nodiscard]] int get_site_id() const;
            [[nodiscard]] int get_tran_id() const;

        private:
            int id_;

            InstructType type_;
            int var_id_;
            int var_value_;
            int site_id_;
            int tran_id_;

            static int initial_id_;
        };
    }// namespace instruction
} // namespace repcrec::instruction.

#endif //REPCREC_INSTRUCTION_H
