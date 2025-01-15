#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

const std::string KOMODO_PATH = "./komodo-14_224afb/Linux/komodo-14.1-linux";

// Function to make pipe non-blocking
void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Function to read engine output until we get a complete response
std::string readUntilComplete(int fd) {
    std::string output;
    char buffer[4096];
    bool complete = false;
    int readAttempts = 0;
    
    while (!complete && readAttempts < 100) {
        ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            output += buffer;
            if (output.find("bestmove") != std::string::npos) {
                complete = true;
            }
        } else if (bytes == 0) {
            break;
        }
        
        if (!complete) {
            usleep(10000); // Wait 10ms before next read
            readAttempts++;
        }
    }
    return output;
}

void startKomodo() {
    if (execlp(KOMODO_PATH.c_str(), "komodo", nullptr) == -1) {
        std::cerr << "Failed to start Komodo. Error: " << strerror(errno) << "\n";
        std::cerr << "Make sure " << KOMODO_PATH << " exists and is executable.\n";
        exit(EXIT_FAILURE);
    }
}

int main() {
    if (access(KOMODO_PATH.c_str(), X_OK) == -1) {
        std::cerr << "Cannot access Komodo engine at " << KOMODO_PATH << "\n";
        std::cerr << "Error: " << strerror(errno) << "\n";
        return EXIT_FAILURE;
    }

    int inPipe[2], outPipe[2];
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1) {
        std::cerr << "Failed to create pipes.\n";
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Failed to fork.\n";
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child process
        close(inPipe[1]);
        close(outPipe[0]);
        
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(outPipe[1], STDERR_FILENO);
        
        close(inPipe[0]);
        close(outPipe[1]);
        
        startKomodo();
    } else {
        // Parent process
        close(inPipe[0]);
        close(outPipe[1]);
        
        setNonBlocking(outPipe[0]);

        // Initialize engine
        std::cout << "Initializing Komodo Chess Engine...\n";
        write(inPipe[1], "uci\n", 4);
        std::string initResponse = readUntilComplete(outPipe[0]);
        
        write(inPipe[1], "isready\n", 8);
        std::string readyResponse = readUntilComplete(outPipe[0]);
        
        if (readyResponse.find("readyok") != std::string::npos) {
            std::cout << "Engine initialized successfully!\n\n";
        }

        std::cout << "Enter moves in UCI format (e.g., e2e4)\n";
        std::cout << "Type 'quit' to exit\n\n";

        while (true) {
            std::string userMove;
            std::cout << "Your move: ";
            std::cin >> userMove;

            if (userMove == "quit") {
                write(inPipe[1], "quit\n", 5);
                break;
            }

            // Send position and move
            std::string posCmd = "position startpos moves " + userMove + "\n";
            write(inPipe[1], posCmd.c_str(), posCmd.length());
            
            // Request engine move with 1 second think time
            std::string goCmd = "go movetime 1000\n";
            write(inPipe[1], goCmd.c_str(), goCmd.length());
            
            std::string response = readUntilComplete(outPipe[0]);
            
            // Extract and display best move
            size_t pos = response.find("bestmove");
            if (pos != std::string::npos) {
                std::string bestMove = response.substr(pos);
                std::cout << "Engine response: " << bestMove << "\n\n";
            }
        }

        close(inPipe[1]);
        close(outPipe[0]);
        wait(nullptr);
    }

    return 0;
}