/*
Author: Leandro Alan Kim
Class: ECE4122/6122
Last Date Modified: Dec 6 2024
Description: C++ application that uses a custom class(es) with OpenGL, a third-party library (i.e.
ASSIMP), and a third-party chess engine (i.e. Komodo, see attached file) to create a fully operational
3D chess game.
*/

#include "chess_game.h"
#include "chessCommon.h"
#include <cmath>
#include <algorithm>
#include <iostream>

ChessGame::ChessGame() : gameOver(false), whiteToMove(true) {
    // piece positions stored when game is played
}

// manage the game's states, turns, and valid moves
bool ChessGame::isValidSquare(const std::string& square) const {
    if (square.length() != 2) return false;
    char file = square[0];
    char rank = square[1];
    return (file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8');
}

/**
 * map of the chessboard to 3d positions
 * @param square chess square
 * @return position of the square
 */
glm::vec3 ChessGame::squareToPosition(const std::string& square) const {
    if (!isValidSquare(square)) return glm::vec3(0);
    
    // chess notation to board coordinates
    float x = (square[0] - 'a' - 3.5f) * CHESS_BOX_SIZE;
    float y = (square[1] - '1' - 3.5f) * CHESS_BOX_SIZE;
    
    return glm::vec3(x, y, PHEIGHT);
}

/**
 * convert 3d position to chess square
 * @param position 3d position to be converted
 * @return chess square in string
 */
std::string ChessGame::positionToSquare(const glm::vec3& position) const {
    // board coordinates back to chess notation
    int file = static_cast<int>(round(position.x / CHESS_BOX_SIZE + 3.5f));
    int rank = static_cast<int>(round(position.y / CHESS_BOX_SIZE + 3.5f));
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return "";
    
    return std::string(1, 'a' + file) + std::string(1, '1' + rank);
}

/**
 * checks moves between two squares
 * @param from starting square
 * @param to destination square
 * @return true if move is valid
 */
bool ChessGame::isValidMove(const std::string& from, const std::string& to) const {
    // Basic move validation
    if (!isValidSquare(from) || !isValidSquare(to)) return false;
    if (from == to) return false;
    
    // accepting any move between valid squares
    // needs more work. come back to it!!!
    return true;
}

/**
 * excuting a move in chess game
 * @param move from-to move
 * @return true if move is made
 */
bool ChessGame::makeMove(const std::string& move) {
    if (move.length() != 4) return false;
    
    // from-to square from move square
    std::string from = move.substr(0, 2);
    std::string to = move.substr(2, 2);
    
    if (!isValidMove(from, to)) {
        std::cout << "Invalid move: " << move << std::endl;
        return false;
    }
    
    // find piece at from
    std::string movingPiece;
    glm::vec3 fromPos = squareToPosition(from);
    for (const auto& pair : piecePositions) {
        // check if its the same
        if (glm::length(pair.second - fromPos) < 0.1f) {
            movingPiece = pair.first;
            break;
        }
    }
    
    if (movingPiece.empty()) {
        std::cout << "No piece at position: " << from << std::endl;
        return false;
    }
    
    // movement animation
    PieceMovement movement;
    movement.pieceId = movingPiece;
    movement.startPos = fromPos;
    movement.endPos = squareToPosition(to);
    movement.progress = 0.0f;
    // check if its a knight
    movement.isKnight = (movingPiece.find("Object") != std::string::npos || 
                        movingPiece.find("CAVALLO") != std::string::npos);
    movement.isCapture = false;
    
    // check for capture
    for (const auto& pair : piecePositions) {
        if (glm::length(pair.second - movement.endPos) < 0.1f) {
            movement.isCapture = true;
            if (onPieceCaptured) {
                onPieceCaptured(pair.first);
            }
            break;
        }
    }
    
    activeMovements.push_back(movement);
    // next turn
    whiteToMove = !whiteToMove;
    return true;
}

void ChessGame::updateAnimations(float deltaTime) {
    for (auto& movement : activeMovements) {
        movement.progress = std::min(1.0f, movement.progress + deltaTime / MOVEMENT_DURATION);
        
        // new position
        glm::vec3 newPos;
        if (movement.isKnight) {
            // for knights
            float t = movement.progress;
            newPos = movement.startPos * (1.0f - t) + movement.endPos * t;
            newPos.z += KNIGHT_HEIGHT * sin(t * M_PI); // Parabolic height
        } else {
            // movement for other pieces
            newPos = glm::mix(movement.startPos, movement.endPos, movement.progress);
        }
        
        // update piece position
        piecePositions[movement.pieceId] = newPos;
    }
    
    // completed movements
    activeMovements.erase(
        std::remove_if(activeMovements.begin(), activeMovements.end(),
            [](const PieceMovement& m) { return m.progress >= 1.0f; }),
        activeMovements.end()
    );
}

// check if pieces are moving
bool ChessGame::isMoving() const {
    return !activeMovements.empty();
}

bool ChessGame::isCheckmate() const {
    // need to come back for checkmate function!!!
    return false;
}

/**
 * get position of a piece
 * @param pieceId id to identify each piece
 * @return position of the piece
 */
glm::vec3 ChessGame::getPiecePosition(const std::string& pieceId) const {
    auto it = piecePositions.find(pieceId);
    return (it != piecePositions.end()) ? it->second : glm::vec3(0);
}