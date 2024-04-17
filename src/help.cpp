#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Bank identifiers


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
