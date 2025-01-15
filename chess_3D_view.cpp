/*
Author: Leandro Alan Kim
Class: ECE4122/6122
Last Date Modified: Dec 6 2024
Description: C++ application that uses a custom class(es) with OpenGL, a third-party library (i.e.
ASSIMP), and a third-party chess engine (i.e. Komodo, see attached file) to create a fully operational
3D chess game.
*/
/*
Objective:
To create a dynamic 3D graphics application using lighting, shading, model transformations, and keyboard inputs.
Description :
    Uses the code from tutorial09_Assimp to create a C++ application that loads in the obj files 
    for the 3D chess pieces and the chess board.

    Obje files used - Lab3/Chess/chess.obj
                    - Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj

    keyboard inputs definitions
        1) �w� key moves the camera radially closer to the origin.
        2) �s� key moves the camera radially farther from the origin.
        3) �a� key rotates the camera to the left maintaining the radial distance from the origin.
        4) �d� key rotates to camera to the right maintaining the radial distance from the origin.
        5) The up arrow key radially rotates the camera up.
        6) The down arrow radially rotates the camera down.
        7) The �L� key toggles the specular and diffuse components of the light on and off but leaves the ambient component unchanged.
        8) Pressing the escape key closes the window and exits the program
*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/select.h>
#include <unistd.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// User supporting files
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

// Lab3 specific chess classes
#include "chessComponent.h"
#include "chessCommon.h"
#include "chess_game.h"
#include "ECE_ChessEngine.h"


// Global chess game instance
ChessGame gChessGame;
glm::vec3 globalLightPos = glm::vec3(0, 0, 15);
float globalLightPower = 1.0f;
ECE_ChessEngine chessEngine;

// Sets up the chess board
void setupChessBoard(tModelMap& cTModelMap);

int main(void)
{
    // Initialize GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Game Of Chess 3D", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    
    // Initialize chess engine
    if (!chessEngine.InitializeEngine()) {
        std::cerr << "Failed to initialize chess engine\n";
        return -1;
    }

    // Ensure we can capture the escape key
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); 

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our uniforms
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
    GLuint LightSwitchID = glGetUniformLocation(programID, "lightSwitch");
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    GLuint PowerID = glGetUniformLocation(programID, "lightPower");

    // Load chess components
    std::vector<chessComponent> gchessComponents;
    bool cBoard = loadAssImpLab3("Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj", gchessComponents);
    bool cComps = loadAssImpLab3("Lab3/Chess/chess-mod.obj", gchessComponents);

    if (!cBoard || !cComps) {
        std::cout << "obj not loading" << std::endl;
        return -1;
    }
    
    // Setup the Chess board locations
    tModelMap cTModelMap;
    setupChessBoard(cTModelMap);

    // Load it into VBO
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++) {
        cit->setupGLBuffers();
        cit->setupTextureBuffers();
    }

    // Use our shader
    glUseProgram(programID);
    //glUniform1f(PowerID, 1.0f);
    glUniform1f(PowerID, globalLightPower);

    // For speed computation
    double lastTime = glfwGetTime();
    
    char inputBuffer[256];
    int bufferPos = 0;

    do {
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    lastTime = currentTime;

    // Update chess animations
    gChessGame.updateAnimations(deltaTime);

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute the MVP matrix from keyboard and mouse input
    computeMatricesFromInputsLab3();
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();

    // Get light switch state and pass to shader
    bool lightSwitch = getLightSwitch();
    glUniform1i(LightSwitchID, static_cast<int>(lightSwitch));
    glUniform1f(PowerID, globalLightPower);

    // Render chess components
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++) {            
        tPosition cTPosition = cTModelMap[cit->getComponentID()];
        
        for (unsigned int pit = 0; pit < cTPosition.rCnt; pit++) {
            tPosition cTPositionMorph = cTPosition;
            cTPositionMorph.tPos.x += pit * cTPosition.rDis * CHESS_BOX_SIZE;
            
            if (gChessGame.isMoving()) {
                glm::vec3 gamePos = gChessGame.getPiecePosition(cit->getComponentID());
                if (gamePos != glm::vec3(0)) {
                    cTPositionMorph.tPos = gamePos;
                }
            }

            glm::mat4 ModelMatrix = cit->genModelMatrix(cTPositionMorph);
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

            glUniform3f(LightID, globalLightPos.x, globalLightPos.y, globalLightPos.z);
            glUniform1f(PowerID, globalLightPower);

            cit->setupTexture(TextureID);
            cit->renderMesh();
        }
    }

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

    // Check for command input without blocking
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // command loops for move, camera, light, quit commands
    if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
        std::string command;
        std::cout << "Please enter a command: ";
        std::cin >> command;

        if (command == "move") {
            std::string moveStr;
            std::cin >> moveStr;
            if (moveStr.length() != 4) {
                std::cout << "Invalid command or move!!\n";
            } else {
                if (chessEngine.sendMove(moveStr)) {
                    std::string engineMove;
                    if (chessEngine.getResponseMove(engineMove)) {
                        std::cout << "Engine plays: " << engineMove << std::endl;
                    }
                }
            }
        }
        else if (command == "camera") {
            float theta, phi, r;
            std::cin >> theta >> phi >> r;
            if (r <= 0.0f || theta < 10.0f || theta > 80.0f || 
                phi < 0.0f || phi > 360.0f) {
                std::cout << "Invalid command or move!!\n";
            } else {
                cTheta = theta;
                cPhi = phi;
                cRadius = r;
            }
        }
        else if (command == "light") {
            float theta, phi, r;
            std::cin >> theta >> phi >> r;
            if (r <= 0.0f || theta < 10.0f || theta > 80.0f || 
                phi < 0.0f || phi > 360.0f) {
                std::cout << "Invalid command or move!!\n";
            } else {
                globalLightPos = glm::vec3(
                    r * sin(glm::radians(theta)) * cos(glm::radians(phi)),
                    r * sin(glm::radians(theta)) * sin(glm::radians(phi)),
                    r * cos(glm::radians(theta))
                );
                glUniform3f(LightID, globalLightPos.x, globalLightPos.y, globalLightPos.z);
                // check to see if the positions are right
                std::cout << "Light position set to: " << globalLightPos.x << ", " 
                          << globalLightPos.y << ", " << globalLightPos.z << std::endl;
            }
        }
        else if (command == "power") {
            float powerValue;
            std::cin >> powerValue;
            if (powerValue >= 0.0f && powerValue <= 100.0f) {
                globalLightPower = powerValue / 100.0f;
                glUniform1f(PowerID, globalLightPower);
                std::cout << "Light power set to: " << powerValue << std::endl;
            } else {
                std::cout << "Invalid command or move!!\n";
            }
        }
        else if (command == "quit") {
            std::cout << "Thanks for playing!!\n";
            break;
        }
        else {
            std::cout << "Invalid command or move!!\n";
        }
    }

} while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && 
        glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


void setupChessBoard(tModelMap& cTModelMap)
{
    // Target spec Hash
    cTModelMap =
    {
        // Chess board              Count  rDis Angle      Axis             Scale                          Position (X, Y, Z)
        {"12951_Stone_Chess_Board", {1,    0,   0.f,    {1, 0, 0},    glm::vec3(CBSCALE), {0.f,     0.f,                             PHEIGHT}}},
        // First player             Count  rDis Angle      Axis             Scale                          Position (X, Y, Z)
        {"TORRE3",                  {2,   (8-1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-3.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"Object3",                 {2,   (6-1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-2.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"ALFIERE3",                {2,   (4-1),90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-1.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"REGINA2",                 {1,    0,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-0.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"RE2",                     {1,    0,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), { 0.5*CHESS_BOX_SIZE, -3.5*CHESS_BOX_SIZE, PHEIGHT}}},
        {"PEDONE13",                {8,    1,   90.f,   {1, 0, 0},    glm::vec3(CPSCALE), {-3.5*CHESS_BOX_SIZE, -2.5*CHESS_BOX_SIZE, PHEIGHT}}}
    };

    // Second player derived from first player!!
    // Second Player (TORRE02)
    cTModelMap["TORRE02"] = cTModelMap["TORRE3"];
    cTModelMap["TORRE02"].tPos.y = -cTModelMap["TORRE3"].tPos.y;
    // Second Player (Object02)
    cTModelMap["Object02"] = cTModelMap["Object3"];
    cTModelMap["Object02"].tPos.y = -cTModelMap["Object3"].tPos.y;
    // Second Player (ALFIERE02)
    cTModelMap["ALFIERE02"] = cTModelMap["ALFIERE3"];
    cTModelMap["ALFIERE02"].tPos.y = -cTModelMap["ALFIERE3"].tPos.y;
    // Second Player (REGINA01)
    cTModelMap["REGINA01"] = cTModelMap["REGINA2"];
    cTModelMap["REGINA01"].tPos.y = -cTModelMap["REGINA2"].tPos.y;
    // Second Player (RE01)
    cTModelMap["RE01"] = cTModelMap["RE2"];
    cTModelMap["RE01"].tPos.y = -cTModelMap["RE2"].tPos.y;
    // Second Player (PEDONE12)
    cTModelMap["PEDONE12"] = cTModelMap["PEDONE13"];
    cTModelMap["PEDONE12"].tPos.y = -cTModelMap["PEDONE13"].tPos.y;
}
