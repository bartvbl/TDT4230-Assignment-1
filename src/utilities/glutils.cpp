#include <cassert>
#include <vector>
#include <glad/glad.h>
#include <program.hpp>
#include "glutils.hpp"

template <class T>
GLuint generateAttribute(int id, int elementsPerEntry, std::vector<T> data, bool normalize) {
    GLuint bufferID = 0;
    glGenBuffers(1, &bufferID);
    assert(bufferID != 0);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(id, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, sizeof(T), 0);
    glEnableVertexAttribArray(id);
    return bufferID;
}

GLuint generateBuffer(Mesh &mesh) {
    GLuint vaoID = 0;
    glGenVertexArrays(1, &vaoID);
    assert(vaoID != 0);
    glBindVertexArray(vaoID);

    generateAttribute(0, 3, mesh.vertices, false);
    if (mesh.normals.size() > 0) {
        generateAttribute(1, 3, mesh.normals, true);
    }
    if (mesh.textureCoordinates.size() > 0) {
        generateAttribute(2, 2, mesh.textureCoordinates, false);
    }

    GLuint indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    return vaoID;
}
