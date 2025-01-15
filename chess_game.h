/*
Author: Leandro Alan Kim
Class: ECE4122/6122
Last Date Modified: Dec 6 2024
Description: C++ application that uses a custom class(es) with OpenGL, a third-party library (i.e.
ASSIMP), and a third-party chess engine (i.e. Komodo, see attached file) to create a fully operational
3D chess game.
*/

#ifndef CHESS_GAME_H
#define CHESS_GAME_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <functional>

// chess piece movement animation
struct PieceMovement {
    std::string pieceId;
    glm::vec3 startPos;
    glm::vec3 endPos;
    float progress;
    bool isKnight;
    bool isCapture;
};

class ChessGame {
private:
    // state of game
    std::map<std::string, glm::vec3> piecePositions;  // curr position of each piece
    std::vector<PieceMovement> activeMovements;       // curr animating moves
    bool gameOver;
    bool whiteToMove;
    
    const float MOVEMENT_DURATION = 2.0f;  // movement speed
    const float KNIGHT_HEIGHT = 2.0f;      // height of knight when jumping
    
    // validate movement
    bool isValidMove(const std::string& from, const std::string& to) const;
    bool isValidSquare(const std::string& square) const;
    glm::vec3 squareToPosition(const std::string& square) const;
    std::string positionToSquare(const glm::vec3& position) const;

public:
    ChessGame();
    
    // gamestate and movement
    bool makeMove(const std::string& move); // e.g., "e2e4"
    void updateAnimations(float deltaTime);
    bool isMoving() const;
    bool isCheckmate() const;
    
    // getters
    glm::vec3 getPiecePosition(const std::string& pieceId) const;
    bool isGameOver() const { return gameOver; }
    bool isWhiteToMove() const { return whiteToMove; }
    
    // for piece capture
    std::function<void(const std::string&)> onPieceCaptured;
};

#endif