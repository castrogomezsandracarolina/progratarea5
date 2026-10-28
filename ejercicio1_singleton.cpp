#include <cmath>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class Account {
private:
    std::string id_;
    double balance_;
    mutable std::mutex mutex_;
    friend class TransactionManager;
public:
    Account(std::string id, double initialBalance)
        : id_(std::move(id)), balance_(initialBalance) {
        if (!std::isfinite(initialBalance) || initialBalance < 0) {
            throw std::invalid_argument("Saldo inicial invalido");
        }
    }
    double getBalance() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return balance_;
    }
};

class Transaction {
public:
    enum class Type { Deposit, Withdrawal };
private:
    Type type_;
    std::string accountId_;
    double amount_;
    bool success_;
public:
    Transaction(Type type, std::string accountId, double amount, bool success)
        : type_(type), accountId_(std::move(accountId)),
          amount_(amount), success_(success) {}
    Type getType() const { return type_; }
    const std::string& getAccountId() const { return accountId_; }
    double getAmount() const { return amount_; }
    bool succeeded() const { return success_; }
};

class TransactionManager {
private:
    std::mutex mutex_;
    std::vector<Transaction> history_;
    TransactionManager() = default;
    static void validate(Account* account, double amount) {
        if (account == nullptr) {
            throw std::invalid_argument("La cuenta no puede ser nula");
        }
        if (!std::isfinite(amount) || amount <= 0) {
            throw std::invalid_argument("El monto debe ser positivo y finito");
        }
    }
public:
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;
    static TransactionManager& getInstance() {
        static TransactionManager instance;
        return instance;
    }
    bool deposit(Account* account, double amount) {
        validate(account, amount);
        std::scoped_lock lock(mutex_, account->mutex_);
        account->balance_ += amount;
        history_.emplace_back(Transaction::Type::Deposit, account->id_, amount, true);
        return true;
    }
    bool withdraw(Account* account, double amount) {
        validate(account, amount);
        std::scoped_lock lock(mutex_, account->mutex_);
        bool success = account->balance_ >= amount;
        if (success) {
            account->balance_ -= amount;
        }
        history_.emplace_back(Transaction::Type::Withdrawal,
                              account->id_, amount, success);
        return success;
    }
    std::vector<Transaction> getHistory() {
        std::lock_guard<std::mutex> lock(mutex_);
        return history_;
    }
};

int main() {
    auto& manager = TransactionManager::getInstance();
    Account account("A1", 1000);
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            manager.deposit(&account, 100);
            manager.withdraw(&account, 50);
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "Balance final: " << account.getBalance() << '\n';
    std::cout << "Direccion manager: " << &TransactionManager::getInstance() << '\n';
    std::cout << "Misma instancia: " << std::boolalpha
              << (&manager == &TransactionManager::getInstance()) << '\n';
    std::cout << "Transacciones registradas: " << manager.getHistory().size() << '\n';
    return 0;
}
