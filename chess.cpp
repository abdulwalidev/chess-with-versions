#include <SFML/Graphics.hpp>
#include <iostream>
#include <cctype>
#include <cmath>

const int BOARD_SIZE = 8;
const int TILE_SIZE = 100;
const int WINDOW_SIZE = TILE_SIZE * BOARD_SIZE;

// Chess game state variables
char board[BOARD_SIZE][BOARD_SIZE];
bool whiteKingMoved = false;
bool blackKingMoved = false;
bool whiteRookLeftMoved = false;
bool whiteRookRightMoved = false;
bool blackRookLeftMoved = false;
bool blackRookRightMoved = false;

struct ChessPiece {
    char type;
    bool isWhite;
    float posX, posY; // Using individual floats instead of sf::Vector2f

    void draw(sf::RenderWindow& window, int row, int col) {
        sf::Color pieceColor = isWhite ? sf::Color(255, 215, 0) : sf::Color(0, 128, 0); // Golden vs Green

        // Calculate center position of tile for perfect alignment
        float centerX = col * TILE_SIZE + TILE_SIZE / 2.0f;
        float centerY = row * TILE_SIZE + TILE_SIZE / 2.0f;
        float pieceSize = TILE_SIZE * 0.7f; // 70% of tile size for better fit

        switch (std::tolower(type)) {
        case 'p': { // Pawn - Circle
            sf::CircleShape pawn(pieceSize / 2);
            pawn.setFillColor(pieceColor);
            pawn.setOutlineThickness(2);
            pawn.setOutlineColor(sf::Color::Black);
            pawn.setOrigin(pieceSize / 2, pieceSize / 2); // Center origin
            pawn.setPosition(centerX, centerY);
            window.draw(pawn);
            break;
        }
        case 'r': { // Rook - Rectangle
            sf::RectangleShape rook;
            rook.setSize(sf::Vector2f(pieceSize, pieceSize)); // FIX for Line 45

            rook.setFillColor(pieceColor);
            rook.setOutlineThickness(2);
            rook.setOutlineColor(sf::Color::Black);
            rook.setOrigin(pieceSize / 2, pieceSize / 2); // Center origin
            rook.setPosition(centerX, centerY);
            window.draw(rook);
            break;
        }
        case 'n': { // Knight - Triangle
            sf::CircleShape knight(pieceSize / 2, 3);
            knight.setFillColor(pieceColor);
            knight.setOutlineThickness(2);
            knight.setOutlineColor(sf::Color::Black);
            knight.setOrigin(pieceSize / 2, pieceSize / 2); // Center origin
            knight.setPosition(centerX, centerY);
            window.draw(knight);
            break;
        }
        case 'b': { // Bishop - Diamond
            sf::CircleShape bishop(pieceSize / 2, 4);
            bishop.setFillColor(pieceColor);
            bishop.setOutlineThickness(2);
            bishop.setOutlineColor(sf::Color::Black);
            bishop.setOrigin(pieceSize / 2, pieceSize / 2); // Center origin
            bishop.setPosition(centerX, centerY);
            bishop.setRotation(45);
            window.draw(bishop);
            break;
        }
        case 'q': { // Queen - Octagon
            sf::CircleShape queen(pieceSize / 2, 8);
            queen.setFillColor(pieceColor);
            queen.setOutlineThickness(2);
            queen.setOutlineColor(sf::Color::Black);
            queen.setOrigin(pieceSize / 2, pieceSize / 2); // Center origin
            queen.setPosition(centerX, centerY);
            window.draw(queen);
            break;
        }
        case 'k': { // King - Hexagon
            sf::CircleShape king(pieceSize / 2, 6);
            king.setFillColor(pieceColor);
            king.setOutlineThickness(3);
            king.setOutlineColor(sf::Color::Red);
            king.setOrigin(pieceSize / 2, pieceSize / 2); // Center origin
            king.setPosition(centerX, centerY);
            window.draw(king);
            break;
        }
        }
    }
};

char currentPlayer = 'w';
int selectedRow = -1, selectedCol = -1;
bool pieceSelected = false;

void initializeBoard() {
    // Initialize pawns
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[1][i] = 'p'; // Black pawns
        board[6][i] = 'P'; // White pawns
    }

    // Initialize other pieces
    const char white_pieces[] = { 'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R' };
    const char black_pieces[] = { 'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r' };

    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[0][i] = black_pieces[i]; // Black back row
        board[7][i] = white_pieces[i]; // White back row
    }

    // Initialize empty squares
    for (int i = 2; i < 6; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board[i][j] = '-';
        }
    }
}

void drawBoard(sf::RenderWindow& window) {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            sf::RectangleShape tile;
            
            tile.setSize(sf::Vector2f(TILE_SIZE, TILE_SIZE)); // FIX for Line 131
            tile.setPosition(col * TILE_SIZE, row * TILE_SIZE);

            // Highlight selected square
            if (pieceSelected && selectedRow == row && selectedCol == col) {
                tile.setFillColor(sf::Color::Blue);
            }
            else {
                tile.setFillColor((row + col) % 2 == 0 ? sf::Color(240, 217, 181) : sf::Color(181, 136, 99));
            }
            window.draw(tile);
        }
    }
}

void drawPieces(sf::RenderWindow& window) {
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            if (board[row][col] != '-') {
                ChessPiece piece;
                piece.type = board[row][col];
                piece.isWhite = std::isupper(board[row][col]);
                piece.posX = col * TILE_SIZE + TILE_SIZE / 2.0f;
                piece.posY = row * TILE_SIZE + TILE_SIZE / 2.0f;
                piece.draw(window, row, col);
            }
        }
    }
}

bool isPathClear(int row, int col1, int col2) {
    if (col1 > col2) {
        int temp = col1;
        col1 = col2;
        col2 = temp;
    }
    for (int c = col1 + 1; c < col2; ++c) {
        if (board[row][c] != '-') return false;
    }
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
    bool isOpponentPiece = (player == 'w') ? std::islower(board[destRow][destCol]) : std::isupper(board[destRow][destCol]);

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
    return target == '-' || (player == 'w' ? std::islower(target) : std::isupper(target));
}

bool isValidBishopMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (abs(srcRow - destRow) != abs(srcCol - destCol)) return false;

    int rowStep = (srcRow < destRow) ? 1 : -1;
    int colStep = (srcCol < destCol) ? 1 : -1;

    for (int i = srcRow + rowStep, j = srcCol + colStep; i != destRow; i += rowStep, j += colStep) {
        if (board[i][j] != '-') return false;
    }

    char target = board[destRow][destCol];
    return target == '-' || (player == 'w' ? std::islower(target) : std::isupper(target));
}

bool isValidKnightMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    int dr = abs(srcRow - destRow), dc = abs(srcCol - destCol);
    char target = board[destRow][destCol];
    return (dr == 2 && dc == 1 || dr == 1 && dc == 2) &&
        (target == '-' || (player == 'w' ? std::islower(target) : std::isupper(target)));
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
        (target == '-' || (player == 'w' ? std::islower(target) : std::isupper(target)));
}

bool isValidMove(int srcRow, int srcCol, int destRow, int destCol, char player) {
    if (srcRow == destRow && srcCol == destCol) return false;

    char piece = board[srcRow][srcCol];
    if (player == 'w' && !std::isupper(piece)) return false;
    if (player == 'b' && !std::islower(piece)) return false;

    piece = std::tolower(piece);
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

    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] == king) {
                kr = i;
                kc = j;
            }
        }
    }

    char opponent = (player == 'w') ? 'b' : 'w';
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if ((opponent == 'w' && std::isupper(board[i][j])) ||
                (opponent == 'b' && std::islower(board[i][j]))) {
                if (isValidMove(i, j, kr, kc, opponent)) return true;
            }
        }
    }
    return false;
}

void promotePawn(int row, int col, char player) {
    char newPiece = (player == 'w') ? 'Q' : 'q';
    board[row][col] = newPiece;
}

void handleMove(int targetRow, int targetCol) {
    if (!pieceSelected) {
        char piece = board[targetRow][targetCol];
        if ((currentPlayer == 'w' && std::isupper(piece)) ||
            (currentPlayer == 'b' && std::islower(piece))) {
            selectedRow = targetRow;
            selectedCol = targetCol;
            pieceSelected = true;
        }
    }
    else {
        if (isValidMove(selectedRow, selectedCol, targetRow, targetCol, currentPlayer)) {
            char movedPiece = board[selectedRow][selectedCol];
            char captured = board[targetRow][targetCol];
            bool castling = isCastlingMove(selectedRow, selectedCol, targetRow, targetCol, currentPlayer);

            // Make the move
            board[targetRow][targetCol] = movedPiece;
            board[selectedRow][selectedCol] = '-';
            if (castling) performCastling(targetRow, targetCol);

            // Check if move puts own king in check
            if (isKingInCheck(currentPlayer)) {
                // Undo the move
                board[selectedRow][selectedCol] = movedPiece;
                board[targetRow][targetCol] = captured;
                if (castling) performCastling(targetRow, targetCol);
            }
            else {
                // Update castling flags - no more shenanigans with castling after pieces move
                if (movedPiece == 'K') whiteKingMoved = true;
                if (movedPiece == 'k') blackKingMoved = true;
                if (movedPiece == 'R' && selectedCol == 0 && selectedRow == 7) whiteRookLeftMoved = true;
                if (movedPiece == 'R' && selectedCol == 7 && selectedRow == 7) whiteRookRightMoved = true;
                if (movedPiece == 'r' && selectedCol == 0 && selectedRow == 0) blackRookLeftMoved = true;
                if (movedPiece == 'r' && selectedCol == 7 && selectedRow == 0) blackRookRightMoved = true;

                // Pawn promotion
                if ((movedPiece == 'P' && targetRow == 0) || (movedPiece == 'p' && targetRow == 7)) {
                    promotePawn(targetRow, targetCol, currentPlayer);
                }

                // Switch players
                currentPlayer = (currentPlayer == 'w') ? 'b' : 'w';
            }
        }

        pieceSelected = false;
        selectedRow = -1;
        selectedCol = -1;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "SFML Chess Game - Perfect Alignment");
    initializeBoard();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int col = event.mouseButton.x / TILE_SIZE;
                int row = event.mouseButton.y / TILE_SIZE;

                if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE) {
                    handleMove(row, col);
                }
            }
        }

        window.clear(sf::Color::White);
        drawBoard(window);
        drawPieces(window);
        window.display();
    }

    return 0;
}
