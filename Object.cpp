#include "Object.h"

Object::Object() {}

void Object::setGeometricNode(GeometryNode* gn) {
	m_geometric_node = gn;
}

GeometryNode* Object::getGeometricNode() {
	return m_geometric_node;
}

void Object::setTransformationMatrix(glm::mat4 transformation_matrix) {
	m_geometric_transformation_matrix = transformation_matrix;
}

glm::mat4* Object::getTransformationMatrix()
{
	return &m_geometric_transformation_matrix;
}

void Object::setNormalMatrix() {
	m_geometric_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_transformation_matrix))));
}

glm::mat4* Object::getNormalMatrix()
{
	return &m_geometric_transformation_normal_matrix;
}

void Object::setPosition(glm::vec3 n_p)
{
	m_position = n_p;
}

glm::vec3 Object::getPosition()
{
	return m_position;
}

void Object::setDirection(glm::vec3 n_d)
{
	direction = n_d;
}

glm::vec3 Object::getDirection()
{
	return direction;
}

//Reduces total coins by 10
void Object::ReduceCoins()
{
	coins -= 10;
}


Object::~Object() {}