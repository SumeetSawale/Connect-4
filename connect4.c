#include <assert.h>
#include <stdint.h>
#include <stdio.h>

/* Implement connect 4 on a 5 (columns) x 4 (rows) board. */
enum {
    EMPTY = 0,
    R,
    B,
};

typedef char board_t[4][5];
typedef char player_t;

// Done has_won
int has_won(board_t board, player_t player)
{
    // getting character of player
    char c = (char) player;

    // Initializing variables
    int i = 0;
    int j = 0;

    // Checking Rows
    for (i = 0; i < 4; i++) // Parsing rows
    {
        // If any one of the central 3 does not match, move to next row
        if ((board[i][1] != c) || (board[i][2] != c) || (board[i][3] != c)) {continue;}

        // If the first or last matches found four consecutive
        if ((board[i][0] == c) || (board[i][4] == c)) {
            return 1;
            }
    }

    // Checking Columns
    for (j = 0; j < 5; j++) // Parsing the columns
    {
        // If any one element of the column does not match, move on to next column
        if ((board[0][j] != c) || (board[1][j] != c) || (board[2][j] != c) || (board[3][j] != c)) {continue;}

        // All elements matched
        return 1;
    }

    // Checking primary diagonals
    for (j = 0; j < 2; j++) // Only 2 primary diagonals
    {
        int found = 0;
        for (i = 0; i < 4; i++)
        {
            // if any one element of diagonal does not match, move to next diagonal
            if (board[i][i+j] != c) {break;}
            found++;
        }
        if (found == 4) {return 1;}
    }

    // Checking secondary diagonals
    for (i = 0; i < 2; i++) // Only 2 secondary diagonals
    {
        int found = 0;
        for (j = 0; j < 4; j++)
        {
            // If any one element of diagonal does not match, move to next diagonal
            if (board[j][4-j-i] != c){break;}
            found++;
        }
        if (found == 4) {return 1;}
    }

    // Not Won
    return 0;
}

// Done is_full
int is_full(board_t board)
{
    // Parsing through all columns
    for (int j = 0; j < 5; j++)
    {
        // Checking top element
        if (board[0][j] == '.') {return 0;}
    }
    return 1;
}

typedef struct {
    int col;
    int score;
} move_t;

// Changing the player
player_t other_player(player_t player)
{
    switch (player) {
    case 'R': return 'B';
    case 'B': return 'R';
    default: assert(0);
    }
}

// Max possible position
#define MAX_ORD (24300000)

// Array for memoization
uint8_t computed_moves[MAX_ORD+1];

// Encoded move in one byte
uint8_t encode_move(move_t m)
{
    uint8_t b = 0;

    assert(0 <= m.col && m.col <= 4);
    // First 3 bits for column
    b |= m.col;
 
    switch (m.score) {
    case -1: b |= 1 << 5; break; // 6th bit for loss
    case 0: b |= 1 << 4; break;  // 5th bit for draw
    case 1: b |= 1 << 3; break;  // 4th bit for win
    }

    return b;
}

// Decoded the move
move_t decode_move(uint8_t b)
{
    move_t m;
    m.col = (b & 0x7);
    if (b & 010) m.score = 1;
    if (b & 020) m.score = 0;
    if (b & 040) m.score = -1;
    return m;
}

// Calculating the order of board
int ord(board_t board)
{
    int p;
    int p2 = 1;
    int i;
    int d;
    int o = 0;

    for (int col = 0; col < 5; ++col) {
        i = 0;
        p = 1;
        for (int row = 3; row >= 0 ; row--) {
            switch (board[row][col]) {
            case 'R': d = 1; break;
            case 'B': d = 2; break;
            case '.': d = 0; break;
            }
            i += d * p;
            p *= 2;
        }
        o += i*(p2);
        p2 *= 30;
    }

    return o;
}

move_t best_move(board_t board, player_t player)
{
    // Asserting board is empty and not won
    assert(!is_full(board));
    assert(!has_won(board, player));
    assert(!has_won(board, other_player(player)));

    // Initializing variables
    move_t response;
    move_t candidate;
    int no_candidate = 1;

    // If move is already computed return that
    int o = ord(board);
    if (computed_moves[o]) 
    {
        return decode_move(computed_moves[o]);
    }

    // Finding the bottom-most empty spot for every col
    int row[5];
    for (int col = 0; col < 5; col++)
    {
        int r = 0;
        while ((board[r][col] == '.') && (r < 4)) {r++;}
        r--;
        row[col] = r;
    }

    for (int col = 0; col < 5; col++)
    {
        if (board[0][col] != '.') {continue;} // If column is full move to next column
        
        board[row[col]][col] = player;

        if (has_won(board, player))
        {
            board[row[col]][col] = '.';
            computed_moves[o] = encode_move(candidate = (move_t) {.col = col, .score = 1});
            return candidate;
        }

        // Resetting the board
        board[row[col]][col] = '.';
    }

    for (int col = 0; col < 5; col++)
    {
        if (board[0][col] != '.') {continue;} // If column is full
        
        board[row[col]][col] = player;

        if (is_full(board))
        {
            board[row[col]][col] = '.';
            computed_moves[o] = encode_move(candidate = (move_t) {.col = col, .score = 0});
            return candidate;
        }

        response = best_move(board, other_player(player));
        board[row[col]][col] = '.';

        if (response.score == -1)
        {
            computed_moves[o] = encode_move(candidate = (move_t) {.col = col, .score = 1});
            return candidate;
        }

        else if (response.score == 0)
        {
            candidate = (move_t) {.col = col, .score = 0};
            no_candidate = 0;
        }

        else  // response.score = 1
        {
            if (no_candidate)
            {
                candidate = (move_t) {.col = col, .score = -1};
                no_candidate = 0;
            }
        }
    }

    computed_moves[o] = encode_move(candidate);
    return candidate;
}

// Done print_board
void print_board(board_t board)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

int main()
{
    /* Your game play logic. */
    /* The user should have the option to select red or blue player. */

    // Initializing board
    board_t board;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            board[i][j] = '.';
        }
    }

    // Asking for user input
    player_t player;
    printf("Enter \'R\' for red (P1) or \'B\' for blue (P2) : ");
    scanf("%c", &player);

    // while (((char) player != 'R') && ((char) player != 'B'))
    // {
    //     printf("Invalid choice\n");
    //     printf("Enter \'R\' for red (P1) or \'B\' for blue (P2) : ");
    //     scanf("%c\n", &player);
    // }

    // Initializing variables
    int move, col;
    move_t response;
    player_t current = 'R';

    while (1)
    {
        // Player's turn
        if (current == player)
        {
            // Getting valid column
            printf("Enter No. of column: ");
            scanf("%i", &col);
            while (board[0][col] != '.')
            {
                printf("Invalid Move!\n");
                printf("Enter No. of column: ");
                scanf("%i", &col);
            }
            
            // Finding the value of row
            int row = 0;
            while ((board[row][col] == '.') && (row < 4)) {row++;}
            row--;

            board[row][col] = current;
            print_board(board);
        }

        // Computer's turn
        else
        {
            response = best_move(board, current);

            // Finding row
            int r = 0;
            while ((board[r][response.col] == '.') && (r < 4)) {r++;}
            r--;

            board[r][response.col] = current;
            print_board(board);
        }
    
        // If the current player has won
        if (has_won(board, current))
        {
            if (current == player) {printf("Congratulations you won!");}
            else {printf("Well played, better luck next time");}
            break;
        }
    
        // If board is full without anyone winning
        else if (is_full(board))
        {
            printf("Draw.\n");
            break;
        }
    
        // Giving the chance to other player
        current = other_player(current);
        printf("===========\n");
    }
    return 0;
}
