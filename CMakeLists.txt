cmake_minimum_required(VERSION 3.23)
project(RepCRec)

set(CMAKE_CXX_STANDARD 20)

add_executable(RepCRec src/main.cpp src/transaction_manager.cc src/transaction_manager.h src/site_manager.cc src/site_manager.h src/transaction.cc src/transaction.h src/instruction.cc src/instruction.h src/site.h src/site.cc src/variable.h src/variable.cc src/lock_manager.cc src/lock_manager.h src/request.h src/request.cc src/global.h src/utils.h)
