#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "help.cpp"
#include "common.h"

AccountBalance balances[MAX_ACCOUNTS];    // Array of AccountBalance structures
AccountBacking backings[100];             // Array of AccountBacking structures
int backingCount = 0;                     // Initial count of backings
int balanceCount = 0;                     // Initial count of balances

double min(double a, double b) {
    return (a < b) ? a : b;
}

int findArgIndex(int argc, char* argv[], const char* argToFind) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], argToFind) == 0) {
            return i;
        }
    }
    return -1;
}

// Secure file opening utility
FILE* secureOpenFile(const char* filename, const char* mode) {
    FILE* file;
    errno_t err = fopen_s(&file, filename, mode);
    if (err != 0 || file == NULL) {
        printf("Error opening file %s.\n", filename);
        return NULL;
    }
    return file;
}

void handleDeposit(int argc, char* argv[]) {
    int accountIndex = findArgIndex(argc, argv, "--account") + 1;
    int amountIndex = findArgIndex(argc, argv, "--amount") + 1;
    int timestampIndex = findArgIndex(argc, argv, "--timestamp") + 1;

    if (accountIndex <= 0 || amountIndex <= 0 || timestampIndex <= 0) {
        printf("Missing arguments for deposit command.\n");
        return;
    }

    char* end;
    double amount = strtod(argv[amountIndex], &end);
    if (*end != '\0' || amount <= 0) {
        printf("Invalid deposit amount.\n");
        return;
    }

    addBalance(argv[accountIndex], amount);
    FILE* file = secureOpenFile("transactions.txt", "a");
    if (!file) {
        return; // Error message handled by secureOpenFile
    }

    fprintf(file, "%s: DEPOSIT sent %s %.2lf\n", argv[timestampIndex], argv[accountIndex], amount); // Adjusted to use %lf for double
    fclose(file);
    printf("Deposited %.2lf to account %s at timestamp %s\n", amount, argv[accountIndex], argv[timestampIndex]); // Adjusted to use %.2lf for double
}

void handleBalance(int argc, char* argv[]) {
    int accountIndex = findArgIndex(argc, argv, "--account") + 1;

    if (accountIndex <= 0 || accountIndex >= argc) {
        printf("Invalid or missing account argument.\n");
        return;
    }

    showBalance(argv[accountIndex]);
}

void handleTransfer(int argc, char* argv[]) {
    int fromIndex = findArgIndex(argc, argv, "--from") + 1;
    int toIndex = findArgIndex(argc, argv, "--to") + 1;
    int amountIndex = findArgIndex(argc, argv, "--amount") + 1;
    int timestampIndex = findArgIndex(argc, argv, "--timestamp") + 1;

    if (fromIndex <= 0 || toIndex <= 0 || amountIndex <= 0 || timestampIndex <= 0) {
        printf("Usage: /tx transfer --from <source> --to <destination> --amount <amount> --timestamp <timestamp>\n");
        return;
    }

    if (strcmp(argv[fromIndex], argv[toIndex]) == 0) {
        printf("Error: Cannot transfer to the same account.\n");
        return;
    }

    char* end;
    double amount = strtod(argv[amountIndex], &end);
    if (*end != '\0' || amount <= 0) {
        printf("Invalid transfer amount.\n");
        return;
    }

    double fromBalance = getBalance(argv[fromIndex]);
    if (fromBalance < amount) {
        printf("Error: Insufficient funds in account %s.\n", argv[fromIndex]);
        return;
    }

    addTransaction(argv[fromIndex], argv[toIndex], amount, (unsigned int)atoi(argv[timestampIndex]));
}


void handleWithdraw(int argc, char* argv[]) {
    int accountIndex = findArgIndex(argc, argv, "--account") + 1;
    int amountIndex = findArgIndex(argc, argv, "--amount") + 1;

    if (accountIndex <= 0 || amountIndex <= 0 || accountIndex >= argc || amountIndex >= argc) {
        printf("Missing arguments for withdraw command.\n");
        return;
    }

    char* end;
    double amount = strtod(argv[amountIndex], &end);
    if (*end != '\0' || amount <= 0) {
        printf("Invalid withdraw amount.\n");
        return;
    }

    double balance = getBalance(argv[accountIndex]);
    if (balance < amount) {
        printf("Error: Insufficient funds for withdrawal from account %s.\n", argv[accountIndex]);
        return;
    }

    addBalance(argv[accountIndex], -amount);  // Subtract the withdrawal amount from the account balance

    FILE* file = secureOpenFile("transactions.txt", "a");
    if (!file) {
        return;  // Error message handled by secureOpenFile
    }

    fprintf(file, "0: %s sent WITHDRAWAL %.2lf\n", argv[accountIndex], amount);
    fclose(file);
    printf("Withdrew %.2lf from account %s\n", amount, argv[accountIndex]);
}

int main(int argc, char* argv[]) {
    loadTransactions();
    initializeBanks();

    if (argc < 2) {
        printf("Usage: tx <command> [arguments]\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            if (argc == 2) {
                showGeneralHelp();
            }
            else if (i > 1) {
                if (strcmp(argv[i - 1], "deposit") == 0) {
                    showDepositHelp();
                }
                else if (strcmp(argv[i - 1], "transfer") == 0) {
                    showTransferHelp();
                }
                else if (strcmp(argv[i - 1], "balance") == 0) {
                    showBalanceHelp();
                }
                else if (strcmp(argv[i - 1], "settle") == 0) {
                    showSettleHelp();
                }
                else if (strcmp(argv[i - 1], "delete") == 0) {
                    showDeleteHelp();
                }
                else if (strcmp(argv[i - 1], "batch") == 0) {
                    showBatchHelp();
                }
                else if (strcmp(argv[i - 1], "withdraw") == 0) {
                    showWithdrawHelp();
                }
                else if (strcmp(argv[i - 1], "back") == 0) {
                    showBackHelp();
                }
            }
            return 0;  // Exit after showing help
        }
    }

    if (strcmp(argv[1], "--list-accounts") == 0) {
        listAccounts();
    }
    else if (strcmp(argv[1], "deposit") == 0) {
        handleDeposit(argc, argv);
    }
    else if (strcmp(argv[1], "transfer") == 0) {
        handleTransfer(argc, argv);
    }
    else if (strcmp(argv[1], "balance") == 0) {
        handleBalance(argc, argv);
    }
    else if (strcmp(argv[1], "settle") == 0) {
        settleAccounts();
    }
    else if (strcmp(argv[1], "delete") == 0) {
        int timestampIndex = findArgIndex(argc, argv, "--timestamp") + 1;
        if (timestampIndex > 0) {
            unsigned int timestamp = (unsigned int)atoi(argv[timestampIndex]);
            deleteTransaction(timestamp, 0);
        }
        else {
            deleteTransaction(0, 1);
        }
        return 0;
    }
    else if (strcmp(argv[1], "batch") == 0) {
        processBatch(argv[2]);
    }
    else if (strcmp(argv[1], "withdraw") == 0) {
        handleWithdraw(argc, argv);
    }
    else if (strcmp(argv[1], "back") == 0) {
        handleBack(argc, argv);
    }
    else if (strcmp(argv[1], "reset") == 0) {
        reset();
    }
    else {
        printf("Unknown command. Supported commands are 'deposit', 'transfer', 'balance', 'settle', 'delete', 'batch'.\n");
    }

    return 0;
}

void addBalance(const char* account, double amount) {
    for (int i = 0; i < balanceCount; i++) {
        if (strcmp(balances[i].account, account) == 0) {
            balances[i].balance += amount;
            return;
        }
    }
    if (balanceCount < MAX_ACCOUNTS) {
        if (strcpy_s(balances[balanceCount].account, sizeof(balances[balanceCount].account), account) != 0) {
            printf("Error copying account name.\n");
            return;
        }
        balances[balanceCount].balance = amount;
        balanceCount++;
    }
    else {
        printf("Maximum number of accounts reached.\n");
    }
}

void addTransaction(const char* from, const char* to, double amount, unsigned int timestamp) {
    // Basic validation of parameters
    if (amount <= 0 || timestamp <= 0 || strlen(from) == 0 || strlen(to) == 0) {
        printf("Invalid transaction data.\n");
        return;
    }

    // Check for sufficient funds in the source account for transfers (excluding deposits)
    if (strcmp(from, "DEPOSIT") != 0 && getBalance(from) < amount) {
        printf("Transaction failed: Insufficient funds in %s's account.\n", from);
        return;
    }

    FILE* file;
    errno_t err = fopen_s(&file, "transactions.txt", "a");
    if (err != 0 || file == NULL) {
        printf("Error opening transactions file.\n");
        return;
    }

    // Record the transaction
    fprintf(file, "%u: %s sent %s %.2lf\n", timestamp, from, to, amount);
    fclose(file);
    printf("Transaction from %s to %s for amount %.2lf at timestamp %u added.\n", from, to, amount, timestamp);

    // Update balances
    if (strcmp(from, "DEPOSIT") != 0) {
        addBalance(from, -amount);
    }
    addBalance(to, amount);
}


void settleAccounts() {

    loadTransactions();

    // Identify creditors and debtors based on their net balances
    int creditors[MAX_ACCOUNTS], debtors[MAX_ACCOUNTS];
    int nCreditors = 0, nDebtors = 0;

    for (int i = 0; i < balanceCount; i++) {
        if (balances[i].balance > 0) {
            creditors[nCreditors++] = i;
        }
        else if (balances[i].balance < 0) {
            debtors[nDebtors++] = i;
        }
    }

    // Settle accounts
    printf("Settlement transactions:\n");
    for (int i = 0; i < nDebtors; i++) {
        int debtorIdx = debtors[i];
        for (int j = 0; j < nCreditors && balances[debtorIdx].balance < 0; j++) {
            int creditorIdx = creditors[j];
            if (balances[creditorIdx].balance > 0) {
                double settleAmount = min(-balances[debtorIdx].balance, balances[creditorIdx].balance);
                balances[debtorIdx].balance += settleAmount;
                balances[creditorIdx].balance -= settleAmount;
                printf("%s pays %s: %2lf\n", balances[debtorIdx].account, balances[creditorIdx].account, settleAmount);
            }
        }
    }
}

void deleteTransaction(unsigned int cutoffTimestamp, int deleteAll) {
    const char* transactionsFile = "transactions.txt";

    if (deleteAll) {
        // Wipe the application's state
        if (remove(transactionsFile) == 0) {
            printf("All transactions have been deleted.\n");
        }
        else {
            printf("Error deleting transactions.\n");
        }
        return;
    }

    // Delete transactions from a specific timestamp
    FILE* file = secureOpenFile(transactionsFile, "r");
    if (!file) {
        return; // Error message already handled
    }

    FILE* tempFile = secureOpenFile("temp_transactions.txt", "w");
    if (!tempFile) {
        fclose(file);
        return; // Error message already handled
    }

    unsigned int readTimestamp;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf_s(line, "%u:", &readTimestamp) != 1) {
            continue; // skip malformed lines
        }
        if (readTimestamp < cutoffTimestamp) {
            // Keep transactions before the given timestamp
            fputs(line, tempFile);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the updated one
    remove(transactionsFile);
    rename("temp_transactions.txt", transactionsFile);

    printf("Transactions from timestamp %u have been deleted.\n", cutoffTimestamp);
}

void processBatch(const char* filepath) {
    FILE* file;
    errno_t err = fopen_s(&file, filepath, "r");
    if (err != 0 || file == NULL) {
        printf("Error opening batch file.\n");
        return;
    }

    char line[256], prevFrom[50] = "", prevTo[50] = "", prevAmount[50] = "", prevTimestamp[50] = "";
    while (fgets(line, sizeof(line), file)) {
        char from[50] = "", to[50] = "", amount[50] = "", timestamp[50] = "";

        if (sscanf_s(line, "%49[^:]:%49[^:]:%49[^:]:%49s\n", from, sizeof(from), to, sizeof(to), amount, sizeof(amount), timestamp, sizeof(timestamp)) != 4) {
            continue; // skip malformed lines
        }

        // Use previous values if current ones are missing
        if (strcmp(from, "") == 0) strcpy_s(from, sizeof(from), prevFrom);
        if (strcmp(to, "") == 0) strcpy_s(to, sizeof(to), prevTo);
        if (strcmp(amount, "") == 0) strcpy_s(amount, sizeof(amount), prevAmount);
        if (strcmp(timestamp, "") == 0) strcpy_s(timestamp, sizeof(timestamp), prevTimestamp);

        // Save current values for next iteration
        strcpy_s(prevFrom, sizeof(prevFrom), from);
        strcpy_s(prevTo, sizeof(prevTo), to);
        strcpy_s(prevAmount, sizeof(prevAmount), amount);
        strcpy_s(prevTimestamp, sizeof(prevTimestamp), timestamp);

        // Process the transaction, handling backed transactions if necessary
        if (strchr(from, '(') != NULL) {
            // This is a backed transaction
            handleBackedTransaction(from, to, atoi(amount), (unsigned int)atoi(timestamp));
        }
        else {
            // This is a regular transaction
            addTransaction(from, to, atoi(amount), (unsigned int)atoi(timestamp));
        }
    }

    fclose(file);
}





void handleBack(int argc, char* argv[]) {
    if (argc < 9) {
        printf("Usage: tx back --account <account_name> --by <backing_account> --limit <limit> --timestamp <timestamp> --duration <duration>\n");
        return;
    }

    int accountIndex = findArgIndex(argc, argv, "--account") + 1;
    int byIndex = findArgIndex(argc, argv, "--by") + 1;
    int limitIndex = findArgIndex(argc, argv, "--limit") + 1;
    int timestampIndex = findArgIndex(argc, argv, "--timestamp") + 1;
    int durationIndex = findArgIndex(argc, argv, "--duration") + 1;

    if (accountIndex <= 0 || byIndex <= 0 || limitIndex <= 0 || timestampIndex <= 0 || durationIndex <= 0) {
        printf("Missing arguments for back command.\n");
        return;
    }

    const char* account = argv[accountIndex];
    const char* by = argv[byIndex];
    double limit = atoi(argv[limitIndex]);
    unsigned int timestamp = (unsigned int)atoi(argv[timestampIndex]);
    unsigned int duration = (unsigned int)atoi(argv[durationIndex]);

    if (!isValidAccount(account) || !isValidAccount(by)) {
        printf("Error: Invalid account name.\n");
        return;
    }

    if (limit <= 0) {
        printf("Error: Limit must be a positive integer.\n");
        return;
    }

    strcpy_s(backings[backingCount].account, sizeof(backings[backingCount].account), account);
    strcpy_s(backings[backingCount].backedBy, sizeof(backings[backingCount].backedBy), by);
    backings[backingCount].limit = limit;
    backings[backingCount].startTimestamp = timestamp;
    backings[backingCount].duration = duration;
    backingCount++;

    printf("Backing relationship added: %s backed by %s with a limit of %.01f from timestamp %u for duration %u\n",
        account, by, limit, timestamp, duration);
}

void handleBackedTransaction(char* from, const char* to, double amount, unsigned int timestamp) {
    char primaryAccount[50], backingAccounts[100];
    sscanf_s(from, "%49[^()](%99[^)])", primaryAccount, sizeof(primaryAccount), backingAccounts, sizeof(backingAccounts));

    double primaryBalance = getBalance(primaryAccount);
    if (primaryBalance >= amount) {
        addTransaction(primaryAccount, to, amount, timestamp);
        return;
    }

    char* token = strtok_s(backingAccounts, ",", NULL);
    double totalCoveredAmount = 0;

    while (token != NULL && totalCoveredAmount < amount) {
        for (int i = 0; i < backingCount; i++) {
            if (strcmp(backings[i].account, primaryAccount) == 0 && strcmp(backings[i].backedBy, token) == 0 &&
                timestamp >= backings[i].startTimestamp && timestamp <= backings[i].startTimestamp + backings[i].duration) {

                double backerBalance = getBalance(token) - backings[i].limit;
                double contribution = min(amount - totalCoveredAmount, backerBalance);
                if (contribution > 0) {
                    printf("%s backs %s with %2lf units\n", token, primaryAccount, contribution);
                    addBalance(token, -contribution);
                    totalCoveredAmount += contribution;
                }
            }
        }
        token = strtok_s(NULL, ",", NULL);
    }

    if (totalCoveredAmount + primaryBalance >= amount) {
        double shortfall = amount - primaryBalance;
        addBalance(primaryAccount, shortfall); // Temporarily adjust to cover transaction
        addTransaction(primaryAccount, to, amount, timestamp);
        addBalance(primaryAccount, -amount); // Adjust after recording the transaction
    }
    else {
        printf("Transaction failed: Insufficient backing for %s.\n", primaryAccount);
    }
}

void reset() {
    // Clear all transactions
    FILE* transactionsFile = secureOpenFile("transactions.txt", "w");
    if (transactionsFile == NULL) {
        printf("Error opening transactions file for writing.\n");
        return;
    }
    fclose(transactionsFile);

    // Reset account balances
    initializeBanks();
}