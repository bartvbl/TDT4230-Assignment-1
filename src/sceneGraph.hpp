#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cstdio>
#include <stdbool.h>
#include <cstdlib>
#include <ctime>

#include "utilities/shader.hpp"

enum SceneNodeType {
	// Geometry nodes have
	GEOMETRY,
	// Point l
	POINT_LIGHT,
	// Light with direction and spread
	SPOT_LIGHT,
};

struct SceneNode {
	SceneNode(Gloom::Shader *shader, SceneNodeType nodeType) {
		this->shader = shader;
		this->nodeType = nodeType;
	}

	// The node's rigid transform relative to its parent
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(0, 0, 0);
	glm::vec3 scale = glm::vec3(1, 1, 1);

	// The location of the node's reference point, about which rotation and scaling happens
	glm::vec3 referencePoint = glm::vec3(0, 0, 0);

	// A transformation matrix representing the total transformation of this
	// node in the scene, also contianing what it inherited from its parent.
	// This matrix is updated every frame by updateNodeTransformations.
	glm::mat4 currentTransformationMatrix;

	// A list of all children that belong to this node. For instance, in case of
	// the scene graph of a human body shown in the assignment text, the "Upper
	// Torso" node would contain the "Left Arm", "Right Arm", "Head" and "Lower
	// Torso" nodes in its list of children.
	std::vector<SceneNode*> children;

	// The shader this node should be drawn with
	Gloom::Shader *shader;

	// Used to determine how to handle the contents of a node
	// in places like renderNode
	SceneNodeType nodeType;

	// The ID of the VAO containing the "appearance" of this SceneNode if any.
	GLuint vertexArrayObjectID = 0;
	unsigned int VAOIndexCount = 0;

};

SceneNode* createSceneNode(Gloom::Shader *shader, SceneNodeType nodeType);
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
int totalChildren(SceneNode* parent);

// For more details, see SceneGraph.cpp.
