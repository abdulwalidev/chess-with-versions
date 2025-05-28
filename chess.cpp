#include <iostream>
#include <string>
#include <cctype>
#include <cmath>
#include <sstream>

using namespace std;

const int BOARD_SIZE = 8;
char board[BOARD_SIZE][BOARD_SIZE];

// Clear the screen
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

// Initialize the chessboard
void initializeBoard() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[1][i] = 'p'; // Black pawns
        board[6][i] = 'P'; // White pawns
    }
    const char white_pieces[] = { 'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R' };
    const char black_pieces[] = { 'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r' };
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[0][i] = black_pieces[i];
        board[7][i] = white_pieces[i];
    }
    for (int i = 2; i < 6; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = '-';
        }
    }
}

// Print the chessboard
void printBoard() {
    cout << "  a b c d e f g h\n";
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        cout << i + 1 << " ";
        for (int j = 0; j < BOARD_SIZE; ++j) {
            cout << board[i][j] << " ";
        }
        cout << i + 1 << "\n";
    }
    cout << "  a b c d e f g h\n";
}

// Convert chess notation to board coordinates
bool parsePosition(const string& position, int& row, int& col) {
    if (position.length() != 2) return false;
    col = position[0] - 'a';
    row = position[1] - '1';
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

// Validate pawn moves
bool isValidPawnMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    int direction = (player == 'w') ? -1 : 1; // White pawns move up, black down
    bool isOpponentPiece = (player == 'w') ? islower(board[destRow][destCol]) : isupper(board[destRow][destCol]);

    // Regular move
    if (srcCol == destCol && board[destRow][destCol] == '-') {
        return (destRow - srcRow == direction) ||
            (srcRow == (player == 'w' ? 6 : 1) && destRow - srcRow == 2 * direction && board[srcRow + direction][srcCol] == '-');
    }

    // Capture move
    if (abs(srcCol - destCol) == 1 && destRow - srcRow == direction &&
        board[destRow][destCol] != '-' && isOpponentPiece) {
        return true;
    }

    return false;
}

// Validate rook moves
bool isValidRookMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (srcRow != destRow && srcCol != destCol) return false;

    int rowStep = (srcRow == destRow) ? 0 : ((srcRow < destRow) ? 1 : -1);
    int colStep = (srcCol == destRow) ? 0 : ((srcCol < destCol) ? 1 : -1);

    int currentRow = srcRow + rowStep;
    int currentCol = srcCol + colStep;

    // Check path is clear
    while (currentRow != destRow || currentCol != destCol) {
        if (board[currentRow][currentCol] != '-') return false;
        currentRow += (currentRow != destRow ? rowStep : 0);
        currentCol += (currentCol != destCol ? colStep : 0);
    }

    return board[destRow][destCol] == '-' ||
        (player == 'w' ? islower(board[destRow][destCol]) : isupper(board[destRow][destCol]));
}

// Validate bishop moves
bool isValidBishopMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (abs(srcRow - destRow) != abs(srcCol - destCol)) return false;

    int rowStep = (srcRow < destRow) ? 1 : -1;
    int colStep = (srcCol < destCol) ? 1 : -1;

    int currentRow = srcRow + rowStep;
    int currentCol = srcCol + colStep;

    // Check path is clear
    while (currentRow != destRow) {
        if (board[currentRow][currentCol] != '-') return false;
        currentRow += rowStep;
        currentCol += colStep;
    }

    return board[destRow][destCol] == '-' ||
        (player == 'w' ? islower(board[destRow][destCol]) : isupper(board[destRow][destCol]));
}

// Validate knight moves
bool isValidKnightMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    int rowDiff = abs(srcRow - destRow);
    int colDiff = abs(srcCol - destCol);
    char destPiece = board[destRow][destCol];

    return (rowDiff == 2 && colDiff == 1 || rowDiff == 1 && colDiff == 2) &&
        (destPiece == '-' || (player == 'w' ? islower(destPiece) : isupper(destPiece)));
}

// Validate queen moves
bool isValidQueenMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    return isValidRookMove(srcRow, srcCol, destRow, destCol, player) ||
        isValidBishopMove(srcRow, srcCol, destRow, destCol, player);
}

// Validate king moves
bool isValidKingMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    int rowDiff = abs(srcRow - destRow);
    int colDiff = abs(srcCol - destCol);
    char destPiece = board[destRow][destCol];

    return rowDiff <= 1 && colDiff <= 1 &&
        (destPiece == '-' || (player == 'w' ? islower(destPiece) : isupper(destPiece)));
}

// Validate moves for a piece
bool isValidMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    // First, check if source and destination are different
    if (srcRow == destRow && srcCol == destCol) return false;

    char piece = board[srcRow][srcCol];
    if (player == 'w' && !isupper(piece)) return false;
    if (player == 'b' && !islower(piece)) return false;
    piece = tolower(piece);

    switch (piece) {
    case 'p': return isValidPawnMove(srcRow, srcCol, destRow, destCol, player);
    case 'r': return isValidRookMove(srcRow, srcCol, destRow, destCol, player);
    case 'n': return isValidKnightMove(srcRow, srcCol, destRow, destCol, player);
    case 'b': return isValidBishopMove(srcRow, srcCol, destRow, destCol, player);
    case 'q': return isValidQueenMove(srcRow, srcCol, destRow, destCol, player);
    case 'k': return isValidKingMove(srcRow, srcCol, destRow, destCol, player);
    default: return false;
    }
}

// Execute a move
bool makeMove(const string& move, char player) {
    // Trim whitespace
    stringstream ss(move);
    string src, dest;
    ss >> src >> dest;

    int srcRow, srcCol, destRow, destCol;
    if (!parsePosition(src, srcRow, srcCol) || !parsePosition(dest, destRow, destCol)) {
        cout << "Invalid move format!\n";
        return false;
    }

    if (isValidMove(srcRow, srcCol, destRow, destCol, player)) {
        board[destRow][destCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = '-';
        return true;
    }
    cout << "Invalid move!\n";
    return false;
}

// Main game loop
void playGame() {
    char currentPlayer = 'w';
    string move;

    while (true) {
        clearScreen();
        printBoard();
        cout << (currentPlayer == 'w' ? "White" : "Black") << "'s turn. Enter move (e.g., e2 e4): ";
        getline(cin, move);

        if (move == "quit") {
            cout << "Game ended!\n";
            break;
        }

        if (makeMove(move, currentPlayer)) {
            currentPlayer = (currentPlayer == 'w' ? 'b' : 'w');
        }
    }
}

int main() {
    initializeBoard();
    playGame();
    return 0;
}
