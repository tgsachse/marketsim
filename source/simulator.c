// Part of Marketsim by Tiger Sachse.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "simulator.h"

// Create a new stock.
Stock* createStock(char* ticker, double price, double totalShares, Stock* next) {
    int i;
    Stock* stock;

    // Protect against invalid data.
    if (price <= 0 || totalShares <= 0) {
        printf("Stock price/share count must be positive.\n");

        return NULL;
    }

    // Attempt to allocate the stock.
    if ((stock = malloc(sizeof(Stock))) == NULL) {
        printf("Could not allocate memory for stock.\n");

        return NULL;
    }

    // Insert characters into the stock's ticker safely. Using strcmp here
    // could result in overflow if the ticker is too long.
    for (i = 0; i < TICKER_LENGTH; i++) {
        stock->ticker[i] = ticker[i];

        if (ticker[i] == '\0') {
            break;
        }
    }

    // If the ticker is exactly the TICKER_LENGTH + 1, then there won't be a
    // null terminator at the end, resulting in catastrophe. This line ensures
    // that the stock's ticker is null terminated.
    stock->ticker[TICKER_LENGTH] = '\0';

    stock->price = price;
    stock->totalShares = totalShares;
    stock->next = next;

    return stock;
}

// Print a stock to the screen.
void printStock(Stock* stock) {
    if (stock == NULL) {
        return;
    }

    printf("%" TICKER_LENGTH_STR "s -> ", stock->ticker);
    printf("total shares: %8.2lf, ", stock->totalShares);
    printf("price: $ %7.2lf, ", stock->price);
    printf("cap: $ %11.2lf\n", stock->totalShares * stock->price);
}

// Recursively destroy the stock list of a market.
void destroyStockList(Stock* stock) {
    if (stock == NULL) {
        return;
    }

    destroyStockList(stock->next); 
    free(stock);
}

// Create a new market.
Market* createMarket(char *marketFile) {
    FILE *f;
    double price;
    Market *market;
    double totalShares;
    char ticker[TICKER_LENGTH + 1];
    
    if ((f = fopen(marketFile, "r")) == NULL) {
        printf("Could not open file %s.\n", marketFile);

        return NULL;
    }

    if ((market = calloc(1, sizeof(Market))) == NULL) {
        printf("Could not allocate memory for market.\n");
        fclose(f);

        return NULL;
    }

    // Scan all of the stocks from file into a linked list in the market struct.
    while (fscanf(f, "%" TICKER_LENGTH_STR "s %lf %lf\n", ticker, &price, &totalShares) != EOF) {

        // If any of the calls to insertStock fail, then explode.
        if (insertStock(market, ticker, price, totalShares) == FAILURE) {
            free(market);
            fclose(f);

            return NULL;
        }
    }

    fclose(f);

    return market;
}

// Insert a stock into a market.
int insertStock(Market* market, char* ticker, double price, double totalShares) {
    Stock *stock;

    if (market == NULL) {
        printf("No market provided.\n");

        return FAILURE;
    }

    // Attempt to create the stock.
    if ((stock = createStock(ticker, price, totalShares, market->head)) == NULL) {
        return FAILURE;
    }

    // Insert the stock at the head of the list. The old head is set to
    // be the next pointer of the new stock in the call to createStock.
    market->head = stock;

    return SUCCESS;
}

// Get a stock in the market, if it exists.
Stock* getStock(Market* market, char* ticker) {
    Stock* current;
    
    if (market == NULL) {
        return NULL;
    }

    current = market->head;
    while (current != NULL) {
        if (strcmp(ticker, current->ticker) == 0) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

// Buy stock on the market.
int buyStock(Market* market, Portfolio* portfolio, char* ticker, double count) {
    Stock* stock;
    Share* share;
   
    // Check a bunch of conditions that would make this purchase impossible.
    if (market == NULL || portfolio == NULL) {
        printf("No market or portfolio provided.\n");

        return FAILURE;
    }

    if (count <= 0) {
        printf("Shares desired must be positive.\n");

        return FAILURE;
    }
    
    if ((stock = getStock(market, ticker)) == NULL) {
        printf("Stock not available.\n");

        return FAILURE;
    }

    if (count > stock->totalShares) {
        printf("Not enough shares available.\n");

        return FAILURE;
    }

    if (portfolio->balance < stock->price * count) {
        printf("You can't afford that trade.\n");

        return FAILURE;
    }

    // If the share isn't already in the portfolio, then add it.
    if ((share = getShare(portfolio, ticker)) == NULL) {
        if (insertShare(portfolio, ticker, count) == FAILURE) {
            return FAILURE;
        }
    }
    else {

        // Otherwise adjust the share count.
        share->count += count;
    }

    // Subtract the cost from the portfolio's balance.
    portfolio->balance -= stock->price * count;

    printf("Bought %.3lf shares of %s.", count, ticker);
    
    return SUCCESS;
}

int sellStock(Market* market, Portfolio* portfolio, char* ticker, double count) {
    Stock* stock;
    Share* share;
   
    // Check a bunch of conditions that would make this sale impossible.
    if (market == NULL || portfolio == NULL) {
        printf("No market or portfolio provided.\n");

        return FAILURE;
    }

    if (count <= 0) {
        printf("Shares to sell must be positive.\n");

        return FAILURE;
    }
    
    if ((share = getShare(portfolio, ticker)) == NULL) {
        printf("No shares of that stock in portfolio available.\n");

        return FAILURE;
    }

    if ((stock = getStock(market, ticker)) == NULL) {
        printf("That stock doesn't exist in the market.\n");

        return FAILURE;
    }

    if (count > share->count) {
        printf("Not enough shares available.\n");

        return FAILURE;
    }

    // Adjust the portfolio accordingly.
    share->count -= count;
    portfolio->balance += stock->price * count;

    printf("Sold %.3lf shares of %s.", count, ticker);

    return SUCCESS;
}

// Print a market to the screen.
void printMarket(Market* market) {
    Stock* current;

    if (market == NULL) {
        return;
    }

    current = market->head;
    while (current != NULL) {
        printStock(current);
        current = current->next;
    }
}

// Save a market to file.
int saveMarket(Market* market, char* marketFile) {
    FILE* f;
    Stock* current;

    if (market == NULL) {
        printf("No market provided.\n");

        return FAILURE;
    }

    if ((f = fopen(marketFile, "w")) == NULL) {
        printf("Could not open file %s.\n", marketFile);

        return FAILURE;
    }

    // Save all of the stocks as separate lines.
    current = market->head;
    while(current != NULL) {
        fprintf(f, "%s %lf %lf\n", current->ticker, current->price, current->totalShares);
        current = current->next;
    }

    fclose(f);

    return SUCCESS;
}

// Destroy all the contents of a market.
void destroyMarket(Market* market) {
    if (market == NULL) {
        return;
    }
    
    destroyStockList(market->head);
    
    free(market);
}

// Create a new share for a portfolio.
Share* createShare(char* ticker, double count, Share* next) {
    int i;
    Share* share;

    // Protect against invalid data.
    if (count < 0) {
        printf("Share count must be positive.\n");

        return NULL;
    }

    // Attempt to allocate the share.
    if ((share = malloc(sizeof(Share))) == NULL) {
        printf("Could not allocate memory for share.\n");

        return NULL;
    }

    // Insert characters into the share's ticker safely. Using strcmp here
    // could result in overflow if the ticker is too long.
    for (i = 0; i < TICKER_LENGTH; i++) {
        share->ticker[i] = ticker[i];

        if (ticker[i] == '\0') {
            break;
        }
    }

    // If the ticker is exactly the TICKER_LENGTH + 1, then there won't be a
    // null terminator at the end, resulting in catastrophe. This line ensures
    // that the share's ticker is null terminated.
    share->ticker[TICKER_LENGTH] = '\0';

    share->count = count;
    share->next = next;

    return share;
}

// Recursively destroy the share list of a portfolio.
void destroyShareList(Share* share) {
    if (share == NULL) {
        return;
    }

    destroyShareList(share->next); 
    free(share);
}

// Create a new portfolio.
Portfolio* createPortfolio(char *portfolioFile) {
    FILE *f;
    double shares;
    Portfolio *portfolio;
    char ticker[TICKER_LENGTH + 1];
    
    if ((f = fopen(portfolioFile, "r")) == NULL) {
        printf("Could not open file %s.\n", portfolioFile);

        return NULL;
    }

    if ((portfolio = calloc(1, sizeof(Portfolio))) == NULL) {
        printf("Could not allocate memory for portfolio.\n");
        fclose(f);

        return NULL;
    }

    fscanf(f, "%lf\n", &(portfolio->balance));

    // Scan all of the stocks from file into a linked list in the market struct.
    while (fscanf(f, "%" TICKER_LENGTH_STR "s %lf\n", ticker, &shares) != EOF) {
         
        // If any of the calls to insertStock fail, then explode.
        if (shares != 0 && insertShare(portfolio, ticker, shares) == FAILURE) {
            free(portfolio);
            fclose(f);

            return NULL;
        }
    }

    fclose(f);

    return portfolio;
}

// Insert a new share into a portfolio.
int insertShare(Portfolio* portfolio, char *ticker, double count) {
    Share* share;

    if (portfolio == NULL) {
        printf("No portfolio provided.\n");

        return FAILURE;
    }

    // Attempt to create the share.
    if ((share = createShare(ticker, count, portfolio->head)) == NULL) {
        return FAILURE;
    }

    // Insert the share at the head of the list. The old head is set to
    // be the next pointer of the new share in the call to createShare.
    portfolio->head = share;

    return SUCCESS;
}

// Get a share from a portfolio, if it exists.
Share* getShare(Portfolio* portfolio, char* ticker) {
    Share* current;
    
    if (portfolio == NULL) {
        return NULL;
    }

    current = portfolio->head;
    while (current != NULL) {
        if (strcmp(ticker, current->ticker) == 0) {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

// Print a portfolio to the screen.
void printPortfolio(Portfolio* portfolio) {
    Share* current;

    if (portfolio == NULL) {
        return;
    }

    printf("balance: %lf\n", portfolio->balance);

    current = portfolio->head;
    while (current != NULL) {
        printf("%" TICKER_LENGTH_STR "s -> shares: %8.2lf\n",
               current->ticker,
               current->count);
        current = current->next;
    }
}

// Save a portfolio to file.
int savePortfolio(Portfolio* portfolio, char* portfolioFile) {
    FILE* f;
    Share* current;

    if (portfolio == NULL) {
        printf("No portfolio provided.\n");

        return FAILURE;
    }

    if ((f = fopen(portfolioFile, "w")) == NULL) {
        printf("Could not open file %s.\n", portfolioFile);

        return FAILURE;
    }

    // The balance must be the first token in the file.
    fprintf(f, "%lf\n", portfolio->balance);

    // Save all of the shares as separate lines.
    current = portfolio->head;
    while(current != NULL) {
        fprintf(f, "%s %lf\n", current->ticker, current->count);
        current = current->next;
    }

    fclose(f);

    return SUCCESS;
}

// Destroy a portfolio.
void destroyPortfolio(Portfolio* portfolio) {
    if (portfolio == NULL) {
        return;
    }
    
    destroyShareList(portfolio->head);
    
    free(portfolio);
}

// Clear out stdin after a scanf call.
void cleanSTDIN(void) {
    while(getchar() != '\n');
}

// Print some friendly help.
void printHelp(void) {
    printf("\nEnter a command from the list below:\n");
    printf("v) view a specific stock\n");
    printf("a) view all stocks\n");
    printf("p) view portfolio\n");
    printf("b) buy stock\n");
    printf("s) sell stock\n");
    printf("u) force the market to update\n");
    printf("q) quit\n");
}

// The main menu of the CLI.
void mainMenu(Market* market, Portfolio* portfolio) {
    char command;
    Stock *stock;
    double count;
    char ticker[TICKER_LENGTH + 1];
    
    printf("Welcome to the Marketsim!\n");
    printHelp();

    command = '\0';
    while (command != 'q') {
        printf("\n> ");
        scanf(" %c", &command);
        cleanSTDIN();

        switch(command) {

            // View a specific stock.
            case 'v':
                printf("Enter the ticker of the stock you wish to view.\n> ");
                scanf("%" TICKER_LENGTH_STR "s", ticker);
                cleanSTDIN();
                
                if ((stock = getStock(market, ticker)) != NULL) {
                    printStock(stock);
                }
                else {
                    printf("Stock not found.\n");
                }
                break;

            // View all available stocks.
            case 'a':
                printf("Current market:\n");
                printMarket(market);
                break;

            // View the current portfolio.
            case 'p':
                printf("Current portfolio:\n");
                printPortfolio(portfolio);
                break;

            // Buy stock.
            case 'b':
                printf("Enter the ticker of the stock you wish to buy.\n> ");
                scanf("%" TICKER_LENGTH_STR "s", ticker);
                cleanSTDIN();
                
                printf("Enter the amount of shares you'd like to purchase.\n> ");
                scanf("%lf", &count);
                cleanSTDIN();
                
                buyStock(market, portfolio, ticker, count);
                break;

            // Sell stock.
            case 's':
                printf("Enter the ticker of the stock you wish to sell.\n> ");
                scanf("%" TICKER_LENGTH_STR "s", ticker);
                cleanSTDIN();

                printf("Enter the amount of shares you'd like to sell.\n> ");
                scanf("%lf", &count);
                cleanSTDIN();

                sellStock(market, portfolio, ticker, count);
                break;

            // Update the market.
            case 'u':
                break;

            // Quit the simulator.
            case 'q':
                break;

            // An invalid command was entered. Print the help menu.
            default:
                printHelp();
                break;
        }
    }
}

// Main entry point of the program.
int main(int argCount, char** argVector) {
    Market* market;
    Portfolio* portfolio;
   
    char* marketFile = DEFAULT_MARKET;    
    char* portfolioFile = DEFAULT_PORTFOLIO;

    // If a non-default market file is passed as the first argument, use it.
    if (argCount >= 2) {
        marketFile = argVector[1];
    }

    // If a non-default portfolio file is passed as the second argument, use it.
    if (argCount >= 3) {
        portfolioFile = argVector[2];
    }

    // Attempt to create the market.
    if ((market = createMarket(marketFile)) == NULL) {
        return -1;
    }

    // Attempt to create a portfolio.
    if ((portfolio = createPortfolio(portfolioFile)) == NULL) {
        destroyMarket(market);

        return -1;
    }

    mainMenu(market, portfolio);

    // Save everything to files.
    saveMarket(market, marketFile);
    savePortfolio(portfolio, portfolioFile);

    // Destroy all dynamic memory.
    destroyMarket(market);
    destroyPortfolio(portfolio);

    return 0;
}
