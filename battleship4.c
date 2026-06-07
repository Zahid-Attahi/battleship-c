
// Battleship 3
// Zahidullah 
// single-player battleship game with computer opponent placement and gameplay
 


#include <stdio.h>            // standard I/O header 
#include <stdlib.h>           // standard library header 
#include <stdbool.h>          // boolean type header 
#include <ctype.h>            // character helpers 
#include <string.h>           // string helpers 
#include <time.h>             // time functions for srand 
#include <math.h>             // math header 

#include <unistd.h>           // close() and POSIX functions  
#include <sys/types.h>        // socket type defs 
#include <sys/socket.h>       // socket API 
#include <netdb.h>            // getaddrinfo and related 
#include <arpa/inet.h>        // inet_ntop, inet_pton 
#include <errno.h>            // errno for recv errors 

#define SIZE 10               // board size constant 

// checking if a spot has an un-hit ship
#define IS_UNHIT_SHIP(type) (type >= CARRIER && type <= DESTROYER) // why: Improves readability of win/loss checks. 

// ship types with lengths
typedef enum { NONE, CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER, HIT_STATUS, MISS_STATUS } ShipType; // it define types of ships 
// it define types of ships
// why: makes it easy to store and identify ships on the board, and track when player's ship is hit/missed

// shot types
typedef enum { NOT_TRIED, HIT, MISS } ShotType; // shot enum 



// Grids
ShipType **shipGrid; // what is it?: player's grid for ships. 
//  and WHY: stores which ship is in each square

ShotType **shotGrid; // player's grid for shots on computer's board. 
// keeps track of player's shots

ShipType **computerShipGrid; // computer's ship grid 
// store computer's ship positions

ShotType **computerShotGrid; // computer's shot grid 




// Ship lengths 
const int SHIP_LENGTHS[] = { 5, 4, 3, 3, 2 }; // ship sizes array 

// our function declarations 
void prepare(void); // TODO: just shows the games instructions 
//  instruct player before placing ships

void endGame(void); // this line prints goodbye message 
// end game feedback

void InitializeGrids(void); // it allocates and init grids 


void TearDownGrids(void); //. it just  free allocated memory
// to prevent memory leaks

void DisplayWorld(void); // it  print current ship grid AND shot grid 
//  to  show player the status of both boards 

bool acceptShipInput(char *input, int length, int *rowStart, int *colStart, int *rowEnd, int *colEnd); // parse prototype 
// it parses, validates compact input , and calculates end coordinates based on length

bool updateGrid(ShipType ship, int rowStart, int colStart, int rowEnd, int colEnd); // update prototype 
// it updates ship grid
// WHY: places ship if valid, prevents overlap

void placeShips(void); // it lets player place all ships 
// why: it handles game loop for placement



void SetupSinglePlayer(void); // it sets up computer player 
//   computer needs ships and boards 

void TeardownSinglePlayer(void); // it frees computer memory 
//prevent memory leaks 

void UpdateState(void); // it handles game turns 


//   gameplay loop 
bool MakeSinglePlayerShot(int row, int col); // it processes player shot and checks computer's ship grid 
// WHY: check if player hit computer ship 

void GetSinglePlayerShot(int *row, int *col); // it gets computer shot 
// WHY?: computer picks target 

void SinglePlayerResponse(int row, int col, bool hit); // it updates computer shot grid 
//  track computer's shots 

bool SinglePlayerDidWin(void); // it checks if player won
//  detect game end 



#define NETBUF 128            // network buffer size for messages 

int start_server(const char *port); // start listening server 

int accept_client(int listenfd); // accept client connection 
int start_client(const char *ip, const char *port); // connect to server 

ssize_t send_line(int fd, const char *s); // send string helper 
ssize_t recv_line(int fd, char *buf, size_t maxlen); // recv until newline 

void play_twoplayer_server(int connfd); // server network gameplay 
void play_twoplayer_client(int sockfd); // client network gameplay 


int main(int argc, char *argv[]) { // updated main signature to support args 
    srand(time(0)); // seed random for computer 
    //why: computer needs random placements

    if (argc == 1) { // single-player mode if no args 

        prepare(); // show instructions  
        InitializeGrids(); // allocate grids 
        SetupSinglePlayer(); // place computer ships 

        placeShips(); // let player place ships 
        UpdateState(); // run original single-player loop 

        TeardownSinglePlayer(); // free computer grids 
        TearDownGrids(); // free player grids 
        endGame(); // goodbye 

        return 0; // exit success for single-player 

    } else if (argc == 2) { // server mode if one arg 

        const char *port = argv[1]; // port string from args 

        int listenfd = start_server(port); // start listening on port 
        if (listenfd == -1) { // check error 
            fprintf(stderr, "failed to start server on port bro %s\n", port); // print error 
            return 1; // error exit 
        }

        prepare(); // show instructions  
        InitializeGrids(); // allocate grids 

        
        placeShips(); // let the   local player place ships as before 

        printf("Waiting for client to connect on port %s...\n", port); // inform user 

        int connfd = accept_client(listenfd); // accept one client 
        close(listenfd); // close listening socket 
        if (connfd == -1) { // accept failed 
            fprintf(stderr, "Accept failed.\n"); // print error 
            TearDownGrids(); // cleanup 
            return 1; 
        }

        play_twoplayer_server(connfd); // play networked game as server 

        TearDownGrids(); // free grids after game  
        endGame(); // goodbye 
        return 0; // exit success 
    } else if (argc == 3) { // client mode if two args 
        const char *ip = argv[1]; // ip arg 
        const char *port = argv[2]; // port arg 

        int sockfd = start_client(ip, port); // connect to server 
        if (sockfd == -1) { // check connect error 
            fprintf(stderr, "Failed to connect to %s:%s\n", ip, port); // print error 
            return 1; // error exit 
        }

        prepare(); // show instructions  
        InitializeGrids(); // allocate grids 
       
        placeShips(); // let local player place ships 

        play_twoplayer_client(sockfd); // play networked game as client 

        TearDownGrids(); // free grids 
        endGame(); // goodbye 
        return 0; // exit success 
    } else { // wrong usage 
        fprintf(stderr, "Usage:\n"); // print usage 
        fprintf(stderr, "  %s            (single-player)\n", argv[0]); // usage single 
        fprintf(stderr, "  %s <port>     (server mode)\n", argv[0]); // usage server 
        fprintf(stderr, "  %s <ip> <port> (client mode)\n", argv[0]); // usage client 
        return 1; // error exit 
    }
} // end of main 

// showing the  instructions
void prepare(void) { // print welcome and instructions  
    printf("Welcome to Battleship 3!\n"); // greeting 

    printf("Place your ships on a 10x10 board.\n"); // placement instructions 
    
    printf("Enter start row, end row, and column (e.g., AE4 for rows A-E, column 4).\n"); // format 

    printf("Horizontal and vertical placement only. Diagonals not allowed !!.\n"); // placement constraint 
    printf("Then play against computer or (in network modes) another player!\n"); // note network mode (modified text) 
}

// our exit message
void endGame(void) { // goodbye message  
    printf("Game complete. thanks for playing Battleship bro. Goodbye! \n"); // goodbye 
}

// allocate memory and initialize grids

void InitializeGrids(void) { // allocate ship and shot grids  
    // allocation logic and failure checks for shipGrid and shotGrid

    shipGrid = malloc(SIZE * sizeof(ShipType*)); // allocate rows for shipGrid 
    shotGrid = malloc(SIZE * sizeof(ShotType*)); // allocate rows for shotGrid 

    if (!shipGrid || !shotGrid) { // check allocation 
        printf("Memory allocation failed.\n"); // error message 
        exit(1); // exit on failure 
    }
    for (int i = 0; i < SIZE; i++) { // allocate each row 

        shipGrid[i] = malloc(SIZE * sizeof(ShipType)); // allocate columns for shipGrid 
        shotGrid[i] = malloc(SIZE * sizeof(ShotType)); // allocate columns for shotGrid 

        if (!shipGrid[i] || !shotGrid[i]) { // check row allocation 
            printf("Memory allocation failed.\n"); // error 
            exit(1); // exit 
        }
        for (int j = 0; j < SIZE; j++) { // initialize cells 
            shipGrid[i][j] = NONE; // default no ship 
            shotGrid[i][j] = NOT_TRIED; // default no shot 
        }
    }
}

// setup computer player with random ships
void SetupSinglePlayer(void) { // allocate and fill computer grids  
    
    computerShipGrid = malloc(SIZE * sizeof(ShipType*)); // allocate computer ship rows 
    computerShotGrid = malloc(SIZE * sizeof(ShotType*)); // allocate computer shot rows 

    if (!computerShipGrid || !computerShotGrid) { // check allocation 

        printf("Memory allocation failed.\n"); // error message 
        exit(1); // exit 
    }
    for (int i = 0; i < SIZE; i++) { // allocate each computer row 

        computerShipGrid[i] = malloc(SIZE * sizeof(ShipType)); // allocate computer ship cols 
        computerShotGrid[i] = malloc(SIZE * sizeof(ShotType)); // allocate computer shot cols 

        if (!computerShipGrid[i] || !computerShotGrid[i]) { // check allocation 

            printf("Memory allocation failed.\n"); // error 
            exit(1); // exit 
        }
        for (int j = 0; j < SIZE; j++) { // initialize computer grids 
            computerShipGrid[i][j] = NONE; // default no ship 
            computerShotGrid[i][j] = NOT_TRIED; // default not tried 
        }
    }

    // place computer ships randomly, checking for bounds and overlap
    ShipType ships[] = { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER }; // ship list 

    for (int i = 0; i < 5; i++) { // iterate ships 
        bool placed = false;  // flag to check if the current ship is successfully placed 

        int length = SHIP_LENGTHS[i]; // length of current ship 

        while (!placed) { // attempt placement until success 

            bool horizontal = rand() % 2;   // random orientation  // why: random variety
            int rowStart, colStart, rowEnd, colEnd; // coordinates 

            if (horizontal) {  // horizontal placement 

      rowStart = rowEnd = rand() % SIZE; // random row when horizontal 
         colStart = rand() % (SIZE - length + 1); // random start col 
                colEnd = colStart + length - 1;  // dnding theeeeeeeeeee column based on length 

            } else { // vertical placemnet  
 
    colStart = colEnd = rand() % SIZE; // random column when vertical 
     rowStart = rand() % (SIZE - length + 1); // random start row 
    rowEnd = rowStart + length - 1; // ending row based on length 
            }

            // check for overlap
            bool overlap = false; // overlap flag 

            for (int r = rowStart; r <= rowEnd; r++) { // check each cell 

                for (int c = colStart; c <= colEnd; c++) {  // check each cell 
                     if (computerShipGrid[r][c] != NONE) { 
                        overlap = true; // mark overlap 
                        break; // break inner loop 
                    }
                }
                if (overlap) break; // break outer loop if overlap 
            }

            if (!overlap) { // if no overlap place ship 
                // Placing the ship
                for (int r = rowStart; r <= rowEnd; r++) { // nested loop to set cells 
                    for (int c = colStart; c <= colEnd; c++) { // nested loop 
                        computerShipGrid[r][c] = ships[i]; // mark grid with ship type 
                    }
                }
                placed = true; // placement done 
            }
        }
    }} 

// free   the memory
void TearDownGrids(void) { // free player grids 

    for (int i = 0; i < SIZE; i++) { // free rows 

        free(shipGrid[i]); // free ship row 
        free(shotGrid[i]); // free shot row 
    }
    free(shipGrid); // free shipGrid array 
    free(shotGrid); // free shotGrid array 
}

// free computer memory
void TeardownSinglePlayer(void) { // free computer grids 

    for (int i = 0; i < SIZE; i++) { // free rows 

        free(computerShipGrid[i]); // free computer ship row 
        free(computerShotGrid[i]); // free computer shot row 
    }
    free(computerShipGrid); // free computerShipGrid array 
    free(computerShotGrid); // free computerShotGrid array 
}

// display board 
void DisplayWorld(void) { // prints both boards 

    // Prints player's ship grid
    printf("\n--- YOUR SHIPS ---\n"); // header 
    printf("   "); // spacing for columns 

    for (int c = 0; c < SIZE; c++) printf(" %d ", c); // column numbers 
    printf("\n"); // newline 

    for (int r = 0; r < SIZE; r++) { // each row 
        printf("%c |", 'A' + r); // Row letter 

        for (int c = 0; c < SIZE; c++) { // each column 
            switch (shipGrid[r][c]) { // display player's ship status 

                case NONE:       printf(" . "); break; // empty 
                case CARRIER:    printf("CV "); break; // carrier 
                case BATTLESHIP: printf("BB "); break; // battleship 
                case CRUISER:    printf("CR "); break; // cruiser 
                case SUBMARINE:  printf("SS "); break; // submarine 
                case DESTROYER:  printf("DD "); break; // destroyer 
                case HIT_STATUS:   printf(" X "); break; // computer hit player's ship 
                case MISS_STATUS:  printf(" O "); break; // computer missed Player's ship 
            }
        }
        printf("\n"); // 
    }

    // prints player's shot grid 
    printf("\n--- YOUR SHOTS ---\n"); // shots header 
    printf("   "); // spacing for columns 

    for (int c = 0; c < SIZE; c++) printf(" %d ", c); // ccolumn numbers 
    printf("\n"); // newline 

    for (int r = 0; r < SIZE; r++) { // each row 

        printf("%c |", 'A' + r); // row letter 
        for (int c = 0; c < SIZE; c++) { // each column 

            switch (shotGrid[r][c]) { // Display player's shots 

                case NOT_TRIED:  printf(" . "); break; // not tried 
                case HIT:        printf(" X "); break; // player Hit Computer's Ship 
                case MISS:       printf(" O "); break; // PLayer Missed Computer's Ship 
            }
        }
        printf("\n"); // end row 
    }
    printf("\n"); // extra newline 
}

// we let the player place all ships
void placeShips(void) { // player places ships interactively 

    ShipType ships[] = { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER }; // ship list 
    char input[10]; // input buffer 

    for (int i = 0; i < 5; i++) { // for each ship 

        int length = SHIP_LENGTHS[i]; // ship length 
        int rowStart, colStart, rowEnd, colEnd; // placement coords 
        bool valid = false; // loop until valid 
        while (!valid) { // placement loop 

            printf("Please enter a location for a ship of %d squares (e.g. AE4): ", length); // prompt 

            if (!fgets(input, sizeof(input), stdin)) continue; // read input 
            input[strcspn(input, "\n")] = 0; // trim newline 

            if (acceptShipInput(input, length, &rowStart, &colStart, &rowEnd, &colEnd)) { // parse 
                if (updateGrid(ships[i], rowStart, colStart, rowEnd, colEnd)) { // place 
                    valid = true; // accepted 

                } else {
                    printf("Placement invalid: Overlap or out of bounds. Try again bru.\n"); // overlap msg 
                }
            } else {
                printf("Placement invalid: Check format (R1R2C) or length (%d). Try again man !.\n", length); // format msg 
            }
        }
        DisplayWorld(); // show boards after placement 
    } // end for 
} // end placeShips 

// parse input and check validity
bool acceptShipInput(char *input, int length, int *rowStart, int *colStart, int *rowEnd, int *colEnd) { // parse placement string 
    int len = strlen(input); // length of input 
    
    
    if (len < 3 || !isalpha(input[0]) || !isalpha(input[1])) { // basic format check 
        return false; // invalid 
    }

    
    *rowStart = toupper(input[0]) - 'A'; // parse start row letter 
    *rowEnd = toupper(input[1]) - 'A'; // parse end row letter 

    // use atoi for column part, which starts at index 2
    *colStart = *colEnd = atoi(&input[2]); // parse numeric column 

    
    // normalize coordinates immediately for internal checks
    int r1 = *rowStart; // copy start row 
    int r2 = *rowEnd; // copy end row 
    int c1 = *colStart; // copy start col 
    int c2 = *colEnd; // copy end col 

    // Ensure r1 <= r2 and c1 <= c2 for length calculation
    if (r1 > r2) { int temp = r1; r1 = r2; r2 = temp; } // swap rows if needed 
    if (c1 > c2) { int temp = c1; c1 = c2; c2 = temp; } // swap cols if needed 

    
    // checking the  bounds
    if (r1 < 0 || r2 >= SIZE || c1 < 0 || c2 >= SIZE) { // bounds check 
        return false; // invalid 
    }

    // ensure it's straight 
    if (r1 != r2 && c1 != c2) { // not straight line 
        return false; // invalid diagonal 
    }

    //  we ensure length is correct
    int actual_length; 
    if (r1 == r2) { // Horizontal: Col diff 
        actual_length = c2 - c1 + 1; // horizontal length 
    } else { 
        actual_length = r2 - r1 + 1; // vertical length 
    }

    if (actual_length != length) { // length check 
        return false; // mismatch 
    }

    // Set our normalized coordinates back to the pointers for updateGrid
    *rowStart = r1; // write normalized start row 
    *rowEnd = r2; // write normalized end row 
    *colStart = c1; // write normalized start col 
    *colEnd = c2; // write normalized end col 

    return true; // valid input 
}

// update grid with ship placement
bool updateGrid(ShipType ship, int rowStart, int colStart, int rowEnd, int colEnd) { // place ship in grid 

    // Check for overlap
    for (int r = rowStart; r <= rowEnd; r++) { // iterate rows 
        for (int c = colStart; c <= colEnd; c++) { // iterate cols 
            if (shipGrid[r][c] != NONE) { // if occupied 
                return false; // overlap 
            }
        }

    }

    // place the ship
    for (int r = rowStart; r <= rowEnd; r++) { // set rows 

        for (int c = colStart; c <= colEnd; c++) { // set cols 
            shipGrid[r][c] = ship; // put ship type 
        }
    }

    return true; // placed 
}

// main game loop
void UpdateState(void) { // core single-player loop 
    bool gameOver = false; // game state flag 

    while (!gameOver) { // loop until game over 
        // player's turn
        printf("\n=== YOUR  TURN  ===\n"); // player's turn header 
        DisplayWorld(); // show boards 

        char input[10]; // shot input buffer 
        int row, col;  // shot coords 
        bool validShot = false; // input validation flag 

        while (!validShot) { // Repeat until the player enters a valid shot 
 
            printf("Enter target (e.g., A4): "); // prompt for shot 

            if (!fgets(input, sizeof(input), stdin)) continue; 
          input[strcspn(input, "\n")] = 0; // trim newline 

     if (strlen(input) < 2) continue;  

      row = toupper(input[0]) - 'A'; // parse row 

      col = atoi(&input[1]); // parse column 

     if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) {  // Check if coordinates are within board bounds 

          printf("Target out of bounds.\n"); // out of bounds message 
                continue; // retry 
            }

            if (shotGrid[row][col] != NOT_TRIED) {   // Check if player already shot at this location 
                printf("You already shot there.\n"); // already shot msg 
                continue; // retry 
            }

            validShot = true; // valid shot accepted 
        }

        // Process player's shot
        bool hit = MakeSinglePlayerShot(row, col); // check computerShipGrid for hit 

        shotGrid[row][col] = hit ? HIT : MISS; // update player's shot grid 

        if (hit) { // ourr hit message 
            printf("HIT!\n"); // print hit 
        } 
            else { // miss 
            printf("MISS!\n"); // print miss 
        }

        // check if player won
        if (SinglePlayerDidWin()) { // if all computer ships hit 
            printf("Congratulations broo ! You sunk all computer ships!\n"); // win msg 
            gameOver = true; // flag finish 
            break; // break loop 
        }

        // computer's turn

        printf("\n=== COMPUTER'S TURN ===\n"); // comp turn header 
        
        int compRow, compCol; // comp shot coords 
        GetSinglePlayerShot(&compRow, &compCol); // choose comp shot 

        // ddetermine if computer hit player's ship
        bool compHit = IS_UNHIT_SHIP(shipGrid[compRow][compCol]); // check if player's ship at location 
        // and WHY?: Uses macro to check if the square holds an unhit ship part

        // we update player's ship grid status for display

        if (compHit) { // if hit 
            shipGrid[compRow][compCol] = HIT_STATUS; // mark hit status 
            printf("Computer hit at %c%d!\n", 'A' + compRow, compCol); // print hit 
        } 
        else { // miss 
            shipGrid[compRow][compCol] = MISS_STATUS; // mark miss status 
            printf("Computer missed at %c%d.\n", 'A' + compRow, compCol); // print miss 
        }

        SinglePlayerResponse(compRow, compCol, compHit); // record comp result 

        // check if computer won
        bool playerLost = true; // assume lost until proven otherwise 
        for (int r = 0; r < SIZE; r++) { // scan player grid 
            for (int c = 0; c < SIZE; c++) { // scan columns 
                // If any square is a ship AND not marked as hit, player has not lost
              if (IS_UNHIT_SHIP(shipGrid[r][c])) { // un-hit ship found 
              playerLost = false; // not lost 
                    break; // break inner loop 
                }
            }

     if (!playerLost) break; // break outer if still alive 

        }

        if (playerLost) { // all ships hit 
            printf("Computer won! All your ships are sunk!\n"); // lose msg 
            gameOver = true; // finish 
            break; // break loop 
        }
    }
    DisplayWorld(); // show final boards 
}

// process player's shot against computer

bool MakeSinglePlayerShot(int row, int col) { // check computer ship grid for hit 

    if (computerShipGrid[row][col] != NONE) { // if there is a ship part 
        computerShipGrid[row][col] = HIT_STATUS; // mark as hit so it isn't hit again 
        return true; // hit 
    }
    computerShipGrid[row][col] = MISS_STATUS; // mark miss for completeness 
    return false; // it misses 
}

// get computer's shot 
void GetSinglePlayerShot(int *row, int *col) { // choose random untried square 
    // why?: Purely random targeting, minimal requirement met.
    int r, c; // local coords 
    while (true) { // loop until untried found 
        r = rand() % SIZE; // random row 
        c = rand() % SIZE; // random col 
        // this Checks computer's tracking grid to ensure the square is NOT_TRIED
        if (computerShotGrid[r][c] == NOT_TRIED) { 
            *row = r; // set output row 
            *col = c; // set output col 
            return; // return coords 
        }
    }
}

// update computer shot grid 
void SinglePlayerResponse(int row, int col, bool hit) { // record result of comp's shot 

    // WHY: because it Updates the computer's tracking grid with the shot result

    computerShotGrid[row][col] = hit ? HIT : MISS; // mark hit/miss in tracking grid 
}

// check if player won..
bool SinglePlayerDidWin(void) { // check if all computer ship parts are hit 
    // WHY: Loops over the enemy's ship grid and checks the player's shot grid for hits

    for (int r = 0; r < SIZE; r++) { // iterate rows 
        for (int c = 0; c < SIZE; c++) { // iterate cols 
            

            if (computerShipGrid[r][c] != NONE && shotGrid[r][c] != HIT) { // un-hit enemy part 
                return false; // not won yet 
            }
        }
    }
    return true; // all enemy parts hit 
}


int start_server(const char *port) { // create, bind and listen, return listen fd or -1 
    struct addrinfo hints, *res, *p; // for getaddrinfo results 
    int listenfd = -1; // listening socket fd 
    int rv; // helper for return values 
    int yes = 1; // option for setsockopt 

    memset(&hints, 0, sizeof hints); // zero hints 
    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */ // allow any IP version 
    hints.ai_socktype = SOCK_STREAM; // TCP socket 
    hints.ai_flags = AI_PASSIVE; /* for bind */ // for local binding 

    if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0) { // resolve address info 
        fprintf(stderr, "getaddrinfo(server): %s\n", gai_strerror(rv)); // print error 
        return -1; // fail 
    }

    for (p = res; p != NULL; p = p->ai_next) { // iterate addresses 
        listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // create socket 
        if (listenfd == -1) continue; // try next if fail 

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); // allow reuse 

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == -1) { // bind socket 
            close(listenfd); // close on fail 
            listenfd = -1; // reset 
            continue; // next address 
        }

        if (listen(listenfd, 1) == -1) { // start listening 
            perror("listen"); // print error 
            close(listenfd); // close 
            listenfd = -1; // reset 
            continue; // next 
        }
        break; // success 
    }

    freeaddrinfo(res); // free addrinfo 
    if (listenfd == -1) { // check bind/listen success 
        fprintf(stderr, "Failed to bind/listen on port %s\n", port); // error 
        return -1; // fail 
    }
    return listenfd; // return listening socket 
}

int accept_client(int listenfd) { // accept a single client connection and return fd 
    struct sockaddr_storage client_addr; // store client addr 
    socklen_t addrlen = sizeof(client_addr); // len var 
    int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen); // accept 
    if (connfd == -1) { // error check 
        perror("accept"); // perror 
        return -1; // fail 
    }
    char host[NI_MAXHOST], serv[NI_MAXSERV]; // buffers for printable address 
    if (getnameinfo((struct sockaddr *)&client_addr, addrlen, host, sizeof(host), serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV) == 0) { // get human-readable addr 
        printf("Accepted connection from %s:%s\n", host, serv); // print peer 
    }
    return connfd; // return connected socket 
}

int start_client(const char *ip, const char *port) { // connect to remote server and return sockfd 
    struct addrinfo hints, *res, *p; // for getaddrinfo 
    int sockfd = -1; // socket fd 
    int rv; // return var 

    memset(&hints, 0, sizeof hints); // zero hints 
    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */ // any IP 
    hints.ai_socktype = SOCK_STREAM; // TCP 

    if ((rv = getaddrinfo(ip, port, &hints, &res)) != 0) { // resolve server addr 
        fprintf(stderr, "getaddrinfo(client): %s\n", gai_strerror(rv)); // error 
        return -1; // fail 
    }

    for (p = res; p != NULL; p = p->ai_next) { // iterate options 
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol); // create socket 
        if (sockfd == -1) continue; // try next 

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { // attempt connect 
            close(sockfd); // close on fail 
            sockfd = -1; // reset 
            continue; // next 
        }
        break; // connected 
    }

    freeaddrinfo(res); // free 
    if (sockfd == -1) { // check success 
        fprintf(stderr, "Failed to connect to %s:%s\n", ip, port); // error 
        return -1; // fail 
    }
    return sockfd; // return connected socket 
}

ssize_t send_line(int fd, const char *s) { // send entire string s, return bytes or -1 
    size_t len = strlen(s); // compute length 
    size_t total = 0; // bytes sent 
    while (total < len) { // loop until all sent 
        ssize_t n = send(fd, s + total, len - total, 0); // send chunk 
        if (n <= 0) return n; // error or closed 
        total += n; // accumulate 
    }
    return (ssize_t)total; // return bytes sent 
}

ssize_t recv_line(int fd, char *buf, size_t maxlen) { // receive until newline or maxlen-1 
    size_t pos = 0; // current pos 

    while (pos + 1 < maxlen) { // leave room for null 

        char ch; // read one char 
        ssize_t n = recv(fd, &ch, 1, 0); // receive 1 byte 

        if (n == 1) { // got a byte 
            buf[pos++] = ch; // store it 
            if (ch == '\n') break; // stop at newline 

        } else if (n == 0) { // peer closed 
            if (pos == 0) return 0; // nothing read 
            break; // return what we have 
        } else { // error 
            if (errno == EINTR) continue; // retry on interrupt 
            return -1; // other errors 
        }
    }
    buf[pos] = '\0'; // null-terminate 
    return (ssize_t)pos; // return bytes stored 
}



void play_twoplayer_server(int connfd) { // server plays first and communicates over connfd 
    char buf[NETBUF]; // buffer for incoming messages 

    if (send_line(connfd, "READY\n") <= 0) { // send READY to client 
        fprintf(stderr, "Failed to send READY to client.\n"); // error 
        close(connfd); // close connection 
        return; // abort 
    }
    if (recv_line(connfd, buf, sizeof(buf)) <= 0) { // wait for client's READY 
        fprintf(stderr, "Failed to receive READY from client.\n"); // error 
        close(connfd); // close 
        return; // abort 
    }
    if (strncmp(buf, "READY", 5) != 0) { // protocol check 
        fprintf(stderr, "Protocol error: expected READY, got: %s\n", buf); // error 
        close(connfd); // close 
        return; // abort 
    }
    printf("Client ready. Server (you) will start.\n"); // info 

    bool gameOver = false; // loop control 
    while (!gameOver) { // main network game loop 
        // server's (local) turn to shoot
        printf("\n=== YOUR TURN (network) ===\n"); // prompt 
        DisplayWorld(); // show boards 

        char input[16]; // input buffer 
        int row = -1, col = -1; // shot coords 
        bool validShot = false; // validation 
        while (!validShot) { // input loop 
            printf("Enter target (e.g., A4): "); // prompt 
            if (!fgets(input, sizeof(input), stdin)) continue; // read input 
            input[strcspn(input, "\n")] = 0; // trim newline 
            if (strlen(input) < 2) continue; // sanity 
            row = toupper(input[0]) - 'A'; // parse row 
            col = atoi(&input[1]); // parse col 
            if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) { // bounds check 
                printf("Target out of bounds.\n"); // msg 
                continue; // retry 
            }
            if (shotGrid[row][col] != NOT_TRIED) { // already shot check 
                printf("You already shot there.\n"); // msg 
                continue; // retry 
            }
            validShot = true; // accepted 
        }

        char out[NETBUF]; // outgoing msg buffer 
        snprintf(out, sizeof(out), "SHOT %d %d\n", row, col); // format shot message 
        if (send_line(connfd, out) <= 0) { // send shot 
            fprintf(stderr, "Send failed.\n"); // error 
            break; // abort 
        }

        if (recv_line(connfd, buf, sizeof(buf)) <= 0) { // wait for result 
            fprintf(stderr, "Connection closed by client.\n"); // error 
            break; // abort 
        }
        if (strncmp(buf, "RESULT ", 7) == 0) { // parse result 
            char *arg = buf + 7; // result argument pointer 
            if (strncmp(arg, "MISS", 4) == 0) { // miss 
                printf("You missed.\n"); // info 
                shotGrid[row][col] = MISS; // record miss 
            } else if (strncmp(arg, "HIT", 3) == 0) { // hit 
                printf("You hit!\n"); // info 
                shotGrid[row][col] = HIT; // record hit 
            } else if (strncmp(arg, "WIN", 3) == 0) { // opponent lost 
                printf("You hit and sank the last ship. You win!\n"); // win msg 
                shotGrid[row][col] = HIT; // record final hit 
                break; // finish 
            } else { // unknown 
                printf("Unknown RESULT: %s\n", arg); 
                break; // abort 
            }
        } else { // protocol error 
            printf("Protocol error: expected RESULT, got: %s\n", buf); 
            break; // abort 
        }

        // Now receive opponent's shot
        printf("\n=== WAITING FOR OPPONENT'S SHOT ===\n"); // waiting msg 
        if (recv_line(connfd, buf, sizeof(buf)) <= 0) { // receive shot 
            fprintf(stderr, "Connection closed by client.\n"); // error 
            break; // abort 
        }
        if (strncmp(buf, "SHOT ", 5) == 0) { // parse shot 
            int r, c; // shot coords 
            if (sscanf(buf + 5, "%d %d", &r, &c) != 2) { // parse ints 
                fprintf(stderr, "Malformed SHOT: %s\n", buf); // error 
                break; // abort 
            }
            printf("Opponent fired at %c%d\n", 'A' + r, c); // show shot 

            bool hit = IS_UNHIT_SHIP(shipGrid[r][c]); // check if hit on our ships 
            if (hit) shipGrid[r][c] = HIT_STATUS; // mark hit 
            else shipGrid[r][c] = MISS_STATUS; // mark miss 

            // check if we lost
            bool playerLost = true; // assume lost until found alive 

            for (int rr = 0; rr < SIZE; rr++) { // scan rows 
                for (int cc = 0; cc < SIZE; cc++) { // scan cols 
                    if (IS_UNHIT_SHIP(shipGrid[rr][cc])) { playerLost = false; break; } // if any part left 
                }
                if (!playerLost) break; // break outer 
            }

            if (playerLost) { // if lost 

                send_line(connfd, "RESULT WIN\n"); // inform opponent they win 
                printf("All your ships have been sunk. You lose.\n"); // msg to local player 
                break; // end loop 
            } else { // still alive 

                if (hit) { // hit case 
                    send_line(connfd, "RESULT HIT\n"); // reply hit 
                    printf("You were hit!\n"); // info 

                } else { // miss case 
                    send_line(connfd, "RESULT MISS\n"); // reply miss 
                    printf("Opponent missed.\n"); // info 
                }
            }
        } else { // unexpected message 
            printf("Unexpected message from client: %s\n", buf); // debug 
            break; // abort 
        }

    } // end while game
    close(connfd); // close connection 
    printf("Network game ended (server).\n"); // info 
}

void play_twoplayer_client(int sockfd) { // client loop: server shoots first 
    char buf[NETBUF]; // incoming buffer 

    if (recv_line(sockfd, buf, sizeof(buf)) <= 0) { // expect READY from server 
        fprintf(stderr, "Failed to receive READY from server.\n"); // error 
        close(sockfd); // close socket 
        return; // abort 
    }
    if (strncmp(buf, "READY", 5) != 0) { // protocol check 
        fprintf(stderr, "Protocol error: expected READY, got: %s\n", buf); // error 
        close(sockfd); // close 
        return; // abort 
    }
    if (send_line(sockfd, "READY\n") <= 0) { // reply READY 
        fprintf(stderr, "Failed to send READY to server.\n"); // error 
        close(sockfd); // close 
        return; // abort 
    }
    printf("Handshake complete. Server will make the first move.\n"); // info 

    bool gameOver = false; // control 
    while (!gameOver) { // main loop 
        // First, receive server's shot
        printf("\n=== WAITING FOR SERVER'S SHOT ===\n"); // waiting msg 
        if (recv_line(sockfd, buf, sizeof(buf)) <= 0) { // receive 
            fprintf(stderr, "Connection closed by server.\n"); // error 
            break; // abort 
        }
        if (strncmp(buf, "SHOT ", 5) == 0) { // parse SHOT 
            int r, c; // shot coords 
            if (sscanf(buf + 5, "%d %d", &r, &c) != 2) { // parse ints 
                fprintf(stderr, "Malformed SHOT: %s\n", buf); // error 
                break; // abort 
            }
            printf("Server fired at %c%d\n", 'A' + r, c); // show shot 

            bool hit = IS_UNHIT_SHIP(shipGrid[r][c]); // check hit 
            if (hit) shipGrid[r][c] = HIT_STATUS; // mark hit 
            else shipGrid[r][c] = MISS_STATUS; // mark miss 

            // check if we lost
            bool playerLost = true; // assume lost 
            for (int rr = 0; rr < SIZE; rr++) { // scan 
                for (int cc = 0; cc < SIZE; cc++) { // scan cols 
                    if (IS_UNHIT_SHIP(shipGrid[rr][cc])) { playerLost = false; break; } // any left 
                }
                if (!playerLost) break; // break outer 
            }

            if (playerLost) { // inform server they win 
                send_line(sockfd, "RESULT WIN\n"); // send win 
                printf("All your ships have been sunk. You lose.\n"); // info 
                break; // finish 
            } else { // reply hit/miss 
                if (hit) { // hit 
                    send_line(sockfd, "RESULT HIT\n"); // send hit 
                    printf("You were hit!\n"); // info 
                } else { // miss 
                    send_line(sockfd, "RESULT MISS\n"); // send miss 
                    printf("Server missed.\n"); // info 
                }
            }

        } else { // unexpected message 
            printf("Unexpected message from server: %s\n", buf); // debug 
            break; // abort 
        }

        // now its the client's turn to shoot
        printf("\n=== YOUR TURN (network) ===\n"); // prompt 
        DisplayWorld(); // show boards 

        char input[16]; // input buffer 
        int row = -1, col = -1; // coords 
        bool validShot = false; // validation 
        while (!validShot) { // loop until valid 

            printf("Enter target (e.g., A4): "); // prompt 

            if (!fgets(input, sizeof(input), stdin)) continue; // read 
            input[strcspn(input, "\n")] = 0; // trim newline 
            if (strlen(input) < 2) continue; // sanity 
            row = toupper(input[0]) - 'A'; // parse row 
            col = atoi(&input[1]); // parse col 
            if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) { // bounds 
                printf("Target out of bounds.\n"); // msg 
                continue; // retry 
            }
            if (shotGrid[row][col] != NOT_TRIED) { // already shot 
                printf("You already shot there.\n"); // msg 
                continue; // retry 
            }
            validShot = true; // accepted 
        }

        char out[NETBUF]; // outgoing buffer 
        snprintf(out, sizeof(out), "SHOT %d %d\n", row, col); // format shot 

        if (send_line(sockfd, out) <= 0) { // send 
            fprintf(stderr, "Send failed.\n"); // error 
            break; // abort 
        }

        if (recv_line(sockfd, buf, sizeof(buf)) <= 0) { // wait for result 
            fprintf(stderr, "Connection closed by server.\n"); // error 
            break; // abort 
        }
        if (strncmp(buf, "RESULT ", 7) == 0) { // parse result 
            char *arg = buf + 7; // pointer 
            if (strncmp(arg, "MISS", 4) == 0) { // miss 
                printf("You missed.\n"); // info 
                shotGrid[row][col] = MISS; // record 
            } else if (strncmp(arg, "HIT", 3) == 0) { // hit 
                printf("You hit!\n"); // info 
                shotGrid[row][col] = HIT; // record 
            } else if (strncmp(arg, "WIN", 3) == 0) { // win 
                printf("You hit and sank the last ship. You win!\n"); // info 
                shotGrid[row][col] = HIT; // record 
                break; // finish 
            } else { // unknown 
                printf("Unknown RESULT: %s\n", arg); // debug 
                break; // abort 
            }
        } else { // protocol error 
            printf("Protocol error: expected RESULT, got: %s\n", buf); // debug 
            break; // abort 
        }
    } // end while
    close(sockfd); // close connection 
    printf("Network game ended (client).\n"); // info 
}

