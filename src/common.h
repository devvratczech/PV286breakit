#ifndef FILE_B_H
#define FILE_B_H

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <cstdint>
#include <cerrno>
#ifdef USE_OPENSSL
#include <openssl/crypto.h>
#endif

#define CENTRAL_BANK "Central"
#define BANK_US "US"
#define BANK_EU "EU"
#define BANK_AS "AS"
#define MAX_ACCOUNTS 100
#define MAX_BACKINGS 100
#define MAX_NAME_LENGTH 50

typedef struct {
    char account[MAX_NAME_LENGTH];    // Account receiving the backing
    char backedBy[MAX_NAME_LENGTH];   // Account providing the backing
    double limit;                        // Maximum amount allowed for backing
    unsigned int startTimestamp;      // When the backing starts
    unsigned int duration;            // How long the backing lasts
} AccountBacking;

typedef struct {
    char account[MAX_NAME_LENGTH];    // Account name
    double balance;                      // Account balance
} AccountBalance;

extern AccountBalance balances[MAX_ACCOUNTS];
extern AccountBacking backings[MAX_BACKINGS];
extern int backingCount;
extern int balanceCount;

void addTransaction(const char* from, const char* to, double amount, unsigned int timestamp);
void showBalance(const char* account);
void settleAccounts();
void deleteTransaction(unsigned int cutoffTimestamp, int deleteAll);
void processBatch(const char* filepath);
void addBalance(const char* account, double amount);
void initializeBanks();
void listAccounts();
double getBalance(const char* account);
void handleBackedTransaction(char* from, const char* to, double amount, unsigned int timestamp);
void showGeneralHelp();
void showDepositHelp();
void showTransferHelp();
void showBalanceHelp();
void showSettleHelp();
void showDeleteHelp();
void showBatchHelp();
void showWithdrawHelp();
void showBackHelp();
int isValidAccount(const char* account);
void handleBack(int argc, char* argv[]);
void reset();

void safeStrCopy(char* dest, const char* src, size_t destSize) {
    if (destSize > 0) {
        strncpy_s(dest, destSize, src, destSize - 1);
        dest[destSize - 1] = '\0';
    }
}

FILE* secureFileOpen(const char* filename, const char* mode) {
    FILE* file = nullptr;
    errno_t err;
    char errBuf[1024];

    if ((err = fopen_s(&file, filename, mode)) != 0) {
        strerror_s(errBuf, sizeof(errBuf), errno);
        std::cerr << "Error opening file '" << filename << "': " << errBuf << std::endl;
    }
    return file;
}

void secureErase(void* ptr, size_t size) {
#ifdef USE_OPENSSL
    OPENSSL_cleanse(ptr, size);
#else
    volatile char* p = reinterpret_cast<volatile char*>(ptr);
    while (size--) *p++ = 0;
#endif
}

double getBalance(const char* account) {
    for (int i = 0; i < balanceCount; i++) {
        if (strcmp(balances[i].account, account) == 0) {
            return balances[i].balance;
        }
    }
    return 0.0; // If the account is not found, assume a balance of 0
}

void showBalance(const char* account) {
    if (account == nullptr || strlen(account) == 0) {
        std::cout << "Invalid account name." << std::endl;
        return;
    }
    FILE* file = secureFileOpen("transactions.txt", "r");
    if (file == nullptr) {
        std::cout << "No transactions found." << std::endl;
        return;
    }

    double balance = 0;
    unsigned int timestamp;
    char from[MAX_NAME_LENGTH], to[MAX_NAME_LENGTH];
    double amount;

    while (fscanf_s(file, "%u: %49s sent %49s %lf\n", &timestamp, from, static_cast<unsigned int>(sizeof(from)), to, static_cast<unsigned int>(sizeof(to)), &amount) == 4) {
        from[sizeof(from) - 1] = '\0';
        to[sizeof(to) - 1] = '\0';
        if (strcmp(from, account) == 0) {
            balance -= amount;
        }
        if (strcmp(to, account) == 0) {
            balance += amount;
        }
    }

    fclose(file);
    std::cout << "Balance for " << account << ": " << balance << std::endl;
}

void listAccounts() {
    if (balanceCount == 0) {
        std::cout << "No accounts found." << std::endl;
    }
    else {
        std::cout << "List of accounts:" << std::endl;
        for (int i = 0; i < balanceCount; i++) {
            std::cout << balances[i].account << std::endl;
        }
    }
}

int isValidAccount(const char* account) {
    if (account == nullptr || strlen(account) == 0 || strlen(account) >= MAX_NAME_LENGTH) {
        return 0; // Account name length invalid
    }
    for (int i = 0; account[i] != '\0'; i++) {
        if (!isalnum(account[i])) {
            return 0; // Account name contains invalid characters
        }
    }
    return 1; // Account name is valid
}

void initializeBanks() {
    addBalance(BANK_US, 0);
    addBalance(BANK_EU, 0);
    addBalance(BANK_AS, 0);
}

void loadTransactions() {
    FILE* file = secureFileOpen("transactions.txt", "r");
    if (file == nullptr) {
        std::cout << "No transactions file found. Starting fresh." << std::endl;
        return;
    }

    unsigned int timestamp;
    char from[MAX_NAME_LENGTH], to[MAX_NAME_LENGTH];
    double amount;

    while (fscanf_s(file, "%u: %49s sent %49s %lf\n", &timestamp, from, static_cast<unsigned int>(sizeof(from)), to, static_cast<unsigned int>(sizeof(to)), &amount) == 4) {
        from[sizeof(from) - 1] = '\0';
        to[sizeof(to) - 1] = '\0';
        if (strcmp(from, "DEPOSIT") == 0) {
            addBalance(to, amount);
        }
        else {
            addBalance(from, -amount);
            addBalance(to, amount);
        }
    }
    fclose(file);

}

#endif // FILE_B_H