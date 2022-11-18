cmake_minimum_required(VERSION 3.23)
project(RepCRec)

set(CMAKE_CXX_STANDARD 20)

add_executable(RepCRec src/main.cpp src/transaction_manager.cc src/transaction_manager.h src/data_manager.cc src/data_manager.h src/transaction.cc src/transaction.h src/site.cc src/site.h src/variable.cc src/variable.h src/instruction.cc src/instruction.h)
