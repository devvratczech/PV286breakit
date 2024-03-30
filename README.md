Application for Bank Transactions

Table of Contents

Short Description

Full Description

Simple Example
Command-line interface
Additional Requirements
Account Names
Money Amounts
Timestamps


Subcommands

deposit subcommand
delete subcommand
balance subcommand
transfer subcommand
withdraw subcommand
batch subcommand
back subcommand
settle subcommand


Attacker Model


Short Description
Your task is to implement an application with a command-line interface (CLI)
for managing bank transactions.
The application will need to handle multiple CLI subcommands, arguments and
options that will alter its behaviour. Moreover, the application will need to
preserve an application state across multiple runs; state may be saved as
files. The CLI will allow the user to record transfers between various
accounts, query account balance, and use a smart debt-settling feature.
Given the CLI interface, an attacker may invoke the application any number of
times. Before any invocation, the attacker may modify the application's inputs
as she desires, including the command-line arguments and files saved to disk by
the application. The attacker may only invoke the application after it is done
running---you do not have to worry about the application running in parallel.
When given a valid input, the application shall behave as expected. If any
malicious input is provided, the application should not crash or enter an
inconsistent state (e.g., increase the balance of an account out of thin air).

Full Description
The aim is to implement a command-line (CLI) application tx that manages
money transactions among banks. The application will store the transaction data
(including those from past invocations) and handle common operations, such as
depositing, withdrawing, checking balance, transferring funds, etc. The
individual transactions will be timestamped. An advanced feature will be to
minimize the number of transactions that need to happen to settle the debts
among the banks.
Your solution has to be written from scratch and cannot reuse an existing code,
apart from the standard library features provided by the language of your
choice. However, you can use any other language for testing. Ask if you are
unsure whether you can use a certain feature.
The description is intentionally high-level to create room for your own
decisions. In case you encounter serious discrepancies or problems that require
further specification, feel free to ask through GitLab issues (you can also
create a merge request), in the course discussion forum or send an e-mail to
Jan Kvapil at kvapil@mail.muni.cz.
The examples invoke tx as a binary on Linux in a command-line environment. If
your language does not compile to a binary, use the standard way of running the
program in your target language.

Simple Example
Imagine three banks---American (US), European (EU) and Asian (AS)---with their
respective budgets being 100, 80 and 20. The application assumes a single
currency. Therefore, we are only interested in the transferred amount. The
Central Bank uses tx to record these obligations. It recorded the following
amounts: 20 from the American to the European bank, 10 from European bank to
the Asian bank, and finally, 10 from the Asian bank to the American bank. The
commands to record these using tx are:

$ ./tx deposit --account US --amount 100 --timestamp 0
$ ./tx deposit --account EU --amount 80 --timestamp 0
$ ./tx deposit --account AS --amount 20 --timestamp 0
$ ./tx transfer --from US --to EU --amount 20 --timestamp 1
$ ./tx transfer --from EU --to AS --amount 10 --timestamp 2
$ ./tx transfer --from AS --to US --amount 10 --timestamp 3


After entering all the previous transactions, it is possible to query the
banks' balances:

$ ./tx balance --account US
US:90
$ ./tx balance --account EU
EU:90
$ ./tx balance --account AS
AS:20


A lot of such transfers may be made, and your application must keep a record
of all the transfers. In order to settle the debts between the banks, the
Central Bank also wants to find the minimum number of transactions that must
actually happen (instead of requiring each transfer to be made separately). In
the above example, we can see that the Asian bank received 10 and sent 10, and
no transactions are therefore needed. The American bank owes 20 and is owed 10,
so it must send 10 to compensate. These 10 are exactly the 10 that the European
bank must receive. Therefore, the settlement can be reached within a single
transaction, US sending 10 to EU.
The settle command does not delete/override the state of the application, but
simply calculates the settlement and outputs it in a format
{from}:{to}:{amount}, one transaction per line. On a command-line it would
look as follows:

$ ./tx settle
US:EU:10



Command Line Interface
The application supports multiple subcommands:  deposit, withdraw,
balance, transfer, reset, batch, settle, and back. By executing one
of the subcommands deposit, withdraw, transfer or reset, the state
kept by the application is altered. If there is an error during processing of
the subcommand, however, the changes made by the subcommand must be fully
rolled back, such as in the case where account balance would go below zero.
Therefore, all modifications made by the subcommand must be valid for the
state to be updated. Note: The batch subcommand also changes the state,
but is allowed to fail partially; see below.
When no subcommand is given, the --list-accounts CLI option will print all
account names. For example:

$ ./tx --list-accounts
US
EU
AS


Whenever invocation results in an error, tx must return a non-zero exit code.
The order of command-line options for subcommands does not matter. Valid
command-line arguments must include a whitespace between the option and its
value, such as --account US. The application shall display basic help when
--help is used, which takes precedence over any other functionality. Help can
either be used on its own, or applied to a subcommand. Example valid
invocations for --help:

$ ./tx --help
...
$ ./tx deposit --help
...
$ ./tx deposit --account US --amount 10 --help
...



Additional Resources and Application State
The application must not make any network requests---while there is no need for
any networking, this is given as an explicit requirement. Any files or
directories created by your implementation must be created relative to the path
specified with --path {path} option. If {path} is not set, the current
working directory is used as the default value. This option aims to simplify
the job of testing and reviewing your application.

Account Names
The account names are set using --account {name}, where {name} is not
limited in size. Reasonable sizes must be supported---there can easily be
thousands of banks. Unreasonable sizes need to be handled gracefully (end with
an error exit code instead of crashing or corrupting). Valid account names
roughly match the regular expression ^([a-zA-Z]|[a-zA-Z][a-zA-Z0-9 _-]*[a-zA-Z0-9_-])$, i.e., the names can be a single character, must start
with a letter from the English alphabet and can contain alphanumerics, spaces,
dashes and underscores. However, account names cannot end with a whitespace. An
account is created by making a deposit to it (see later).
Account names containing whitespace need to be enclosed using double quotes
when passed as a command-line argument, such as --account "European Central Bank".

Money Amounts
A valid amount is a non-negative integer or decimal number (in the decimal
base). It can be left padded with 0 (the padding does not need to be stored).
A decimal number consists of two parts: {integral-part}.{fractional-part}.
Both the {integral-part} and the {fractional-part} of a valid amount (as
well as a valid account balance) fit into 32 bits. Examples of valid amounts:

0
01
1.0
0.4294967295
4294967295.4294967295


Examples of invalid amounts:

hundred
1.1.0
-1


Several subcommands accept the --amount {amount} argument. When provided
multiple times (--amount {amount1} --amount {amount2}), the individual
amounts are summed together to be used. For example, the following two
invocations result in the same amount being used:

./tx [...] --amount 100 --amount 100 --amount 200
./tx [...] --amount 400



Timestamps
Each transaction has a timestamp assigned to it, the higher the timestamp the
newer the transaction. The timestamp is a 32-bit non-negative integer and has
to be explicitly set for each transaction.

deposit subcommand
This subcommand deposits the amount into one or more accounts (in case the
--account option is given multiple times). If a deposit is made to a
non-existent account, the account is considered created by that deposit.
Syntax:

deposit --account {name} [--account {name} ...] --amount {amount} [--amount {amount} ...]  --timestamp {timestamp}


Examples:

$ ./tx deposit --account US --amount 10.1 --timestamp 0
$ ./tx deposit --account EU --account AS --amount 13.13 --timestamp 2



delete subcommand
By default, this subcommand wipes the application's state, deleting resources
created in previous runs (the files used for storing application state), as if
the application was never invoked. When --timestamp {timestamp} is given, all
transactions that happened at {timestamp} or later, i.e., more recent ones,
are deleted. Syntax:

delete [--timestamp {timestamp}]


Example:

$ ./tx delete
$ ./tx deposit --account US --amount 100 --timestamp 0
$ ./tx balance --account US --timestamp 1
US:100
$ ./tx delete
$ ./tx balance --account US --timestamp 1
Error: US account does not exist



balance subcommand
This subcommand outputs the balances for the queried accounts. The balance
shows the amount in the account at the {timestamp}, therefore it includes
also all transactions older than the {timestamp}, and also those that
happened exactly at {timestamp}. If the --timestamp {timestamp} is not
provided, all transactions are use for calculating the balance. Syntax:

balance --account {name} [--account {name} ...] [--timestamp {timestamp}]


Example:

$ ./tx deposit --account US --amount 10.111 --amount 20 --timestamp 0
$ ./tx balance --account US --timestamp 0
US:30.111
$ 
$ ./tx delete
$
$ ./tx deposit --account EU --amount 10 --timestamp 1
$ ./tx balance --account EU --timestamp 0
EU:0
$ ./tx balance --account EU
EU:0



transfer subcommand
Allows to record a transaction between banks. Money cannot be transferred from
an account to itself---in particular, an account can appear at most once in a
valid invocation (including backing, see below). Each of the options --from {from} and --to {to} must be specified at least once. Repeating them results
in all pair-transactions being recorded (each --from with each --to).
Syntax:

transfer --from {from} [--from {from} ...] --to {to} [--to {to} ...] --amount {amount} [--amount {amount} ...] --timestamp {timestamp}


Examples:

$ ./tx deposit --account US --amount 40 --timestamp 0
$ ./tx deposit --account EU --amount 0 --timestamp 0
$ ./tx deposit --account AS --amount 0 --timestamp 0
$ ./tx transfer --from US --to EU --to AS --amount 10 --amount  10 --timestamp 1
$ ./tx balance --account US --timestamp 2
US:0
$ ./tx balance --account EU --timestamp 2
EU:20
$ ./tx balance --account AS --timestamp 2
AS:20


For a valid transfer all the accounts used must exist. The {from} account
must have non-negative balance after the transfer. If an account is backing
another account at a certain {timestamp} it cannot make a regular
(non-backing) transfer that would make the account balance go below the
{limit} set in backing. See back subcommand for more details. If any of
these contraints don't hold the changes done in that execution are rolled back.

withdraw subcommand
This subcommand is the opposite to the deposit subcommand. It withdraws the
amount from the specified accounts. Withdrawing cannot make the account balance
go below zero, but has to error. Also, the already recorded transactions that
are newer than the withdrawal have to be verified. If any of the withdrawals
cannot happen, either because the account does not have the balance at that
point in time or the already recorded transactions would be invalid, the
execution fails and the withdrawals are rolled back (the state doesn't
change). Syntax:

withdraw --account {name} [--account {name} ...] --amount {amount} [--amount {amount} ...]  --timestamp {timestamp}


Examples:

$ ./tx deposit --account US --amount 10 --amount 20 --timestamp 0
$ ./tx balance --account US --timestamp 1
US:30
$ ./tx withdraw --account US --amount 20 --timestamp 2
$ ./tx balance --account US --timestamp 3
US:10
$ ./tx balance --account US --timestamp 0
US:30
$ ./tx withdraw --account US --amount 20
Error: US account has only 10
$ ./tx deposit --account EU --amount 10 --timestamp 1
$ ./tx transfer --from EU --to US --amount 10 --timestamp 3
$ ./tx withdraw --account EU --amount 10 --timestamp 2
Error: withdrawing would invalide future transaction(s) EU:US:10:3



batch subcommand
The batch subcommand reads a text file that contains multiple transactions.
Each line in the file corresponds to a single transaction. Syntax:

batch {filepath}


The fields are split by the colon : character and the lines by the newline
\n one. The transactions file formats are the following (the second line
describes how to specify a backed transfer):

{from}:{to}:{amount}:{timestamp}
{from}({account},...):{to}:{amount}:{timestamp}
...


Note: in order for backed transaction to be entered with the batch subcommand
the back subcommand has to be used prior to describe the backing parameters.
Starting from the second row, not all values are required. If a value is left
out, the value from the previous line will be reused. For example:

American Bank:European Bank:100.10:10
             :Asian Bank   :      :100


There are two transactions in the example file; one from the American Bank to
the European Bank (amounting to 100.10) and another one from the American Bank to the Asian Bank (also amounting to 100.10) ninety seconds later.
The optional whitespace character   that is not a part of the account names,
i.e., in the middle of it, is skipped when parsing the values in the fields.
For each transaction in the file the same transfer subcommand validation
rules apply. However, when an invalid transactions is observed, processing of
the transactions is stopped and non-zero exit code is returned. The valid ones
are committed and the rest are outputted, starting with the first invalid one.
Backed transactions can also be entered through batch command, see below.
Example:

$ ./tx deposit --account US --amount 10 --timestamp 2
$ ./tx deposit --account EU --amount 10.33 --timestamp 2
$ cat transactions.txt
US:EU:5:3
US:EU:5:1
US:EU:5:4
$ ./tx batch ./transactions.txt
US:EU:5:1
US:EU:5:4
$ ./tx balance --account US --timestamp 4
US:5:4
$ ./tx balance --account EU --timestamp 4
EU:15.33:4


In the example above the second transaction US:EU:5:1 is not valid, because
at timestamp 1 US does have 0 balance. The last transaction US:EU:5:4
is therefore also not recorded, even though it would pass on its own.

back subcommand
Backing allows an account to use another account's deposit for its transfers.
It has to be explicitly allowed using the back command. The amount allowed
for backing is limited using --limit {limit}. Finally, backing is valid in a
predefined time range (starting at a {timestamp}, inclusive, and continues
for the {duration} ticks, inclusive). Syntax:

back --account {account} --by {account} --limit {limit} --timestamp {timestamp} --duration {duration}]


When all options are omitted the subcommand shows existing backings in the
following format:

{account}:{by}:{limit}:{timestamp}:{duration}
...


Example:

$ ./tx deposit --account US --amount 100 --timestamp 1
$ ./tx deposit --account EU --amount 100 --timestamp 2
$ ./tx back --account EU --by US --limit 20 --timestamp 2 --duration 10
$ ./tx back
EU:US:20:2:10


At timestamp 2, both US and EU hold 100 in their accounts. Moreover,
the back subcommand allows EU to transfer up to 20 from US, starting
from timestamp 2 and lasting for 10 ticks. Such backed up transfers are
done with regular transfer subcommand with a special syntax {account} ({account},...). A transfer can be backed by multiple accounts. The account
that provides backing (US in the example) must have at least the {limit}
balance for the {duration}, starting at {timestamp}---instead of going
below 0 the account balance cannot go below {limit} for the {duration}.

$ ./tx deposit --acount AS --amount 0 --timestamp 1
$ ./tx transfer --from "EU(US)" --to AS --amount 110 --timemstamp 12
$ ./tx balance --account EU
EU:0
$ ./tx balance --account US
US:90
$ ./tx balance --account AS
AS:110


The last transaction is valid, eventhough EU has only 100 in their account,
because the extra 10 are taken from US. An account can be backed by
multiple other accounts and also can back multiple other ones. However,
circular backing is not allowed and an account can still appear at most once in
the command-line options. When multiple backing accounts are set the order of
withdrawals respects the order (left to right) from the options, e.g., in
--from EU(US, AS) the balance of EU decreases first, then US one and
finally the AS balance.

settle subcommand
The Central Bank wants to enforce the recorded obligations to be settled. To
save up on resources the settle commands tries to find the minimum number of
transactions needed to end up in the exact same state as if every transaction
was performed (see the Simple Example section). This is a difficult
optimization problem. Therefore settle is an advanced feature that will be
tested at the end of the course and the best solutions (with minimum
settlements across multiple tests) will be awarded bonus points. When
{timestamp} is set, the transactions newer than the {timestamp} are not
included.
The settle command does not remove any of the existing transactions that were
stored. It's purpose is to calculate a smaller number of transactions that
would result in the exact final balances as the transactions already recorded
with tx. Syntax:

settle [--timestamp {timestamp}]


The output format of newline separated transactions:

{from}:{to}:{amount}
...


To settle the debts in small number of transactions is not an easy task. There are several materials that you can consult:

https://web.archive.org/web/20190214205754/http://www.settleup.info/files/master-thesis-david-vavra.pdf
https://stackoverflow.com/questions/974922/algorithm-to-share-settle-expenses-among-a-group
https://hackernoon.com/adventures-in-programming-interviews-misleadingly-difficult-np-hard-problem-43092597018c
https://medium.com/@mithunmk93/algorithm-behind-splitwises-debt-simplification-feature-8ac485e97688


Attacker Model
The attacker can make any number of consequent runs of the application (not in
parallel) and also modify the inputs (i.e., the CLI arguments and also any
files created by the application) as she desires and the environment the
application is executed in. In case of valid inputs, the application shall
proceed as specified above; in case of malicious (invalid) ones the application
shouldn't crash or get into an incosistent behaviour (e.g., increase the
balance of an account out of thin air), etc. The attacker cannot modify the
binary. In other words, attacks working on modified binaries are disregarded.
