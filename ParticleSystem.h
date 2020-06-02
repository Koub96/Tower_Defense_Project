#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"

class ParticleEmitter
{
protected:
	std::vector<glm::vec3>							m_particles_position;
	std::vector<glm::vec3>							m_particles_velocity;
	std::vector<float>								m_particles_life;

	// Store Particle Data on the GPU
	GLuint											m_vbo;
	GLuint											m_vao;

	float m_continous_time;

public:
	ParticleEmitter();
	~ParticleEmitter();
	bool										Init();
	void										Emit(glm::vec3 root);
	void										Update(float dt);
	void										Render();

	bool										active;
};

#endif
