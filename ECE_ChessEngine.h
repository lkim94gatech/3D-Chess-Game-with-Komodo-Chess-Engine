/*
Author: Leandro Alan Kim
Class: ECE4122/6122
Last Date Modified: Dec 6 2024
Description: Interface class for communicating with the Komodo chess engine using UCI protocol
*/

#ifndef ECE_CHESS_ENGINE_H
#define ECE_CHESS_ENGINE_H

#include <string>
#include <unistd.h>

// manage communication with the chess engine
class ECE_ChessEngine {
private:
    int inPipe[2];
    int outPipe[2];
    pid_t enginePid;
    bool isRunning;

public:
    ECE_ChessEngine() : isRunning(false) {}
    ~ECE_ChessEngine();

    bool InitializeEngine();
    bool sendMove(const std::string& strMove);
    bool getResponseMove(std::string& strMove);
    
private:
    void SendToEngine(const std::string& command);
    std::string ReadFromEngine();
};

#endif

