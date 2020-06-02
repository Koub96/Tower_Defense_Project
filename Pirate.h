#pragma once
#include <glm\glm.hpp>
#include <vector>
#include "GeometryNode.h"
#include "GLEW\glew.h"
class Pirate {
public:
	Pirate(void);
	Pirate(glm::vec3 position, glm::mat4 model_matrix);
	void setPosition(glm::vec3 position,glm::mat4 &model_matrix,float angle,float continous_time,float dt);
	glm::vec3 getPosition(void);
	GeometryNode* getGeometricNode();
	void setGeometricNode(GeometryNode* gn);
	glm::mat4 getGeometricTransformationMatrix();
	void setGeometricTransformationMatrix(glm::mat4 transformation_matrix);
	void setNormalTransformationMatrix();
	glm::mat4 getGeometricTransformationNormalMatrix();

	void movePirate(glm::mat4& model_matrix, glm::vec3 positions[19 * sizeof(glm::vec3)], float continous_time,float dt);
	void setFlag(bool x);
	void increasePositionCounter();
	int getPositionCounter();
	void SaveSpawnTime(float continuous_time);
	float getSpawnTime();

	void setHealth(int n_h);
	int getHealth();
	void reduceHealth();

	~Pirate();


	GeometryNode*			m_geometric_node;
	glm::vec3				m_position;
	glm::vec3				sphere_center = glm::vec3(-0.5957,11.683,-4.274)*0.09f; //multiplied by 0.09 due to the scaling of the model by 0.09
	float					sphere_radius = 12.87075*0.09; //multiplied by 0.09 due to the scaling of the model by 0.09
	glm::mat4				m_geometric_transformation_matrix;
	glm::mat4				m_geometric_transformation_normal_matrix;
	int						i = 1;
	float					angle;
	bool					flag = false;
	float					t1;
	float					t2;
	float					spawn_time = -1;

	int						health; //Number of hits he can take before dying

	float					velocity = 1.0; //Factor that alters pirate's speed
};
