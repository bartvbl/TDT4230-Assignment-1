#include <cassert>
#include <iostream>
#include <glm/glm.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System/Time.hpp>

#include "gamelogic.hpp"
#include "program.hpp"
#include "utilities/glutils.hpp"
#include "utilities/shader.hpp"
#include "utilities/shapes.hpp"
#include "utilities/timeutils.h"
#include "utilities/window.hpp"

void GLAPIENTRY
openglErrorCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam
)
{
    const char *typeStr;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "OTHER"; break;
        default:
            typeStr = "UNKNOWN"; break;
    }

    fprintf(stderr,
        "GL CALLBACK: %s (0x%x), %s (severity = 0x%x)\n",
        typeStr,
        type,
        message,
        severity
    );
}

void runProgram(GLFWwindow* window, CommandLineOptions options)
{
    // Enable depth (Z) buffer (accept "closest" fragment)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Configure miscellaneous OpenGL settings
    glEnable(GL_CULL_FACE);

    // Disable built-in dithering
    glDisable(GL_DITHER);

    // Enable transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable debug output
#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglErrorCallback, 0);
#endif

    // Set default colour after clearing the colour buffer
    glClearColor(0.3f, 0.5f, 0.8f, 1.0f);

	initGame(window, options);

    // Rendering Loop
    while (!glfwWindowShouldClose(window))
    {
	    // Clear colour and depth buffers
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Handle other events
        glfwPollEvents();
        handleKeyboardInput(window);

        updateFrame(window);
        renderFrame(window);


        // Flip buffers (double buffering)
        glfwSwapBuffers(window);
    }
}

void handleKeyboardInput(GLFWwindow* window)
{
    // Use escape key for terminating the GLFW window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}
