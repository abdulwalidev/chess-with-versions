#include <iostream>
#include <string>
#include <cctype>
#include <cmath>
#include <sstream>
#include <windows.h>
#include <conio.h>

using namespace std;

bool possible[8][8] = { false };

const int BOARD_SIZE = 8;
char board[BOARD_SIZE][BOARD_SIZE];
int cursorRow = 7, cursorCol = 0;
bool pieceSelected = false;
int selectedRow, selectedCol;

bool whiteKingMoved = false;     // CASTLING
bool blackKingMoved = false;     // CASTLING
bool whiteRookLeftMoved = false; // CASTLING
bool whiteRookRightMoved = false;
bool blackRookLeftMoved = false;
bool blackRookRightMoved = false;

void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

void initializeBoard() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[1][i] = 'p';
        board[6][i] = 'P';
    }
    const char white_pieces[] = { 'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R' };
    const char black_pieces[] = { 'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r' };
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[0][i] = black_pieces[i];
        board[7][i] = white_pieces[i];
    }
    for (int i = 2; i < 6; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board[i][j] = '-';
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void drawCursorBoard() {
    const int padding = 10;
    cout << string(padding + 7, ' ') << "  a b c d e f g h\n";
    for (int i = BOARD_SIZE - 1; i >= 0; --i) {
        cout << string(padding + 7, ' ') << i + 1 << " ";
        for (int j = 0; j < BOARD_SIZE; ++j) {
            char piece = board[i][j];
            bool isCursor = (i == cursorRow && j == cursorCol);
            bool isSelected = pieceSelected && i == selectedRow && j == selectedCol;
            bool isPossible = possible[i][j];

            if (piece == '-') setColor(8);
            else if (isupper(piece)) setColor(15);
            else setColor(11);

            if (isSelected) setColor(79);
            else if (isCursor) setColor(160);
            else if (isPossible) setColor(10);

            cout << (piece == '-' ? "_ " : string(1, piece) + " ");
            setColor(7);
        }
        cout << i + 1 << "\n";
    }
    cout << string(padding + 7, ' ') << "  a b c d e f g h\n";
}

bool isPathClear(int row, int col1, int col2) {
    if (col1 > col2) swap(col1, col2);
    for (int c = col1 + 1; c < col2; ++c)
        if (board[row][c] != '-') return false;
    return true;
}

bool isCastlingMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (player == 'w' && srcRow == 7 && srcCol == 4 && destRow == 7 && (destCol == 6 || destCol == 2)) {
        if (whiteKingMoved) return false;
        if (destCol == 6 && !whiteRookRightMoved && board[7][7] == 'R' && isPathClear(7, 4, 7)) return true;
        if (destCol == 2 && !whiteRookLeftMoved && board[7][0] == 'R' && isPathClear(7, 0, 4)) return true;
    }
    if (player == 'b' && srcRow == 0 && srcCol == 4 && destRow == 0 && (destCol == 6 || destCol == 2)) {
        if (blackKingMoved) return false;
        if (destCol == 6 && !blackRookRightMoved && board[0][7] == 'r' && isPathClear(0, 4, 7)) return true;
        if (destCol == 2 && !blackRookLeftMoved && board[0][0] == 'r' && isPathClear(0, 0, 4)) return true;
    }
    return false;
}

void performCastling(int row, int destCol) {
    if (destCol == 6) {
        board[row][5] = board[row][7];
        board[row][7] = '-';
    }
    else if (destCol == 2) {
        board[row][3] = board[row][0];
        board[row][0] = '-';
    }
}

bool isValidPawnMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    int direction = (player == 'w') ? -1 : 1;
    bool isOpponentPiece = (player == 'w') ? islower(board[destRow][destCol]) : isupper(board[destRow][destCol]);

    if (srcCol == destCol && board[destRow][destCol] == '-') {
        return (destRow - srcRow == direction) ||
            (srcRow == (player == 'w' ? 6 : 1) && destRow - srcRow == 2 * direction && board[srcRow + direction][srcCol] == '-');
    }

    if (abs(srcCol - destCol) == 1 && destRow - srcRow == direction && isOpponentPiece) {
        return true;
    }

    return false;
}

bool isValidRookMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (srcRow != destRow && srcCol != destCol) return false;
    int rowStep = (srcRow < destRow) ? 1 : (srcRow > destRow ? -1 : 0);
    int colStep = (srcCol < destCol) ? 1 : (srcCol > destCol ? -1 : 0);
    for (int i = srcRow + rowStep, j = srcCol + colStep; i != destRow || j != destCol; i += rowStep, j += colStep) {
        if (board[i][j] != '-') return false;
    }
    char target = board[destRow][destCol];
    return target == '-' || (player == 'w' ? islower(target) : isupper(target));
}

bool isValidBishopMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (abs(srcRow - destRow) != abs(srcCol - destCol)) return false;
    int rowStep = (srcRow < destRow) ? 1 : -1;
    int colStep = (srcCol < destCol) ? 1 : -1;
    for (int i = srcRow + rowStep, j = srcCol + colStep; i != destRow; i += rowStep, j += colStep)
        if (board[i][j] != '-') return false;
    char target = board[destRow][destCol];
    return target == '-' || (player == 'w' ? islower(target) : isupper(target));
}

bool isValidKnightMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    int dr = abs(srcRow - destRow), dc = abs(srcCol - destCol);
    char target = board[destRow][destCol];
    return (dr == 2 && dc == 1 || dr == 1 && dc == 2) &&
        (target == '-' || (player == 'w' ? islower(target) : isupper(target)));
}

bool isValidQueenMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    return isValidRookMove(srcRow, srcCol, destRow, destCol, player) ||
        isValidBishopMove(srcRow, srcCol, destRow, destCol, player);
}

bool isValidKingMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (isCastlingMove(srcRow, srcCol, destRow, destCol, player)) return true;
    int dr = abs(srcRow - destRow), dc = abs(srcCol - destCol);
    char target = board[destRow][destCol];
    return dr <= 1 && dc <= 1 &&
        (target == '-' || (player == 'w' ? islower(target) : isupper(target)));
}

bool isValidMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
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

bool isKingInCheck(char player) {
    char king = (player == 'w') ? 'K' : 'k';
    int kr = -1, kc = -1;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            if (board[i][j] == king) kr = i, kc = j;
    char opponent = (player == 'w') ? 'b' : 'w';
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            if ((opponent == 'w' && isupper(board[i][j])) ||
                (opponent == 'b' && islower(board[i][j])))
                if (isValidMove(i, j, kr, kc, opponent)) return true;
    return false;
}

bool hasAnyLegalMove(char player) {
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j) {
            char piece = board[i][j];
            if ((player == 'w' && !isupper(piece)) || (player == 'b' && !islower(piece))) continue;
            for (int ni = 0; ni < 8; ++ni)
                for (int nj = 0; nj < 8; ++nj)
                    if (isValidMove(i, j, ni, nj, player)) {
                        char backup = board[ni][nj];
                        board[ni][nj] = board[i][j];
                        board[i][j] = '-';
                        bool stillCheck = isKingInCheck(player);
                        board[i][j] = board[ni][nj];
                        board[ni][nj] = backup;
                        if (!stillCheck) return true;
                    }
        }
    return false;
}

bool isCheckmate(char player) {
    return isKingInCheck(player) && !hasAnyLegalMove(player);
}

void possibleMoves(int row, int col, char player) {
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            possible[i][j] = isValidMove(row, col, i, j, player);
}

void promotePawn(int row, int col, char player) {
    char newPiece = (player == 'w') ? 'Q' : 'q';
    board[row][col] = newPiece;
}

void playGame() {
    char currentPlayer = 'w';
    while (true) {
        clearScreen();
        drawCursorBoard();
        cout << endl << "\t        -> " << (currentPlayer == 'w' ? "UpperCase's" : "LowerCase's") << " Turn\n";

        if (isCheckmate(currentPlayer)) {
            cout << "\n\t CHECKMATE! " << (currentPlayer == 'w' ? "LowerCase" : "UpperCase") << " wins!\n";
            cout << "\t Press any key to exit...";

            break;
        }
        else if (isKingInCheck(currentPlayer)) {
            cout << "\n\t " << (currentPlayer == 'w' ? "UpperCase" : "LowerCase") << " is in CHECK!";
        }

        int ch = _getch();
        if (ch == 224) {
            int arrow = _getch();
            if (arrow == 72 && cursorRow < 7) cursorRow++;
            else if (arrow == 80 && cursorRow > 0) cursorRow--;
            else if (arrow == 75 && cursorCol > 0) cursorCol--;
            else if (arrow == 77 && cursorCol < 7) cursorCol++;
        }
        else if (ch == 13) {
            if (!pieceSelected) {
                if ((currentPlayer == 'w' && isupper(board[cursorRow][cursorCol])) ||
                    (currentPlayer == 'b' && islower(board[cursorRow][cursorCol]))) {
                    selectedRow = cursorRow;
                    selectedCol = cursorCol;
                    pieceSelected = true;
                    possibleMoves(selectedRow, selectedCol, currentPlayer);
                }
            }
            else {
                if (isValidMove(selectedRow, selectedCol, cursorRow, cursorCol, currentPlayer)) {
                    char movedPiece = board[selectedRow][selectedCol];
                    char captured = board[cursorRow][cursorCol];
                    bool castling = isCastlingMove(selectedRow, selectedCol, cursorRow, cursorCol, currentPlayer);

                    board[cursorRow][cursorCol] = movedPiece;
                    board[selectedRow][selectedCol] = '-';
                    if (castling) performCastling(cursorRow, cursorCol);

                    if (isKingInCheck(currentPlayer)) {
                        board[selectedRow][selectedCol] = movedPiece;
                        board[cursorRow][cursorCol] = captured;
                        if (castling) performCastling(cursorRow, cursorCol); // Undo castling
                        cout << "\nInvalid move! King would be in check.\n";

                    }
                    else {
                        // Update castling flags
                        if (movedPiece == 'K') whiteKingMoved = true;
                        if (movedPiece == 'k') blackKingMoved = true;
                        if (movedPiece == 'R' && selectedCol == 0 && selectedRow == 7) whiteRookLeftMoved = true;
                        if (movedPiece == 'R' && selectedCol == 7 && selectedRow == 7) whiteRookRightMoved = true;
                        if (movedPiece == 'r' && selectedCol == 0 && selectedRow == 0) blackRookLeftMoved = true;
                        if (movedPiece == 'r' && selectedCol == 7 && selectedRow == 0) blackRookRightMoved = true;

                        // Promotion
                        if ((movedPiece == 'P' && cursorRow == 0) || (movedPiece == 'p' && cursorRow == 7)) {
                            promotePawn(cursorRow, cursorCol, currentPlayer);
                        }

                        currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
                    }
                }
                else {
                    cout << "Invalid move!\n";

                }
                pieceSelected = false;
                for (int i = 0; i < 8; ++i)
                    for (int j = 0; j < 8; ++j)
                        possible[i][j] = false;
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
