#include <iostream>
#include <exception>
#include <fstream>
#include <string>
#include "transaction_manager.h"

void process_test_file(const std::string_view& file_name) {
    std::ifstream insr_file;
    insr_file.open(file_name, std::ios::binary);
    std::string insr;
    repcrec::timestamp_t timestamp = 1;
    while (getline(insr_file, insr)) {
        TransactionManager::get_instance().add_instruction(std::make_shared<Instruction>(insr), timestamp);
        TransactionManager::get_instance().execute_instructions(timestamp);
        ++timestamp;
    }
    insr_file.close();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        throw std::invalid_argument("Please specify your testing file.\n");
    }

    process_test_file(argv[1]);
    return 0;
}
