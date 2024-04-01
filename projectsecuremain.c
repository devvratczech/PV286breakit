#define MAX_ACCOUNTS 100
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Bank identifiers
#define CENTRAL_BANK "Central"
#define BANK_US "US"
#define BANK_EU "EU"
#define BANK_AS "AS"


// Function prototypes
void addTransaction(const char* from, const char* to, int amount, unsigned int timestamp);
void showBalance(const char* account);
void settleAccounts();
void deleteTransaction(unsigned int timestamp);
void processBatch(const char* filepath);
void addBalance(const char* account, int amount);
void initializeBanks();
void listAccounts();
int getBalance(const char* account);
void handleBackedTransaction(char* from, const char* to, int amount, unsigned int timestamp);
void showGeneralHelp();
void showDepositHelp();
void showTransferHelp();
void showBalanceHelp();
void showSettleHelp();
void showDeleteHelp();
void showBatchHelp();
void showWithdrawHelp();
void showBackHelp();

typedef struct {
    char account[50];    // Account receiving the backing
    char backedBy[50];   // Account providing the backing
    int limit;           // Maximum amount allowed for backing
    unsigned int startTimestamp;  // When the backing starts
    unsigned int duration;        // How long the backing lasts
} AccountBacking;

typedef struct {
    char account[50];    // Account receiving the backing
    int balance;   // Account providing the backing
    // How long the backing lasts
} AccountBalance;

AccountBalance balances[MAX_ACCOUNTS];
AccountBacking backings[100]; // Assuming a max of 100 backing relationships for simplicity
int backingCount = 0;
int balanceCount = 0;


int findArgIndex(int argc, char* argv[], const char* argToFind) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], argToFind) == 0) {
            return i;
        }
    }
    return -1;
}

void handleDeposit(int argc, char* argv[]) {
    int accountIndex = findArgIndex(argc, argv, "--account") + 1;
    int amountIndex = findArgIndex(argc, argv, "--amount") + 1;
    int timestampIndex = findArgIndex(argc, argv, "--timestamp") + 1;

    if (accountIndex <= 0 || amountIndex <= 0 || timestampIndex <= 0) {
        printf("Missing arguments for deposit command.\n");
        return;
    }

    int amount = atoi(argv[amountIndex]);
    if (amount <= 0) {
        printf("Invalid deposit amount.\n");
        return;
    }

    // Ensure the account exists or create a new one
   // ensureAccountExists(argv[accountIndex]);

    // Update the balance
    addBalance(argv[accountIndex], amount);

    // Record the transaction
    FILE* file = fopen("transactions.txt", "a");
    if (file == NULL) {
        printf("Error opening transactions file.\n");
        return;
    }

    // Assuming "DEPOSIT" as the source for deposit transactions
    fprintf(file, "%s: DEPOSIT sent %s %d\n", argv[timestampIndex], argv[accountIndex], amount);
    fclose(file);
    printf("Deposited %s to account %s at timestamp %s\n", argv[amountIndex], argv[accountIndex], argv[timestampIndex]);
}

void handleBalance(int argc, char* argv[]) {
    int accountIndex = findArgIndex(argc, argv, "--account") + 1;

    if (accountIndex <= 0) {
        printf("Usage: tx balance --account <account>\n");
        return;
    }

    showBalance(argv[accountIndex]);
}

void loadTransactions() {
    FILE* file = fopen("transactions.txt", "r");
    if (file == NULL) {
        printf("No transactions file found. Starting fresh.\n");
        return;
    }

    unsigned int timestamp;
    char from[50], to[50];
    int amount;

    while (fscanf(file, "%u: %s sent %s %d\n", &timestamp, from, to, &amount) == 4) {
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

void handleTransfer(int argc, char* argv[]) {
    int fromIndex = findArgIndex(argc, argv, "--from") + 1;
    int toIndex = findArgIndex(argc, argv, "--to") + 1;
    int amountIndex = findArgIndex(argc, argv, "--amount") + 1;
    int timestampIndex = findArgIndex(argc, argv, "--timestamp") + 1;

    if (fromIndex <= 0 || toIndex <= 0 || amountIndex <= 0 || timestampIndex <= 0) {
        printf("Usage: /tx transfer --from <source> --to <destination> --amount <amount> --timestamp <timestamp>\n");
        return;
    }

    int amount = atoi(argv[amountIndex]);
    unsigned int timestamp = (unsigned int)atoi(argv[timestampIndex]);
    if (amount <= 0) {
        printf("Invalid transfer amount.\n");
        return;
    }

    addTransaction(argv[fromIndex], argv[toIndex], amount, timestamp);
}

void handleWithdraw(int argc, char* argv[]) {
    int accountIndex = findArgIndex(argc, argv, "--account") + 1;
    int amountIndex = findArgIndex(argc, argv, "--amount") + 1;

    if (accountIndex <= 0 || amountIndex <= 0) {
        printf("Missing arguments for withdraw command.\n");
        return;
    }

    int amount = atoi(argv[amountIndex]);
    if (amount <= 0) {
        printf("Invalid withdraw amount.\n");
        return;
    }

    int balance = getBalance(argv[accountIndex]);
    if (balance < amount) {
        printf("Insufficient funds for withdrawal.\n");
        return;
    }

    // Update the balance by subtracting the withdrawal amount
    addBalance(argv[accountIndex], -amount);

    // Record the withdrawal in the transactions file
    FILE* file = fopen("transactions.txt", "a");
    if (file == NULL) {
        printf("Error opening transactions file.\n");
        return;
    }

    // Assuming "WITHDRAWAL" as the source for withdrawal transactions
    fprintf(file, "0: %s sent WITHDRAWAL %d\n", argv[accountIndex], amount);
    fclose(file);
    printf("Withdrew %d from account %s\n", amount, argv[accountIndex]);
}


int main(int argc, char* argv[]) {
    loadTransactions();
    initializeBanks(); // Initialize banks with starting balances

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            if (argc == 2) {
                // General help
                showGeneralHelp();
            }
            else if (i > 1) { // Help for a specific command, based on argument before --help
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
                // Add else if branches for other commands as needed
            }
            return 0; // Exit after showing help
        }
    }


    if (argc < 2) {
        printf("Usage: tx <command> [arguments]\n");
        return 1;
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
        if (argc == 4) {
            deleteTransaction((unsigned int)atoi(argv[3]));
        }
        else {
            printf("Usage: /tx delete --timestamp <timestamp>\n");
        }
    }
    else if (strcmp(argv[1], "batch") == 0) {
        processBatch(argv[2]);
    }
    else if (strcmp(argv[1], "withdraw") == 0) {
        handleWithdraw(argc, argv);
    }

    else {
        printf("Unknown command. Supported commands are 'deposit', 'transfer', 'balance', 'settle', 'delete', 'batch'.\n");
    }

    return 0;
}

void initializeBanks() {
    // Initialize banks with starting balances
    addBalance(BANK_US, 100);
    addBalance(BANK_EU, 80);
    addBalance(BANK_AS, 20);
}

void addBalance(const char* account, int amount) {
    for (int i = 0; i < balanceCount; i++) {
        if (strcmp(balances[i].account, account) == 0) {
            balances[i].balance += amount;
            return;
        }
    }
    // If the account was not found, add a new one
    strcpy(balances[balanceCount].account, account);
    balances[balanceCount].balance = amount;
    balanceCount++;

}

void addTransaction(const char* from, const char* to, int amount, unsigned int timestamp) {
    // Before processing the transaction, check if 'from' account has any backing
    for (int i = 0; i < backingCount; i++) {
        if (strcmp(backings[i].account, from) == 0 &&
            timestamp >= backings[i].startTimestamp &&
            timestamp <= backings[i].startTimestamp + backings[i].duration) {
            // This transaction is backed. Adjust the logic to consider the backing limit
            // and the balance of the backing account ('backedBy')
            // Ensure not to violate the backing limit and update balances accordingly
        }
    }

    // Check for sufficient funds in the source account for transfers (excluding deposits)
    if (strcmp(from, "DEPOSIT") != 0 && getBalance(from) < amount) {
        printf("Transaction failed: Insufficient funds in %s's account.\n", from);
        return;
    }

    // Basic validation of parameters
    if (amount <= 0 || strlen(from) == 0 || strlen(to) == 0) {
        printf("Invalid transaction data.\n");
        return;
    }

    FILE* file = fopen("transactions.txt", "a");
    if (file == NULL) {
        printf("Error opening transactions file.\n");
        return;
    }

    // Record the transaction
    fprintf(file, "%u: %s sent %s %d\n", timestamp, from, to, amount);
    fclose(file);
    printf("Transaction from %s to %s for amount %d at timestamp %u added.\n", from, to, amount, timestamp);

    // Update balances
    if (strcmp(from, "DEPOSIT") != 0) {
        addBalance(from, -amount); // Deduct from sender
    }
    addBalance(to, amount); // Add to receiver
}

// To get balance of an account for checking before transfer
int getBalance(const char* account) {
    for (int i = 0; i < balanceCount; i++) {
        if (strcmp(balances[i].account, account) == 0) {
            return balances[i].balance;
        }
    }
    // If the account is not found, assume a balance of 0
    return 0;
}



void settleAccounts() {
    FILE* file = fopen("transactions.txt", "r");
    if (file == NULL) {
        printf("Error: No transactions found for settling.\n");
        return;
    }

    unsigned int timestamp;
    char from[50], to[50];
    int amount;

    while (fscanf(file, "%u: %s sent %s %d\n", &timestamp, from, to, &amount) == 4) {
        addBalance(from, -amount);
        addBalance(to, amount);
    }

    fclose(file);

    // Example output, does not perform actual settlement
    for (int i = 0; i < balanceCount; i++) {
        if (balances[i].balance != 0) {
            printf("%s has a net balance of %d\n", balances[i].account, balances[i].balance);
        }
    }

    printf("Settlement transactions (simplified):\n");
    // This is a very simplified and not optimized way of settling accounts
    for (int i = 0; i < balanceCount; i++) {
        if (balances[i].balance > 0) {
            for (int j = 0; j < balanceCount; j++) {
                if (balances[j].balance < 0) {
                    // Assuming we can settle the whole debt for simplicity
                    printf("%s sends %s: %d\n", balances[j].account, balances[i].account, balances[i].balance);
                    balances[j].balance += balances[i].balance;
                    balances[i].balance = 0;
                    break; // This is a simplification
                }
            }
        }
    }
}


void showBalance(const char* account) {
    if (strlen(account) == 0) {
        printf("Invalid account name.\n");
        return;
    }
    FILE* file = fopen("transactions.txt", "r");
    if (file == NULL) {
        printf("No transactions found.\n");
        return;
    }

    int balance = 0;
    unsigned int timestamp;
    char from[50], to[50];
    int amount;

    while (fscanf(file, "%u: %s sent %s %d\n", &timestamp, from, to, &amount) == 4) {
        if (strcmp(from, account) == 0) {
            balance -= amount;
        }
        if (strcmp(to, account) == 0) {
            balance += amount;
        }
    }

    fclose(file);
    printf("Balance for %s: %d\n", account, balance);
}

void deleteTransaction(unsigned int cutoffTimestamp) {
    FILE* file = fopen("transactions.txt", "r");
    if (file == NULL) {
        printf("Error opening transactions file for reading.\n");
        return;
    }

    FILE* tempFile = fopen("temp_transactions.txt", "w");
    if (tempFile == NULL) {
        printf("Error creating temporary transactions file.\n");
        fclose(file);
        return;
    }

    unsigned int timestamp;
    char from[50], to[50];
    int amount;

    // Read each transaction and write it to the temp file if it's older than the cutoff timestamp
    while (fscanf(file, "%u: %s sent %s %d\n", &timestamp, from, to, &amount) == 4) {
        if (timestamp > cutoffTimestamp) {
            fprintf(tempFile, "%u: %s sent %s %d\n", timestamp, from, to, amount);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Remove the original transactions file and rename the temporary file
    remove("transactions.txt");
    rename("temp_transactions.txt", "transactions.txt");

    printf("Transactions after timestamp %u have been deleted.\n", cutoffTimestamp);
}

void listAccounts() {
    if (balanceCount == 0) {
        printf("No accounts found.\n");
    }
    else {
        printf("List of accounts:\n");
        for (int i = 0; i < balanceCount; i++) {
            printf("%s\n", balances[i].account);
        }
    }
}

void processBatch(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Error opening batch file.\n");
        return;
    }

    char line[256], prevFrom[50] = "", prevTo[50] = "", prevAmount[50] = "", prevTimestamp[50] = "";
    while (fgets(line, sizeof(line), file)) {
        char from[50] = "", to[50] = "", amount[50] = "", timestamp[50] = "";

        sscanf(line, "%[^:]:%[^:]:%[^:]:%s\n", from, to, amount, timestamp);

        // Use previous values if current ones are missing
        if (strcmp(from, "") == 0) strcpy(from, prevFrom);
        if (strcmp(to, "") == 0) strcpy(to, prevTo);
        if (strcmp(amount, "") == 0) strcpy(amount, prevAmount);
        if (strcmp(timestamp, "") == 0) strcpy(timestamp, prevTimestamp);

        // Save current values for next iteration
        strcpy(prevFrom, from);
        strcpy(prevTo, to);
        strcpy(prevAmount, amount);
        strcpy(prevTimestamp, timestamp);

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
    // Parse command line arguments for the back command
    // You need to find indexes similar to previous handle functions and retrieve values
    // After retrieving and validating all necessary information, proceed to add a new backing relationship

    // Example to add a new backing relationship, details for parsing and validation are omitted for brevity
    strcpy(backings[backingCount].account, "EU");
    strcpy(backings[backingCount].backedBy, "US");
    backings[backingCount].limit = 20;
    backings[backingCount].startTimestamp = 2;
    backings[backingCount].duration = 10;
    backingCount++;
    // Inside handleBack function
    if (argc == 2) { // Only the 'back' command without additional arguments
        for (int i = 0; i < backingCount; i++) {
            printf("%s is backed by %s with a limit of %d from timestamp %u for duration %u\n",
                backings[i].account, backings[i].backedBy, backings[i].limit,
                backings[i].startTimestamp, backings[i].duration);
        }
    }
}

void handleBackedTransaction(char* from, const char* to, int amount, unsigned int timestamp) {
    // Extract the primary account and any backing accounts from the 'from' string
    // For simplicity, let's assume 'from' has a format like "primary(backer1,backer2,...)"
    char primaryAccount[50];
    char backingAccounts[100]; // Adjust size as needed
    sscanf(from, "%[^()](%[^)])", primaryAccount, backingAccounts);

    // Check if the primary account has sufficient balance
    int primaryBalance = getBalance(primaryAccount);
    if (primaryBalance >= amount) {
        // Primary account has enough balance; proceed with the transaction as usual
        addTransaction(primaryAccount, to, amount, timestamp);
        return;
    }

    // If the primary account does not have enough balance, check backing accounts
    char* token = strtok(backingAccounts, ",");
    while (token != NULL) {
        int backerBalance = getBalance(token);
        // Calculate how much of the amount the backer needs to cover
        int needed = amount - primaryBalance; // This needs adjustment based on your logic

        if (backerBalance >= needed) {
            // Backer has enough balance to cover the remaining amount
            // You might need to split the transaction, or adjust balances directly
            // For simplicity, this example will just print a message
            printf("Backed transaction: %s covers %d for %s\n", token, needed, primaryAccount);
            // Adjust balances as needed, record the transaction, etc.
            break;
        }

        token = strtok(NULL, ",");
    }

    // Note: This example doesn't fully implement the logic. You'll need to
    // - Deduct the amount covered by backers from their balances
    // - Add the transaction to the transactions file
    // - Handle cases where multiple backers are needed
    // - Ensure the total covered by backers does not exceed their limits
}

void showGeneralHelp() {
    printf("Usage: tx <command> [arguments]\n");
    printf("Commands:\n");
    printf("  deposit -- Deposits an amount to a specified account\n");
    printf("  transfer -- Transfers an amount from one account to another\n");
    printf("  balance -- Shows the balance of a specified account\n");
    // Add more commands as needed
    printf("Use 'tx <command> --help' for more information on a command.\n");
}

void showDepositHelp() {
    printf("Usage: tx deposit --account <account_name> --amount <amount> [--timestamp <timestamp>]\n");
    printf("Deposits the specified amount into the specified account.\n");
}

void showTransferHelp() {
    printf("Usage: tx transfer --from <source_account> --to <destination_account> --amount <amount> [--timestamp <timestamp>]\n");
    printf("Transfers the specified amount from the source account to the destination account.\n");
}

void showBalanceHelp() {
    printf("Usage: tx balance --account <account_name>\n");
    printf("Displays the current balance of the specified account.\n");
}

void showSettleHelp() {
    printf("Usage: tx settle\n");
    printf("Calculates and displays the settlements required to balance all accounts.\n");
}

void showDeleteHelp() {
    printf("Usage: tx delete --timestamp <timestamp>\n");
    printf("Deletes all transactions recorded after the specified timestamp.\n");
}

void showBatchHelp() {
    printf("Usage: tx batch <filepath>\n");
    printf("Processes a batch of transactions from a specified file.\n");
}

void showWithdrawHelp() {
    printf("Usage: tx withdraw --account <account_name> --amount <amount>\n");
    printf("Withdraws the specified amount from the specified account.\n");
}

void showBackHelp() {
    printf("Usage: tx back --account <account_name> --by <backing_account> --limit <limit> --timestamp <timestamp> --duration <duration>\n");
    printf("Allows an account to use another account's deposit for its transfers within specified limits and time frame.\n");
}