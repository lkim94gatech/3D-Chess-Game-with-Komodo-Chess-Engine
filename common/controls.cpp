/*

Objective:
Use the common.cpp file from the Git Tutorial and add a custom function
for ECE6122 Labs. It does NOT touch the existing functions.

*/

// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include "controls.hpp"
#include "chess_game.h"

extern ChessGame gChessGame;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
bool toggleSDColor = true;
int dbnceCnt = 20;

// Initial camera position and angles
glm::mat4 getViewMatrix() {
    return ViewMatrix;
}

glm::mat4 getProjectionMatrix() {
    return ProjectionMatrix;
}

bool getLightSwitch() {
    return toggleSDColor;
}

// Initial Field of View
float initialFoV = 45.0f;

// Lab3 key movement coordinates (spherical)
extern float cRadius = 10.0f;  // Initial radius
extern float cPhi = -90.f;     // Initial phi angle
extern float cTheta = 90.0f;   // Initial theta angle

// Speed
float speedLab3 = 10.0f;

// Debounce limit
const int DEB_LIMIT = 40;

// Creates the view and Projection matrix based on camera controls
void computeMatricesFromInputsLab3() {
    // Timer for frame updates
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // Debounce counter
    if (dbnceCnt <= DEB_LIMIT) {
        dbnceCnt++;
    }

    // Create origin and up vector
    glm::vec3 origin = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0, 0, 1);

    // w key - move camera closer
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        float lastcRadius = cRadius;
        cRadius -= deltaTime * speedLab3;
        if (cRadius < 0) {
            cRadius = lastcRadius;
        }
    }

    // s key - move camera farther
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cRadius += deltaTime * speedLab3;
    }

    // a key - rotate camera left
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cPhi += deltaTime * speedLab3;
        if (cPhi > 360.0f) {
            cPhi = cPhi - 360.f;
        }
    }

    // d key - rotate camera right
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cPhi -= deltaTime * speedLab3;
        if (cPhi < 0.f) {
            cPhi = 360.f + cPhi;
        }
    }

    // up arrow - rotate camera up
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cTheta += deltaTime * speedLab3;
        if (cTheta > 360.0f) {
            cTheta = cTheta - 360.f;
        }
    }

    // down arrow - rotate camera down
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cTheta -= deltaTime * speedLab3;
        if (cTheta < 0.0f) {
            cTheta = 360.f + cTheta;
        }
    }

    // L key - toggle lighting
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (dbnceCnt >= DEB_LIMIT) {
            toggleSDColor = !toggleSDColor;
            dbnceCnt = 0;
        }
    }

    // Convert spherical to Cartesian coordinates
    float posX = cRadius * sin(glm::radians(cTheta)) * cos(glm::radians(cPhi));
    float posY = cRadius * sin(glm::radians(cTheta)) * sin(glm::radians(cPhi));
    float posZ = cRadius * cos(glm::radians(cTheta));

    // Set up camera position
    glm::vec3 position = glm::vec3(posX, posY, posZ);
    float FoV = initialFoV;

    // Create projection matrix
    ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

    // Handle camera flip for smooth rotation
    if (cTheta > 180.0f) {
        up = glm::vec3(0, 0, -1);
    }

    // Create view matrix
    ViewMatrix = glm::lookAt(
        position,   // Camera position
        origin,     // Look at origin
        up          // Up vector
    );

    lastTime = currentTime;
}