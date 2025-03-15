#include <cassert>
#include <chrono>
#include <fmt/format.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <iostream>
#include "gamelogic.hpp"
#include "sceneGraph.hpp"
#include "timestamps.hpp"
#include "utilities/camera.hpp"
#include "utilities/glutils.hpp"
#include "utilities/mesh.hpp"
#include "utilities/shader.hpp"
#include "utilities/shapes.hpp"
#include "utilities/timeutils.h"
#include "utilities/imageLoader.hpp"
#include "utilities/glfont.hpp"
#include "utilities/window.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

// These are heap allocated, because they should not
// be initialised at the start of the program
sf::SoundBuffer* soundBuffer;
sf::Sound* sound;

// CLI options, parsed in main.cpp
CommandLineOptions cliOptions;

// Camera instance
bool useFreeCamera = false;
Gloom::Camera* freeCamera;





// This is the root scene node that everuthing builds on
SceneNode* rootNode = nullptr;





//// A few lines to help you if you've never used c++ structs:
// struct LightSource {
//     bool a_placeholder_value;
// };
// LightSource lightSources[/*Put number of light sources you want here*/];



// ===============================
//  State and config for Breakout!
// ===============================
bool breakoutHasStarted = false;
bool breakoutPlayerHasLost = false;
bool breakoutJumpedToNextFrame = false;
bool breakoutIsPaused = false;

glm::vec2 padPosition(0, 0);
unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

double ballRadius = 3.0f;
glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double elapsedGameTimeTotal = debug_startTime;
double elapsedGameTimePlayed = debug_startTime;

SceneNode* boxNode = nullptr;
SceneNode* ballNode = nullptr;
SceneNode* padNode = nullptr;
// ===============================




// ===============================
//         mouse handling
// ===============================
bool mouseLeftPressed = false;
bool mouseLeftReleased = false;
bool mouseRightPressed = false;
bool mouseRightReleased = false;
double mouseSensitivity = 1.0;
void mouseCursorPosCallback(GLFWwindow* window, double x, double y) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    static double lastMouseX = (float) initialWindowWidth / 2;
    static double lastMouseY = (float) initialWindowHeight / 2;
    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    bool resetMouse = false;
    if (useFreeCamera) {
        resetMouse = freeCamera->handleCursorDeltaInput(deltaX, deltaY);
    } else {
        resetMouse = breakoutMouseDeltaHandler(deltaX, deltaY);
    }

    if (resetMouse) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        lastMouseX = (float) windowWidth / 2;
        lastMouseY = (float) windowHeight / 2;
        glfwSetCursorPos(window, lastMouseX, lastMouseY);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        lastMouseX = x;
        lastMouseY = y;
    }
}
// ===========================



void initGame(GLFWwindow* window, CommandLineOptions options) {
    cliOptions = options;
    if (cliOptions.enableMusic) {
        soundBuffer = new sf::SoundBuffer();
        if (!soundBuffer->loadFromFile("../res/Hall of the Mountain King.ogg")) {
            return;
        }
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCursorPosCallback);
    glfwSetKeyCallback(window, keyboardCallback);

    // init camera
    freeCamera = new Gloom::Camera(
        // camera position
        glm::vec3(0, 2, -20),
        // movementSpeed
        50.0f,
        // mouseSensitivity
        0.005f
        // lock skyward direction
        // true
    );


    // Define shaders
    Gloom::Shader* simpleShader = new Gloom::Shader();
    simpleShader->makeBasicShader(
        "../res/shaders/simple.vert",
        "../res/shaders/simple.frag");
    simpleShader->activate();


    // Construct scene
    assert(rootNode == nullptr); // ensure we don't run initGame twice
    rootNode = createSceneNode(simpleShader, GEOMETRY);

    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    GLuint padVAO = generateBuffer(pad);
    padNode = createSceneNode(simpleShader, GEOMETRY);
    padNode->vertexArrayObjectID = padVAO;
    padNode->VAOIndexCount = pad.indices.size();
    rootNode->children.push_back(padNode);

    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    GLuint boxVAO = generateBuffer(box);
    boxNode = createSceneNode(simpleShader, GEOMETRY);
    boxNode->vertexArrayObjectID = boxVAO;
    boxNode->VAOIndexCount = box.indices.size();
    rootNode->children.push_back(boxNode);

    Mesh sphere = generateSphere(1.0, 40, 40);
    GLuint ballVAO = generateBuffer(sphere);
    ballNode = createSceneNode(simpleShader, GEOMETRY);
    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = sphere.indices.size();
    rootNode->children.push_back(ballNode);


    // call once to init
    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;
    std::cout << "Ready. Click to start!" << std::endl;
    std::cout << "Or press C to move around!" << std::endl;
}

// ===============================
//      Breakout! game logic
// ===============================
bool breakoutMouseDeltaHandler(double deltaX, double deltaY) {
    padPosition.x -= mouseSensitivity * deltaX / initialWindowWidth;
    padPosition.y -= mouseSensitivity * deltaY / initialWindowHeight;

    if (padPosition.x > 1) padPosition.x = 1;
    if (padPosition.x < 0) padPosition.x = 0;
    if (padPosition.y > 1) padPosition.y = 1;
    if (padPosition.y < 0) padPosition.y = 0;

    return true;
}
void updateBreakoutState(double timeDelta, bool listenToInputs) {
    const float ballBottomY = boxNode->position.y - (boxDimensions.y/2) + ballRadius + padDimensions.y;
    const float ballTopY    = boxNode->position.y + (boxDimensions.y/2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x/2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x/2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z/2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z/2) - ballRadius - cameraWallOffset;

    if(!breakoutHasStarted) {
        if (mouseLeftPressed && listenToInputs) {
            if (cliOptions.enableMusic) {
                sound = new sf::Sound();
                sound->setBuffer(*soundBuffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            elapsedGameTimeTotal = debug_startTime;
            elapsedGameTimePlayed = debug_startTime;
            breakoutHasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPosition.x) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPosition.y) * ((ballMaxZ+cameraWallOffset) - ballMinZ);
    } else {
        elapsedGameTimeTotal += timeDelta;
        if(breakoutPlayerHasLost) {
            if (mouseLeftReleased && listenToInputs) {
                breakoutPlayerHasLost = false;
                breakoutHasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        } else if (breakoutIsPaused) {
            if (mouseRightReleased && listenToInputs) {
                breakoutIsPaused = false;
                if (cliOptions.enableMusic) {
                    sound->play();
                }
            }
        } else {
            elapsedGameTimePlayed += timeDelta;
            if (mouseRightReleased && listenToInputs) {
                breakoutIsPaused = true;
                if (cliOptions.enableMusic) {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++) {
                if (elapsedGameTimePlayed < keyFrameTimeStamps.at(i)) {
                    continue;
                }
                currentKeyFrame = i;
            }

            breakoutJumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = elapsedGameTimePlayed - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM) {
                ballYCoord = ballBottomY;
            } else if (currentOrigin == TOP && currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            } else if (currentDestination == BOTTOM) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            } else if (currentDestination == TOP) {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX) {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            } else if (ballPosition.x > ballMaxX) {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ) {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            } else if (ballPosition.z > ballMaxZ) {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if(cliOptions.enableAutoplay) {
                padPosition.x = 1-(ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPosition.y = 1-(ballPosition.z - ballMinZ) / ((ballMaxZ+cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (breakoutJumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP) {
                double padLeftX  = boxNode->position.x - (boxDimensions.x/2) + (1 - padPosition.x) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z/2) + (1 - padPosition.y) * (boxDimensions.z - padDimensions.z);
                double padBackZ  = padFrontZ + padDimensions.z;

                if (   ballPosition.x < padLeftX
                    || ballPosition.x > padRightX
                    || ballPosition.z < padFrontZ
                    || ballPosition.z > padBackZ
                ) {
                    breakoutPlayerHasLost = true;
                    if (cliOptions.enableMusic) {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }
}
// ===============================


// This function is registered to be called by glfw on keypress __changes__
//     https://www.glfw.org/docs/latest/input_guide.html#input_key
// consider using glfwGetKey like in handleKeyboardInput in program.cpp
// if you find continuous true/false easier to work with
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (useFreeCamera) {
        freeCamera->handleKeyboardInputs(key, action);
    }

    // Quit on Q
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    // Toggle free camera on C
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        useFreeCamera = !useFreeCamera;
        std::cout << useFreeCamera << std::endl;
    }
}

float elapsedTime = 0;
void updateFrame(GLFWwindow* window) {
    // Time since previous frame, used to account for variation in frame times
    double timeDelta = getTimeDeltaSeconds();
    elapsedTime += timeDelta;

    // poll mouse buttons
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    // update free camera
    if (useFreeCamera) {
        freeCamera->handleMouseButtonInputs(GLFW_MOUSE_BUTTON_LEFT, mouseLeftPressed);
    }

    // update breakout
    updateBreakoutState(timeDelta, !useFreeCamera);


    // Move and rotate various SceneNodes
    boxNode->position = { 0, -10, -80 };

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = { 0, elapsedGameTimeTotal*2, 0 };

    padNode->position  = {
        boxNode->position.x - (boxDimensions.x/2) + (padDimensions.x/2) + (1 - padPosition.x) * (boxDimensions.x - padDimensions.x),
        boxNode->position.y - (boxDimensions.y/2) + (padDimensions.y/2),
        boxNode->position.z - (boxDimensions.z/2) + (padDimensions.z/2) + (1 - padPosition.y) * (boxDimensions.z - padDimensions.z)
    };






    // Compute camera and perspective

    glm::mat4 cameraTransform;

    if (useFreeCamera) {
        freeCamera->updateCamera(timeDelta);
        cameraTransform = freeCamera->getViewMatrix();

    } else {
        // Some math to make the Breakout! camera move in a nice way
        float lookRotation = -0.6 / (1 + exp(-5 * (padPosition.x-0.5))) + 0.3;
        glm::vec3 cameraPosition = glm::vec3(0, 2, -20);
        cameraTransform =
            glm::rotate(0.3f + 0.2f * float(-padPosition.y*padPosition.y), glm::vec3(1, 0, 0)) *
            glm::rotate(lookRotation, glm::vec3(0, 1, 0)) *
            glm::translate(-cameraPosition);
    }

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glm::mat4 projection = glm::perspective(
        glm::radians(80.0f), /// Field of view
        float(windowWidth) / float(windowHeight), // aspect
        0.1f, // Near
        350.f // Far
    );

    glm::mat4 VP = projection * cameraTransform;

    // traveerse the scene node graph and precompute all the transformation matrices
    updateNodeTransformations(rootNode, VP);
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    // Uncomment this line if you set any glUniforms here!
    // node->shader->activate();

    glm::mat4 localTransformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * localTransformationMatrix;

    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode* node) {
    node->shader->activate();

    glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != 0) {
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case POINT_LIGHT: break;
        case SPOT_LIGHT: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    renderNode(rootNode);
}
