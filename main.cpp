/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include <fstream>
#include <iostream>
#include <string>

#include "repcrec/transaction_manager/transaction_manager.h"

void execute_advanced_database(const std::string_view& file_name) {
    std::ifstream insr_file;
    insr_file.open(file_name, std::ios::binary);
    std::string insr;
    while (getline(insr_file, insr)) {
        // Skip dump and execute it after finishing all instructions.
        if (insr.starts_with("dump")) {
            continue;
        }
        ++repcrec::transaction_manager::TransactionManager::curr_timestamp;
        repcrec::transaction_manager::TransactionManager::get_instance().add_instruction(std::make_shared<repcrec::instruction::Instruction>(insr), repcrec::transaction_manager::TransactionManager::curr_timestamp);
        repcrec::transaction_manager::TransactionManager::get_instance().execute_instructions(repcrec::transaction_manager::TransactionManager::curr_timestamp);
    }
    insr_file.close();
    while (!repcrec::transaction_manager::TransactionManager::get_instance().is_all_instructions_finished()) {
        repcrec::transaction_manager::TransactionManager::get_instance().execute_instructions(repcrec::transaction_manager::TransactionManager::curr_timestamp);
        ++repcrec::transaction_manager::TransactionManager::curr_timestamp;
    }
    // extra dump
    repcrec::transaction_manager::TransactionManager::get_instance().add_instruction(std::make_shared<repcrec::instruction::Instruction>("dump()"), repcrec::transaction_manager::TransactionManager::curr_timestamp++);
    repcrec::transaction_manager::TransactionManager::get_instance().execute_instructions(repcrec::transaction_manager::TransactionManager::curr_timestamp);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        throw std::invalid_argument("Please specify your testing file.\n");
    }

    execute_advanced_database(argv[1]);
    return 0;
}
