#include <iostream>
#include "transaction_manager.h"

int main() {
    std::unique_ptr<TransactionManager> transaction_manager = std::make_unique<TransactionManager>();
    transaction_manager->dump();
    return 0;
}
