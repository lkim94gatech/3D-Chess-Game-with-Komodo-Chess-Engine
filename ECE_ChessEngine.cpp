/*
Author: Leandro Alan Kim
Class: ECE4122/6122
Last Date Modified: Dec 6 2024
Description: Implementation of chess engine interface
*/

#include "ECE_ChessEngine.h"
#include <iostream>
#include <string.h>
#include <sys/wait.h>

// destructor and pipeline
ECE_ChessEngine::~ECE_ChessEngine() {
    if (isRunning) {
        SendToEngine("quit");
        close(inPipe[1]);
        close(outPipe[0]);
        waitpid(enginePid, NULL, 0);
    }
}

// send command to chess enginge
void ECE_ChessEngine::SendToEngine(const std::string& command) {
    std::string cmd = command + "\n";
    write(inPipe[1], cmd.c_str(), cmd.length());
}

// reads from chess engine
std::string ECE_ChessEngine::ReadFromEngine() {
    char buffer[4096];
    std::string output;
    int bytes = read(outPipe[0], buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        output = buffer;
    }
    return output;
}

/**
 * pipeline to communicate with the chess engine
 * @return true if engine is initialized
 */
bool ECE_ChessEngine::InitializeEngine() {
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1) {
        return false;
    }

    enginePid = fork();
    if (enginePid == -1) {
        return false;
    }

    // child process
    if (enginePid == 0) {
        close(inPipe[1]);
        close(outPipe[0]);
        
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        
        close(inPipe[0]);
        close(outPipe[1]);

        execl("./chess_engine/komodo-14_224afb/Linux/komodo-14.1-linux", "komodo", NULL); // path
        exit(1);
    }

    // parent process
    close(inPipe[0]);
    close(outPipe[1]);

    // uci mode
    SendToEngine("uci");
    std::string response;
    while (response.find("uciok") == std::string::npos) {
        response = ReadFromEngine();
    }

    SendToEngine("isready");
    response = ReadFromEngine();
    
    isRunning = true;
    return true;
}

/**
 * sends chess moves to the engine
 * @param strMove string of the move
 * @return true if command is successful
 */
bool ECE_ChessEngine::sendMove(const std::string& strMove) {
    if (!isRunning) return false;
    
    SendToEngine("position startpos moves " + strMove);
    SendToEngine("go depth 10");
    return true;
}

/**
 * get the best move computed by chess engine
 * @param strMove string of the move
 * @return true if best move is recieved
 */
bool ECE_ChessEngine::getResponseMove(std::string& strMove) {
    if (!isRunning) return false;

    std::string response;
    while ((response = ReadFromEngine()).find("bestmove") == std::string::npos) {
        // Keep reading until we get bestmove
    }
    
    size_t pos = response.find("bestmove");
    if (pos != std::string::npos) {
        strMove = response.substr(pos + 9, 4);
        return true;
    }
    return false;
}