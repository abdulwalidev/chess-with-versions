#include <iostream>
#include <string>
#include <cctype>
#include <cmath>
#include <sstream>
#include <windows.h>

#include <conio.h> // Windows only

using namespace std;

const int BOARD_SIZE = 8;
char board[BOARD_SIZE][BOARD_SIZE];
int cursorRow = 7, cursorCol = 0; bool pieceSelected = false; int selectedRow, selectedCol;

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

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void drawCursorBoard() {
    const int padding = 10; // Adjust to center if needed
    cout << string(padding, ' ') << "  a b c d e f g h\n";
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        cout << string(padding, ' ') << i + 1 << " ";
        for (int j = 0; j < BOARD_SIZE; ++j) {
            bool isCursor = (i == cursorRow && j == cursorCol);
            bool isSelected = pieceSelected && i == selectedRow && j == selectedCol;
            char piece = board[i][j];

            if (isCursor || isSelected) setColor(112); // Highlight cursor/selection

            if (piece == '-') {
                setColor(isCursor || isSelected ? 112 : 7); // Default color
                cout << "_ ";
            }
            else if (isupper(piece)) {
                setColor(isCursor || isSelected ? 112 : 14); // White pieces: yellow
                cout << piece << " ";
            }
            else {
                setColor(isCursor || isSelected ? 112 : 9); // Black pieces: blue
                cout << piece << " ";
            }

            setColor(7); // Reset color after each piece
        }
        cout << i + 1 << "\n";
    }
    cout << string(padding, ' ') << "  a b c d e f g h\n";
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
    int colStep = (srcCol == destCol) ? 0 : ((srcCol < destCol) ? 1 : -1);

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
    while (true) {
        clearScreen();
        drawCursorBoard();
        cout << (currentPlayer == 'w' ? "White" : "Black") << "'s turn. Use arrows to move, Enter to select, q to quit.\n";
        int ch = _getch();
        if (ch == 224) { // Arrow key prefix
            int arrow = _getch();
            switch (arrow) {
            case 72: if (cursorRow < BOARD_SIZE - 1) cursorRow++; break; // up
            case 80: if (cursorRow > 0) cursorRow--; break; // down
            case 75: if (cursorCol > 0) cursorCol--; break; // left
            case 77: if (cursorCol < BOARD_SIZE - 1) cursorCol++; break; // right
            }
        }
        else if (ch == 13) { // Enter key
            if (!pieceSelected) {
                if ((currentPlayer == 'w' && isupper(board[cursorRow][cursorCol])) || (currentPlayer == 'b' && islower(board[cursorRow][cursorCol]))) {
                    selectedRow = cursorRow;
                    selectedCol = cursorCol;
                    pieceSelected = true;
                }
            }
            else {
                if (isValidMove(selectedRow, selectedCol, cursorRow, cursorCol, currentPlayer)) {
                    board[cursorRow][cursorCol] = board[selectedRow][selectedCol];
                    board[selectedRow][selectedCol] = '-';
                    currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
                }
                else {
                    cout << "Invalid move!\n";
                    _getch(); // Pause to show error
                }
                pieceSelected = false;
            }
        }
        else if (ch == 'q' || ch == 'Q') {
            break;
        }
    }
}

int main() {
    initializeBoard();
    playGame();
    return 0;
}
