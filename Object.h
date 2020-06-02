#pragma once
#include <vector>
#include "GeometryNode.h"

class Object {

public:
	Object(void);

	void setGeometricNode(GeometryNode* gn);
	GeometryNode* getGeometricNode();

	void setTransformationMatrix(glm::mat4 transformation_matrix);
	glm::mat4* getTransformationMatrix();

	void setNormalMatrix();
	glm::mat4* getNormalMatrix();

	void setPosition(glm::vec3 n_p);
	glm::vec3 getPosition();

	void setDirection(glm::vec3 n_d);
	glm::vec3 getDirection();

	void ReduceCoins();

	~Object();


	GeometryNode*			m_geometric_node;
	glm::vec3				m_position; //position at each frame
	glm::mat4				m_geometric_transformation_matrix;
	glm::mat4				m_geometric_transformation_normal_matrix;

	glm::vec3				direction = glm::vec3(0);//for projectiles movement (target)
	float					used = 0.0; //for tower firing (hold timestamp of last time fired)
								  //for traps expiration (hold timestamp of placement)
	int						coins = 0; //for Chests
};