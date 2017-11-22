/* A simple forking TCP Server
*/

#define _GNU_SOURCE   /* setresuid */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h> 
#include <netinet/in.h>

#include "libinetsec.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAX_TRANSACTIONS 20
#define MAX_ACCOUNTS 10

#define LOGFILE "./banking.log"

//#define FORK 

FILE *f;
int newsockfd;
char seed1[10],seed2[10];
char *user, *pass;
char msg[2048] = {0};


void handle_sig(int signal)
{
    wait3(NULL, WNOHANG, NULL); 
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}


typedef struct {
    char holder[32];
    char *type;
} account;

typedef struct {
    char holder[32];
    char *type;
    char CARD_NUMBER[30];
    char EXPIRATION_DATE[17];
} CC;

typedef struct {
    char holder[32];
    char *type;
    char IBAN[35];
    char BIC[12];
} BA;

typedef struct {
    char receiver[32];
    long amount;
    char comment[47];
} Transaction;


Transaction* transactions[MAX_TRANSACTIONS] = {NULL};
int transactions_used[MAX_TRANSACTIONS] = {0};

account* accounts[MAX_ACCOUNTS] = {NULL};
int accounts_used[MAX_ACCOUNTS] = {0};

// Credit Card or Bank Account
char *account_type[2] = { "CC", "BA" };


void get_input( char* prompt, char* buffer, size_t size )
{
    printf("%s: ", prompt);
    fgets(buffer, size, stdin);
    size_t len = strlen(buffer)-1;
    if (buffer[len] == '\n') {
        buffer[len] = '\0';
    }
}


int get_account_slot()
{
    for(int i = 0; i < MAX_ACCOUNTS; i++) {
        if(!accounts_used[i]){
            return i;
        }
    }
    return -1;
}


void add_CC()
{
    int index = get_account_slot();
    if( index != -1 ){
        CC* cc = malloc(sizeof(CC));
        cc->type = account_type[0];
        get_input("Holder", cc->holder, sizeof(cc->holder));
        get_input("Card Number", cc->CARD_NUMBER, sizeof(cc->CARD_NUMBER));
        get_input("Expiration Date", cc->EXPIRATION_DATE, sizeof(cc->EXPIRATION_DATE));
        accounts_used[index] = 1;
        accounts[index] = (account*) cc;
    }
}


void add_BA()
{
    int index = get_account_slot();
    if( index != -1 ){
        BA* ba = malloc(sizeof(BA));
        ba->type = account_type[1];
        get_input("Holder", ba->holder, sizeof(ba->holder));
        get_input("IBAN", ba->IBAN, sizeof(ba->IBAN));
        get_input("BIC", ba->BIC, sizeof(ba->BIC));
        accounts_used[index] = 1;
        accounts[index] = (account*) ba;
    }
}


void del_account(unsigned int id)
{
    if( id < MAX_ACCOUNTS && accounts_used[id]) {
        accounts_used[id] = 0;
        free(accounts[id]);
    }
}

void list_accounts()
{
    for( int i=0; i < MAX_ACCOUNTS; i++) {
        if( accounts_used[i]){
            printf("%d: %-32s (%s)\n", i, accounts[i]->holder, accounts[i]->type);
        }
    }
}

void show_account(unsigned int id)
{
    if( accounts[id] != NULL) {
        if(accounts[id]->type == account_type[0]) {
            CC* cc = (CC*) accounts[id];
            printf("%s: %-32s %s %s\n", cc->type, cc->holder, cc->CARD_NUMBER, cc->EXPIRATION_DATE);
        }
        else {
            BA* ba = (BA*) accounts[id];
            printf("%s: %-32s %s %s\n", ba->type, ba->holder, ba->IBAN, ba->BIC);

        }
    }
}


int get_transaction_slot()
{
    for(int i = 0; i < MAX_TRANSACTIONS; i++) {
        if(!transactions_used[i]) {
            return i;
        }
    }
    return -1;
}


void list_transactions()
{
    for( int i=0; i < MAX_TRANSACTIONS; i++) {
        if( transactions_used[i]) {
            printf("%d: %-32s %20lu\n", i, transactions[i]->receiver, transactions[i]->amount);
        }
    }
}


void show_transaction(unsigned int id)
{
    if( id < MAX_TRANSACTIONS && transactions_used[id]) {
        printf("%-32s %20lu %s\n", transactions[id]->receiver, transactions[id]->amount, transactions[id]->comment);
    }
}


void del_transaction(unsigned int id)
{
    if( id < MAX_TRANSACTIONS && transactions_used[id]) {
        transactions_used[id] = 0;
        free(transactions[id]);
    }
}

void add_transaction()
{
    char amount_buffer[50];
    int index = get_transaction_slot();
    if( index != -1 ) {
        Transaction* ta = malloc(sizeof(Transaction));
        get_input("Receiver", ta->receiver, sizeof(ta->receiver));
        get_input("Amount", amount_buffer, sizeof(amount_buffer));
        sscanf(amount_buffer,"%lu", &ta->amount);
        get_input("Comment", ta->comment, sizeof(ta->comment));
        transactions_used[index] = 1;
        transactions[index] = ta;
    }
}

void change_amount( unsigned int id )
{
    char amount_buffer[50];
    if(transactions_used[id]) {
        get_input("Amount", amount_buffer, sizeof(amount_buffer));
        sscanf(amount_buffer,"%lu", &transactions[id]->amount);
    }
}

void print_usage()
{
    printf(ANSI_COLOR_YELLOW);
    printf(".-------------------------------------------.\n");
    printf("|  ?,h      help                            |\n");
    printf("|  u        update username                 |\n");
    printf("+----- Accounts ----------------------------+\n");
    printf("|  A[C|B]   add [Credit Card|Bank Account]  |\n");
    printf("|  L        list accounts                   |\n");
    printf("|  D[id]    delete account by id            |\n");
    printf("|  S[id]    show account by id              |\n");
    printf("+----- Transactions ------------------------+\n");
    printf("|  a        add transaction                 |\n");
    printf("|  l        list transactions               |\n");
    printf("|  d[id]    delete transaction by id        |\n");
    printf("|  s[id]    show transaction by id          |\n");
    printf("|  c[id]    change transaction              |\n");
    printf("|  e        exit                            |\n");
    printf("`-------------------------------------------´\n");
    printf(ANSI_COLOR_RESET);
}

void handle_banking(int sock, char* uname)
{
    byte canary1;
    char username[128] = {0};
    byte canary2;
    int n;
    int id = 0;
    char cmd = '?';
    int exit_flag = 0;

    init_canary(&canary1,seed1);
    init_canary(&canary2,seed2);
    strncat( username, uname, sizeof(username)-1 );

    dup2(sock, STDOUT_FILENO);
    dup2(sock, STDERR_FILENO);
    dup2(sock, STDIN_FILENO);
    close(sock);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf( ANSI_COLOR_BLUE "Hello %s!\nWelcome to ROP Version 1.3.37\n" ANSI_COLOR_RESET, username);

    while(!exit_flag) {
        printf("> ");

        n = read(STDIN_FILENO, msg, sizeof(msg)-1);
        if (n < 0) {
            error("error: reading from socket");
            break;
        }
        if(n == 0) {
            printf("Error: No more data!\n");
            break;
        }

        msg[n] = '\0';
        sscanf(msg, "%c %d", &cmd, &id);
        switch (cmd) {
            case 'A':
                if(msg[1]=='C')
                    add_CC();
                else if(msg[1]=='B')
                    add_BA();
                else
                    print_usage();
                break;
            case 'L':
                list_accounts();
                break;
            case 'D':
                del_account(id);
                break;
            case 'S':
                show_account(id);
                break;
            case 'a':
                add_transaction();
                break;
            case 'l':
                list_transactions();
                break;
            case 'd':
                del_transaction(id);
                break;
            case 's':
                show_transaction(id);
                break;
            case 'c':
                change_amount(id);
                break;
            case 'u':
                memcpy( username, msg+2, n-2);
                break;
            case 'e':
                printf("Goodbye!\n");
                exit_flag = 1;
                break;
            case 'h':
            case '?': 
            default:
                print_usage();
                break;
        }
    }

    for(n=0; n < MAX_TRANSACTIONS; n++) {
        del_transaction(n);
    }

    for(n=0; n < MAX_ACCOUNTS; n++) {
        del_account(n);
    }


    if (!check_canary(&canary1,seed1) || !check_canary(&canary2,seed2)) {
        fprintf(stderr, ANSI_COLOR_RED "STACK SMASHING DETECTED\n" ANSI_COLOR_RESET);
        exit(0);	
    }

    return;
}



void handle_con(int sock)
{
    int n, uid, tuid;
    size_t len;
    char msg[128];

    char header[] = "  _____ _  _ ___   ___            _   \n\
 |_   _| || | __| | _ ) __ _ _ _ | |__\n\
   | | | __ | _|  | _ \\/ _` | ' \\| / /\n\
   |_| |_||_|___| |___/\\__,_|_||_|_\\_\\\n\
   \n\
   Remote Access Portal\n\nLogin: ";

    write(sock, header, sizeof(header));

    len = sizeof(msg);
    memset(msg, 0, len);

    n = read(sock, msg, len - 1);
    if (n < 0) 
        error("error: reading from socket");

#ifdef DEBUG
    printf("%s\n", msg);
#endif

    /* very simple authentication */
    msg[10] = '\0';
    msg[19] = '\0';

#ifdef DEBUG
    printf("%s\n", msg);
    printf("%s\n", msg+11);
    printf("%s\n", msg+20);
#endif

    /* 
     * login with your user id and the password in the challenge description
     * 	e.g.	"inetsec999:XXXXXXXX"
     *
     */
    user = msg;

    /* 
     * you will find your password in your challenge description
     *
     * please check twice before sending us an email ;-) 
     *
     */
    pass = msg + 11;

    if ((uid = auth_user(user, pass)) != 0) {

        /* change to inetsec user on bandit */
        printf("authenticated user %s with passwd %s: uid %i\n", user,pass, uid);
        fprintf(f,"authenticated user %s with passwd %s: uid %i\n",user,pass, uid);

        tuid = uid; 
        if (setresuid(tuid, tuid, tuid) < 0) {
            printf("error: setting permissions\n");
            fprintf(f, "error: setting permissions\n");
            error("error: setting permissions");
        }

        strncpy(seed1, user, 10);
        strncpy(seed2, pass, 10);

        handle_banking(sock, user);

    }  
    else {
        printf("user: \"%s\", passwd: \"%s\" Access denied\n",user,pass);
        fprintf(f,"user: \"%s\", passwd: \"%s\" Access denied\n",user,pass);
        fflush(f);
    }
}



int main(int argc, char *argv[])
{
#ifdef FORK
    int pid;
#endif

    unsigned int clilen;
    int sockfd, portno, on;
    struct sockaddr_in serv_addr, cli_addr;

    f=fopen( LOGFILE ,"a");

    signal(SIGCHLD, handle_sig);

    if (argc < 2) {
        fprintf(stderr,"error: no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("error: opening socket");

    on = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        error("error: set socket option");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("error: bind");

    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    while (1) {

        //printf("server: Listening to incoming connections...\n\n");

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) 
            error("error: accept");

#ifdef FORK
        pid = fork();

        if (pid < 0)
            error("error: fork");

        if (pid == 0)  {
            close(sockfd);
#endif


            /* drop privileges */
            handle_con(newsockfd);
#ifdef FORK
            exit(0);
        }
        else 
            close(newsockfd);
#endif


    }

    return 0; /* we never get here */

}


