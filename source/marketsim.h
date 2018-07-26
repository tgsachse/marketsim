// Part of Marketsim by Tiger Sachse.

#include <limits.h>

// Both of these must be identical. The string version is for print
// formatting and safe scanning.
#define TICKER_LENGTH 4
#define TICKER_LENGTH_STR "4"

#define FAILURE INT_MIN
#define SUCCESS INT_MIN + 1
#define DEFAULT_MARKET "market.txt"
#define DEFAULT_PORTFOLIO "portfolio.txt"

// Node with various share information.
typedef struct Share {
    double count;
    struct Share* next;
    char ticker[TICKER_LENGTH + 1];
} Share;

// Portfolio that holds a linked list of shares and a balance.
typedef struct Portfolio {
    Share *head;
    double balance;
} Portfolio;

// Node with various stock information.
typedef struct Stock {
    double price;
    double totalShares;
    struct Stock* next;
    char ticker[TICKER_LENGTH + 1];
} Stock;

// Market that holds a linked list of stocks.
typedef struct Market {
    Stock* head;
} Market;

// Functional prototypes.
Stock* createStock(char*, double, double, Stock*);
void printStock(Stock*);
void destroyStockList(Stock*);
Market* createMarket(char*);
int insertStock(Market*, char*, double, double);
Stock* getStock(Market*, char*);
int buyStock(Market*, Portfolio*, char*, double);
int sellStock(Market*, Portfolio*, char*, double);
void printMarket(Market*);
void updateMarket(Market*);
int saveMarket(Market*, char*);
void destroyMarket(Market*);
Share* createShare(char*, double, Share*);
void destroyShareList(Share*);
Portfolio* createPortfolio(char*);
int insertShare(Portfolio*, char*, double);
Share* getShare(Portfolio*, char*);
void printPortfolio(Portfolio*);
int savePortfolio(Portfolio*, char*);
void destroyPortfolio(Portfolio*);
void cleanSTDIN(void);
void printHelp(void);
void mainMenu(Market*, Portfolio*);
