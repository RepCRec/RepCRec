cmake_minimum_required(VERSION 3.19.2)
project(RepCRec)

set(CMAKE_CXX_STANDARD 14)

add_executable(RepCRecExec main.cpp repcrec/transaction_manager/transaction_manager.cc repcrec/transaction_manager/transaction_manager.h repcrec/site_manager/site_manager.cc repcrec/site_manager/site_manager.h repcrec/transaction/transaction.cc repcrec/transaction/transaction.h repcrec/instruction/instruction.cc repcrec/instruction/instruction.h repcrec/site/site.h repcrec/site/site.cc repcrec/variable/variable.h repcrec/variable/variable.cc repcrec/lock_manager/lock_manager.cc repcrec/lock_manager/lock_manager.h repcrec/request/request.h repcrec/request/request.cc repcrec/global.h)
