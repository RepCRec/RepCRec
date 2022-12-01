//
// Created by Junhua Liang on 11/25/22.
//

#include <fstream>
#include <iostream>
#include <string>

#include "repcrec/transaction_manager/transaction_manager.h"
using namespace repcrec::transaction_manager;
using namespace repcrec::instruction;
static std::string inputFilePath = "../test/input/";
static std::string resultFilePath = "../test/output/";
static std::string outputFilePath = "../test/";

void execute_advanced_database(const std::string_view &file_name, const std::string& mode) {
    std::ifstream insr_file;
    insr_file.open(inputFilePath+file_name.data(), std::ios::binary);
    std::shared_ptr<std::ofstream> outFile;
    if(mode == "test"){
        outFile = std::make_shared<std::ofstream>(outputFilePath + file_name.data());
        repcrec::site_manager::SiteManager::set_file(outFile);
    }

    std::string insr;
    while (getline(insr_file, insr)) {
        // printf("INFO: Start executing %s\n", insr.c_str());
        if (insr.starts_with("dump")) {
            continue;
        }
        ++TransactionManager::curr_timestamp;
        TransactionManager::get_instance().add_instruction(std::make_shared<Instruction>(insr), TransactionManager::curr_timestamp);
        TransactionManager::get_instance().execute_instructions(TransactionManager::curr_timestamp);
    }
    insr_file.close();
    while (!TransactionManager::get_instance().is_all_instructions_finished()) {
        TransactionManager::get_instance().execute_instructions(TransactionManager::curr_timestamp);
        ++TransactionManager::curr_timestamp;
    }
    // extra dump
    TransactionManager::get_instance().add_instruction(std::make_shared<Instruction>("dump()"), TransactionManager::curr_timestamp++);
    TransactionManager::get_instance().execute_instructions(TransactionManager::curr_timestamp);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        throw std::invalid_argument("Please specify your testing file.\n");
    }
    execute_advanced_database(argv[2],argv[1]);
    return 0;
}
