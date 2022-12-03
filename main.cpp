/**
* @brief       RepCRec database system project.
* @copyright   New York University.
* @authors     Junhua Liang, Guanqun Yang.
* @date        2022-12-02.
*/

#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <memory>
#include "repcrec/transaction_manager/transaction_manager.h"
using namespace repcrec::transaction_manager;
using namespace repcrec::instruction;
//using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
using std::cout;
using std::endl;
using std::string;

static std::string inputFilePath = "./test/input/";
static std::string resultFilePath = "./test/output/";
static std::string outputFilePath = "./test/";

void execute_test_case_check(const string &mode) {
    std::vector<std::string> fileNames;
    int right = 0;
    if (mode == "all") {
        struct dirent *entry = nullptr;
        DIR *dp = nullptr;
        dp = opendir(inputFilePath.c_str());
        if (dp != nullptr) {
            while ((entry = readdir(dp))){
                if(entry->d_type != DT_DIR){
                    fileNames.push_back(entry->d_name);
                }
            }
        }
        closedir(dp);
    } else {
        fileNames.push_back(mode);
    }

    for (auto &fileName: fileNames) {
        cout << "Test case: " << fileName << endl;

        std::ifstream resultFile(resultFilePath + fileName);
        std::ifstream outputFile(outputFilePath + fileName);
        string resultLine;
        string outputLine;
        while (std::getline(resultFile, resultLine) && std::getline(outputFile, outputLine)) {
            if (resultLine != outputLine) {
                cout << "\033[1;31mWrong Line\033[0m" << endl;
                cout << "Result: " << resultLine << endl;
                cout << "Your output: " << outputLine << endl;
                break;
            }
        }
        if (resultLine == outputLine) {
            cout << "\033[1;32mPass\033[0m" << endl;
            right++;
        }
        resultFile.close();
        outputFile.close();
    }

    cout << right << " / " << fileNames.size() << " pass" << endl;
}

void execute_advanced_database(const std::string &file_name, const std::string &mode) {
    std::ifstream insr_file;
    insr_file.open(inputFilePath + file_name);
    std::shared_ptr<std::ofstream> outFile;
    if (mode == "test") {
        outFile = std::make_shared<std::ofstream>(outputFilePath + file_name);
        repcrec::site_manager::SiteManager::set_file(outFile);
    }

    std::string insr;
    while (getline(insr_file, insr)) {
        // Skip dump and execute it after finishing all instructions.
        if (insr.find("dump") == 0) {
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

    if (mode == "test") {
        outFile->close();
    }
}


int main(int argc, char **argv) {
    if (argc < 3) {
        throw std::invalid_argument("Please specify your testing file.\n");
    }
    if (string(argv[2]) == "all") {
        struct dirent *entry = nullptr;
        DIR *dp = nullptr;
        dp = opendir(inputFilePath.c_str());
        if (dp != nullptr) {
            while ((entry = readdir(dp))){
                if(entry->d_type != DT_DIR){
                    cout << "---------------------------------------------------" << endl;
                    cout << "start " << entry->d_name << endl;
                    auto pid = fork();
                    if (pid == 0) {
                        execute_advanced_database(string(entry->d_name), "test");
                        return 0;
                    } else {
                        while (true) {
                            int status;
                            pid_t done = wait(&status);
                            if (done == -1) {
                                if (errno == ECHILD) break;
                            } else {
                                if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                                    std::cerr << entry->d_name << " failed" << endl;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        closedir(dp);
        execute_test_case_check("all");

        return 0;
    } else {
        execute_advanced_database(argv[2], argv[1]);
        execute_test_case_check(argv[2]);
    }
    return 0;
}
