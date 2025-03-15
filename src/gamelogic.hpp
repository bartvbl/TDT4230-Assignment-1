#pragma once

#include <GLFW/glfw3.h>

#include <utilities/window.hpp>
#include "sceneGraph.hpp"

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar);
void initGame(GLFWwindow* window, CommandLineOptions options);
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void updateFrame(GLFWwindow* window);
void renderFrame(GLFWwindow* window);

// not really public just required predeclares due to order
bool breakoutMouseDeltaHandler(double deltaX, double deltaY);
