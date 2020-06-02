#include <glm\glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "Pirate.h"
#include <iostream>
#include <glm\gtc\epsilon.hpp>
#define PI 3.14159265




Pirate::Pirate(){}

Pirate::Pirate(glm::vec3 position, glm::mat4 model_matrix) {
	m_geometric_transformation_matrix = model_matrix;
	m_position = position;
}

void Pirate::setPosition(glm::vec3 position,glm::mat4& model_matrix,float angle,float continous_time,float dt) {
	m_position = position;
	model_matrix = glm::translate(glm::mat4(1.0f), m_position)*glm::rotate(glm::mat4(1.f),-angle, glm::vec3(0, 1, 0))*dt;
	m_geometric_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_transformation_matrix))));
}

void Pirate::setFlag(bool x) {
	flag = x;
}


void Pirate::increasePositionCounter(){
	i = i + 1;
}

int Pirate::getPositionCounter() {
	return i;
}


void Pirate::movePirate(glm::mat4& model_matrix, glm::vec3 positions[19 * sizeof(glm::vec3)],float continous_time,float dt) {
	glm::vec3 current_pos;

	if (flag == false) 
	{
		current_pos = glm::vec3(-18,0,-18);
		angle = atan2(positions[getPositionCounter()].x - current_pos.x, -(positions[getPositionCounter()].z - current_pos.z));
		setPosition(current_pos,model_matrix,angle,0,0);  //with the previous logic,we gave angle = 180.
		setFlag(true);
		t1 = 0;
		t2 = 1;
	}
	else {
		current_pos = getPosition();
	}

	if (glm::epsilonEqual(current_pos.x,positions[i].x,0.27f) && glm::epsilonEqual(current_pos.z, positions[i].z, 0.27f)) 
	{
		//increase the counter so we can have as a target the next tile.
		increasePositionCounter();
		t1 = continous_time;
		t2 = continous_time + 1.f;
	}

	float factor_a = continous_time - t1 / (t2 - t1);

	factor_a *= velocity; //in case we want to alter the speed (velocity = 1.0 by default)
	
	current_pos = glm::mix(current_pos, positions[getPositionCounter()], glm::fract(abs(factor_a*dt)));
	angle = atan2(positions[getPositionCounter()].x - current_pos.x, -(positions[getPositionCounter()].z- current_pos.z));
	setPosition(current_pos, model_matrix, angle, continous_time,dt);
}


void Pirate::setHealth(int n_h)
{
	health = n_h;
}

void Pirate::reduceHealth()
{
	health--;
}

int Pirate::getHealth()
{
	return health;
}

void Pirate::SaveSpawnTime(float continuous_time) {
	spawn_time = continuous_time;
}

float Pirate::getSpawnTime() {
	return spawn_time;
}

glm::vec3 Pirate::getPosition() {
	return m_position;
}

GeometryNode* Pirate::getGeometricNode() {
	return m_geometric_node;
}

void Pirate::setGeometricNode(GeometryNode* gn) {
	m_geometric_node = gn;
}

void Pirate::setGeometricTransformationMatrix(glm::mat4 transformation_matrix) {
	m_geometric_transformation_matrix = transformation_matrix;
}

void Pirate::setNormalTransformationMatrix() {
	m_geometric_transformation_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(m_geometric_transformation_matrix))));
}

glm::mat4 Pirate::getGeometricTransformationMatrix()
{
	return m_geometric_transformation_matrix;
}

glm::mat4 Pirate::getGeometricTransformationNormalMatrix()
{
	return m_geometric_transformation_normal_matrix;
}

Pirate::~Pirate()
{
}
