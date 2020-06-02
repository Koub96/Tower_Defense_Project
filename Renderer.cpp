#include "Renderer.h"
#include "GeometryNode.h"
#include "Tools.h"
#include <algorithm>
#include "ShaderProgram.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OBJLoader.h"
#include <vector>
#include <iostream>
#include "TextureManager.h"
using namespace std;

Renderer::Renderer()
{		
	m_vbo_fbo_vertices = 0;
	m_vao_fbo = 0;
	m_fbo = 0;
	m_fbo_texture = 0;

	terrain = nullptr;
	road = nullptr;
	treasure_chest = nullptr;
	pirate_body = nullptr;
	pirate_arm = nullptr;
	pirate_left_foot = nullptr;
	pirate_right_foot = nullptr;
	tower = nullptr;
	selected_terrain = nullptr;
	selected_road = nullptr;
	cannonball = nullptr;
	trap = nullptr;

	selected_position = glm::vec3(-2, 0, 2);
	available_towers = INIT_AVAILABLE_TOWERS;
	available_traps = INIT_AVAILABLE_TRAPS;

	flagwave = false;
	spawn_time = 0;
	pirate_max_health = INIT_PIRATE_HEALTH;

	m_continous_time = 0.0;

	//Setting up the camera.
	m_camera_position = glm::vec3(0.0, 30.0, -24.0);
	m_camera_target_position = glm::vec3(0, 0, 0);
	m_camera_up_vector = glm::vec3(0, 1, 0);

}

bool Renderer::InitRenderingTechniques()
{
	bool initialized = true;

	// Geometry Rendering Program
	std::string vertex_shader_path = "../Data/Shaders/basic_rendering.vert";
	std::string fragment_shader_path = "../Data/Shaders/basic_rendering.frag";
	m_geometry_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_geometry_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = m_geometry_rendering_program.CreateProgram();
	m_geometry_rendering_program.LoadUniform("uniform_projection_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_view_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_model_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_normal_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_diffuse");
	m_geometry_rendering_program.LoadUniform("uniform_specular");
	m_geometry_rendering_program.LoadUniform("uniform_shininess");
	m_geometry_rendering_program.LoadUniform("uniform_has_texture");
	m_geometry_rendering_program.LoadUniform("diffuse_texture");
	m_geometry_rendering_program.LoadUniform("uniform_camera_position");
	m_geometry_rendering_program.LoadUniform("uniform_alpha");

	// Create and Compile Particle Shader
	vertex_shader_path = "../Data/Shaders/particle_rendering.vert";
	fragment_shader_path = "../Data/Shaders/particle_rendering.frag";
	m_particle_rendering_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_particle_rendering_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_particle_rendering_program.CreateProgram();
	m_particle_rendering_program.LoadUniform("uniform_view_matrix");
	m_particle_rendering_program.LoadUniform("uniform_projection_matrix");

	// Light Source Uniforms
	m_geometry_rendering_program.LoadUniform("uniform_light_projection_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_light_view_matrix");
	m_geometry_rendering_program.LoadUniform("uniform_light_position");
	m_geometry_rendering_program.LoadUniform("uniform_light_direction");
	m_geometry_rendering_program.LoadUniform("uniform_light_color");
	m_geometry_rendering_program.LoadUniform("uniform_light_umbra");
	m_geometry_rendering_program.LoadUniform("uniform_light_penumbra");
	m_geometry_rendering_program.LoadUniform("uniform_cast_shadows");
	m_geometry_rendering_program.LoadUniform("shadowmap_texture");

	// Post Processing Program
	vertex_shader_path = "../Data/Shaders/postproc.vert";
	fragment_shader_path = "../Data/Shaders/postproc.frag";
	m_postprocess_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_postprocess_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_postprocess_program.CreateProgram();
	m_postprocess_program.LoadUniform("uniform_texture");
	m_postprocess_program.LoadUniform("uniform_time");
	m_postprocess_program.LoadUniform("uniform_depth");
	m_postprocess_program.LoadUniform("uniform_projection_inverse_matrix");

	// Shadow mapping Program
	vertex_shader_path = "../Data/Shaders/shadow_map_rendering.vert";
	fragment_shader_path = "../Data/Shaders/shadow_map_rendering.frag";
	m_spot_light_shadow_map_program.LoadVertexShaderFromFile(vertex_shader_path.c_str());
	m_spot_light_shadow_map_program.LoadFragmentShaderFromFile(fragment_shader_path.c_str());
	initialized = initialized && m_spot_light_shadow_map_program.CreateProgram();
	m_spot_light_shadow_map_program.LoadUniform("uniform_projection_matrix");
	m_spot_light_shadow_map_program.LoadUniform("uniform_view_matrix");
	m_spot_light_shadow_map_program.LoadUniform("uniform_model_matrix");

	return initialized;
}
bool Renderer::InitIntermediateShaderBuffers()
{
	// generate texture handles 
	glGenTextures(1, &m_fbo_texture);
	glGenTextures(1, &m_fbo_depth_texture);

	// framebuffer to link everything together
	glGenFramebuffers(1, &m_fbo);

	return ResizeBuffers(m_screen_width, m_screen_height);
}
bool Renderer::ReloadShaders()
{
	bool reloaded = true;
	// rendering techniques
	reloaded = reloaded && m_geometry_rendering_program.ReloadProgram();
	// Reload Particle Shader
	reloaded = reloaded && m_particle_rendering_program.ReloadProgram();
	//Reload Shadow map Shader
	reloaded = reloaded && m_spot_light_shadow_map_program.ReloadProgram();

	return reloaded;
}
bool Renderer::ResizeBuffers(int width, int height)
{
	m_screen_width = width;
	m_screen_height = height;

	// texture
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_screen_width, m_screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// depth texture
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_screen_width, m_screen_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	// framebuffer to link to everything together
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_fbo_depth_texture, 0);

	GLenum status = Tools::CheckFramebufferStatus(m_fbo);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_projection_matrix = glm::perspective(glm::radians(60.f), width / (float)height, 0.1f, 1500.0f);
	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	return true;
}
bool Renderer::Init(int SCREEN_WIDTH, int SCREEN_HEIGHT)
{
	this->m_screen_width = SCREEN_WIDTH;
	this->m_screen_height = SCREEN_HEIGHT;

	// Initialize OpenGL functions

	//Set clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//This enables depth and stencil testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);// sets the value the depth buffer is set at, when we clear it

	// Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// open the viewport
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); //we set up our viewport

	bool techniques_initialization = InitRenderingTechniques();
	bool buffers_initialization = InitIntermediateShaderBuffers();
	bool lights_sources_initialization = InitLightSources();
	bool meshes_initialization = InitGeometricMeshes();
	bool items_initialization = InitCommonItems();

	//If there was any errors
	if (Tools::CheckGLError() != GL_NO_ERROR)
	{
		printf("Exiting with error at Renderer::Init\n");
		return false;
	}

	//If everything initialized
	return techniques_initialization && items_initialization && buffers_initialization && meshes_initialization && lights_sources_initialization;
}

void Renderer::Render()
{
	// Draw the geometry to the shadow maps
	RenderShadowMaps();
	
	// Draw the geometry
	RenderGeometry();	

	// Render to screen
	RenderToOutFB();

	
	/*
	GLenum error = Tools::CheckGLError();
	if (error != GL_NO_ERROR)
	{
		printf("Renderer:Draw GL Error\n");
		system("pause");
	}*/
	
}

void Renderer::CameraMoveForward(bool enable)
{
	m_camera_movement.x = (enable) ? 3.f : 0.f;
}
void Renderer::CameraMoveBackWard(bool enable)
{
	m_camera_movement.x = (enable) ? -3.f : 0.f;
}
void Renderer::CameraMoveLeft(bool enable)
{
	m_camera_movement.y = (enable) ? -3.f : 0.f;
}
void Renderer::CameraMoveRight(bool enable)
{
	m_camera_movement.y = (enable) ? 3.f : 0.f;
}
void Renderer::CameraLook(glm::vec2 lookDir)
{
	m_camera_look_angle_destination = glm::vec2(1, -1) * lookDir;
}

Renderer::~Renderer()
{
	// delete g_buffer
	glDeleteTextures(1, &m_fbo_texture);
	glDeleteFramebuffers(1, &m_fbo);

	// delete common data
	glDeleteVertexArrays(1, &m_vao_fbo);
	glDeleteBuffers(1, &m_vbo_fbo_vertices);

	delete terrain;
	delete road;
	delete treasure_chest;
	delete pirate_body;
	delete pirate_arm;
	delete pirate_left_foot;
	delete pirate_right_foot;
	delete tower;
	delete selected_terrain;
	delete selected_road;
	delete cannonball;
	delete trap;
}


//************************************//

bool Renderer::InitCommonItems()
{
	glGenVertexArrays(1, &m_vao_fbo);
	glBindVertexArray(m_vao_fbo);

	GLfloat fbo_vertices[] = {
		-1, -1,
		1, -1,
		-1, 1,
		1, 1,
	};

	glGenBuffers(1, &m_vbo_fbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_fbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindVertexArray(0);

	//Moving the whole pirate model to its initial position by manipulating the pirate_transformation_matrix which represents the object as a whole in world space coordinates.
	pirate_model_matrix = glm::translate(glm::mat4(1.f), glm::vec3(-18, 0.1, -18)) * glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));
	pirate_model_matrix2 = glm::translate(glm::mat4(1.f), glm::vec3(-18, 0.1, -18)) * glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));
	pirate_model_matrix3 = glm::translate(glm::mat4(1.f), glm::vec3(-18, 0.1, -18)) * glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));
	pirate_model_matrix4 = glm::translate(glm::mat4(1.f), glm::vec3(-18, 0.1, -18)) * glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));
	pirate_model_matrix5 = glm::translate(glm::mat4(1.f), glm::vec3(-18, 0.1, -18)) * glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));

	//Initializing Pirates vector
	Pirate p1;
	p1.setGeometricTransformationMatrix(pirate_model_matrix);
	p1.setNormalTransformationMatrix();
	p1.SaveSpawnTime(0.0);
	p1.setHealth(pirate_max_health);
	pirates.push_back(p1);

	Pirate p2;
	p2.setGeometricTransformationMatrix(pirate_model_matrix2);
	p2.setNormalTransformationMatrix();
	p2.SaveSpawnTime(4.0); //tweaking 1 sec extra
	p2.setHealth(pirate_max_health);
	pirates.push_back(p2);

	Pirate p3;
	p3.setGeometricTransformationMatrix(pirate_model_matrix3);
	p3.setNormalTransformationMatrix();
	p3.SaveSpawnTime(6.5); //tweaking 0.5 sec extra 
	p3.setHealth(pirate_max_health);
	pirates.push_back(p3);

	Pirate p4;
	p4.setGeometricTransformationMatrix(pirate_model_matrix4);
	p4.setNormalTransformationMatrix();
	p4.SaveSpawnTime(9.0);
	p4.setHealth(pirate_max_health);
	pirates.push_back(p4);

	Pirate p5;
	p5.setGeometricTransformationMatrix(pirate_model_matrix5);
	p5.setNormalTransformationMatrix();
	p5.SaveSpawnTime(12.0);
	p5.setHealth(pirate_max_health);
	pirates.push_back(p5);

	//Creating the Chests
	int number_of_chests = TOTAL_COINS / COIN_CAPACITY;

	glm::vec3 starting_pos_of_chests = glm::vec3(8, 0.1, -16);
	float offset = 2.2;
	glm::vec3 chest_pos; //each chest's position

	chest_pos.y = starting_pos_of_chests.y;
	chest_pos.z = starting_pos_of_chests.z; //Same y and z for all chests

	for (int i = 0; i < number_of_chests; i++)
	{
		chest_pos.x = starting_pos_of_chests.x + i * offset; //first one will have 0 offset, second will have 1* offset, 3rd 2*offset, etc

		Object new_chest;
		new_chest.setGeometricNode(treasure_chest);
		new_chest.setPosition(chest_pos);
		new_chest.coins = COIN_CAPACITY;

		chests.push_back(new_chest);
	}

	return true;
}

bool Renderer::InitLightSources()
{
	//Initializing the Lighting Parameteres for the spotlight.
	//1st spotlight.
	m_spotlight_node.SetPosition(glm::vec3(16, 35, 16));
	m_spotlight_node.SetTarget(glm::vec3(4.005, 12.634, -5.66336));
	m_spotlight_node.SetColor(40.0f * glm::vec3(255, 255, 251) / 255.f);
	m_spotlight_node.SetConeSize(50, 70);
	m_spotlight_node.CastShadow(true);

	//2nd spotlight.
	m_spotlight_node2.SetPosition(glm::vec3(16, 30, -16));
	m_spotlight_node2.SetTarget(glm::vec3(4.005, 12.634, -5.66336));
	m_spotlight_node2.SetColor(40.0f * glm::vec3(200, 200, 200) / 255.f);
	m_spotlight_node2.SetConeSize(73, 80);
	m_spotlight_node2.CastShadow(false);
	

	return true;
}

bool Renderer::InitGeometricMeshes()
{
	bool initialized = true;
	OBJLoader loader;
	// load terrain
	auto mesh = loader.load("../Data/Assets/Terrain/terrain.obj");
	if (mesh != nullptr)
	{
		terrain = new GeometryNode();
		terrain->Init(mesh);

	}
	else
		initialized = false;

	// load road
	mesh = loader.load("../Data/Assets/Terrain/road.obj");
	if (mesh != nullptr)
	{
		road = new GeometryNode();
		road->Init(mesh);
		//Setting the Geometry of the Road Object.
		roadTile.setGeometricNode(road);
	}
	else
		initialized = false;

	// load selected_terrain
	mesh = loader.load("../Data/Assets/Various/plane_green.obj");
	if (mesh != nullptr)
	{
		selected_terrain = new GeometryNode();
		selected_terrain->Init(mesh);
	}
	else
		initialized = false;

	// load selected_road
	mesh = loader.load("../Data/Assets/Various/plane_red.obj");
	if (mesh != nullptr)
	{
		selected_road = new GeometryNode();
		selected_road->Init(mesh);
	}
	else
		initialized = false;

	// load chest.
	mesh = loader.load("../Data/Assets/Treasure/treasure_chest.obj");
	if (mesh != nullptr)
	{
		treasure_chest = new GeometryNode();
		treasure_chest->Init(mesh);
	}
	else
		initialized = false;

	//load pirate body
	mesh = loader.load("../Data/Assets/Pirate/pirate_body.obj");
	if (mesh != nullptr)
	{
		pirate_body = new GeometryNode();
		pirate_body->Init(mesh);
		//Setting the Geometry of the Object.
		pirateBody.setGeometricNode(pirate_body);
		pirateBody.setGeometricTransformationMatrix(pirate_body_transformation_matrix);
		pirateBody.setNormalTransformationMatrix();
	}
	else
		initialized = false;
	
	//load pirate arm
	mesh = loader.load("../Data/Assets/Pirate/pirate_arm.obj");
	if (mesh != nullptr)
	{
		pirate_arm = new GeometryNode();
		pirate_arm->Init(mesh);
		//Setting the Geometry of the Object.
		pirateArm.setGeometricNode(pirate_arm);
		pirateArm.setGeometricTransformationMatrix(pirate_arm_transformation_matrix);
		pirateArm.setNormalTransformationMatrix();
	}
	else
		initialized = false;
	
	//load pirate left foot
	mesh = loader.load("../Data/Assets/Pirate/pirate_left_foot.obj");
	if (mesh != nullptr)
	{
		pirate_left_foot = new GeometryNode();
		pirate_left_foot->Init(mesh);
		//Setting the Geometry of the Object.
		pirateLeftFoot.setGeometricNode(pirate_left_foot);
		pirateLeftFoot.setGeometricTransformationMatrix(pirate_left_foot_transformation_matrix);
		pirateLeftFoot.setNormalTransformationMatrix();
	}
	else
		initialized = false;
	
	//load pirate right foot
	mesh = loader.load("../Data/Assets/Pirate/pirate_right_foot.obj");
	if (mesh != nullptr)
	{
		pirate_right_foot = new GeometryNode();
		pirate_right_foot->Init(mesh);
		//Setting the Geometry of the Object.
		pirateRightFoot.setGeometricNode(pirate_right_foot);
		pirateRightFoot.setGeometricTransformationMatrix(pirate_right_foot_transformation_matrix);
		pirateRightFoot.setNormalTransformationMatrix();
	}
	else
		initialized = false;

	//load tower
	mesh = loader.load("../Data/Assets/MedievalTower/tower.obj");
	if (mesh != nullptr)
	{
		tower = new GeometryNode();
		tower->Init(mesh);
	}
	else
		initialized = false;


	//load cannon ball
	mesh = loader.load("../Data/Assets/Various/cannonball.obj");
	if (mesh != nullptr)
	{
		cannonball = new GeometryNode();
		cannonball->Init(mesh);
	}
	else
		initialized = false;

	
	//load trap
	mesh = loader.load("../Data/Assets/Terrain/lava.obj");
	if (mesh != nullptr)
	{
		trap = new GeometryNode();
		trap->Init(mesh);
	}
	else
		initialized = false;

	//Initialize Particle Emitters
	m_particle_emitter.Init();
		
	//We save the parts of the pirate to use it later at Renderer.
	parts_of_pirate[0] = pirateBody;
	parts_of_pirate[1] = pirateArm;
	parts_of_pirate[2] = pirateLeftFoot;
	parts_of_pirate[3] = pirateRightFoot;

	return initialized;
}

void Renderer::Update(float dt)
{
	float movement_speed = 2.0f;
	glm::vec3 direction = glm::normalize(m_camera_target_position - m_camera_position);

	m_camera_position += m_camera_movement.x * movement_speed * direction * dt;
	m_camera_target_position += m_camera_movement.x * movement_speed * direction * dt;

	glm::vec3 right = glm::normalize(glm::cross(direction, m_camera_up_vector));
	m_camera_position += m_camera_movement.y * movement_speed * right * dt;
	m_camera_target_position += m_camera_movement.y * movement_speed * right * dt;

	glm::mat4 rotation = glm::mat4(1.0f);
	float angular_speed = glm::pi<float>() * 0.0025f;

	rotation *= glm::rotate(glm::mat4(1.0), m_camera_look_angle_destination.y * angular_speed, right);
	rotation *= glm::rotate(glm::mat4(1.0), -m_camera_look_angle_destination.x * angular_speed, m_camera_up_vector);
	m_camera_look_angle_destination = glm::vec2(0);

	direction = rotation * glm::vec4(direction, 0);
	float distance = glm::distance(m_camera_position, m_camera_target_position);
	m_camera_target_position = m_camera_position + direction * distance;

	m_view_matrix = glm::lookAt(m_camera_position, m_camera_target_position, m_camera_up_vector);

	m_continous_time += dt;
	my_dt = dt;

	//Check if game is over
	if (getTotalCoins() == 0) game_over = true;

	//Tile selection
	//Navigating selected tile (selected_position is constantly updated)
	//and holds the position the player has navigated to place/remove towers/traps
	if (draw_selection) //if Q has been pressed
	{
		if (move_right)
		{
			if (selected_position.x >= -16) selected_position.x -= 4;
			move_right = false;
		}
		else if (move_left)
		{
			if (selected_position.x <= 16) selected_position.x += 4;
			move_left = false;
		}
		else if (move_up)
		{
			if (selected_position.z <= 16) selected_position.z += 4;
			move_up = false;
		}
		else if (move_down)
		{
			if (selected_position.z >= -16) selected_position.z -= 4;
			move_down = false;
		}

		//If selected terrain is a road tile
		int max_i = sizeof(pos_of_roads) / sizeof(pos_of_roads[0]);
		selected_grass = true;
		selected_position.y = 0.05; // to match road's y-coordinate

		for (int i = 0; i <= max_i; i++)
			if (pos_of_roads[i] == selected_position)
				selected_grass = false;

		if (selected_grass)
			selected_tile.setGeometricNode(selected_terrain);//make selection green
		else
			selected_tile.setGeometricNode(selected_road); //make selection red
	}//if Q pressed END

	//Helping vectors, iterators and counters
	std::vector<Object>::iterator o_it; //iterator for Objects
	std::vector<Pirate>::iterator p_it; //iterator for Pirates
	std::vector<int>::iterator i_it; //iterator for integers
	int i;
	int i2;

	bool expiring = false; //if a trap is going to expire in this frame

	//Tower firing
	float min_pirate_dist;
	float dist;
	glm::vec3 min_pirate_pos;

	i = 0;
	for (o_it = towers.begin(); o_it != towers.end(); ++o_it, ++i) //for every tower
	{
		min_pirate_dist = 100.0; //reseting min distance

		i2 = 0;
		for (p_it = pirates.begin(); p_it != pirates.end(); ++p_it, ++i2) //for every active enemy
		{
			if (pirates[i2].getPosition() == glm::vec3(0)) continue;

			//finding closest enemy of this tower
			dist = glm::length(towers[i].getPosition() - pirates[i2].getPosition());

			if (dist < min_pirate_dist)
			{
				min_pirate_dist = dist;
				min_pirate_pos = pirates[i2].getPosition(); //keeping position of closest enemy
			}
		}

		//if the closest enemy is in range, fire cannonball towards him
		if (min_pirate_dist <= TOWER_FIRE_RANGE)
		{
			if ((float)m_continous_time - (towers[i].used) >= TOWER_FIRE_FREQ)
			{
				Fire(towers[i], min_pirate_pos);
				towers[i].used = (float)m_continous_time;
			}
		}
	}


	//Collision Detection

	//Cannonballs - Pirates
	i = 0;
	glm::vec3 proj_pos;
	glm::vec3 pir_pos;
	bool removed = false; //if we removed a cannonball on previous loop
	for (o_it = projectiles.begin(); o_it != projectiles.end(); ++o_it, ++i)//for every cannonball
	{
		if (projectiles[i].getPosition().y <= 0.3)//if projectile hits the ground
		{
			projectiles.erase(o_it);
			projectiles.shrink_to_fit();
			break;;
		}

		i2 = 0;
		for (p_it = pirates.begin(); p_it != pirates.end(); ++p_it, ++i2) //for every active enemy
		{
			proj_pos = projectiles[i].getPosition();
			pir_pos = pirates[i2].getPosition();

			if (testCollision(proj_pos, pir_pos, 0.1, 12.87075 * 0.09))
			{
				//reduce pirate's health
				pirates[i2].reduceHealth();

				if (pirates[i2].getHealth() <= 0)
				{
					pirates.erase(p_it);//remove pirate if dead
					pirates.shrink_to_fit();
				}

				projectiles.erase(o_it);
				projectiles.shrink_to_fit();
				removed = true;
				break; //stop at first pirate hit
			}
		}
		if (removed) break;
	}

	//Traps - Pirates
	i = 0;
	for (o_it = traps.begin(); o_it != traps.end(); ++o_it, ++i)//for every trap
	{
		//remove trap if expired
		if ((float)m_continous_time - traps[i].used > TRAP_ACTIVE_DURATION)
		{
			traps.erase(o_it);
			traps.shrink_to_fit();
			expiring = true;
		}

		i2 = 0;
		for (p_it = pirates.begin(); p_it != pirates.end(); ++p_it, ++i2) //for every active enemy
		{
			if (expiring)
			{
				pirates[i2].velocity = PIRATE_NORMAL_SPEED; // restore the movement speed of pirates that are on this trap
				continue;
			}

			if (testCollision(pirates[i2].getPosition(), traps[i].getPosition(), 2.0, 0.0)) //if in same tile
			{
				//set lower speed
				pirates[i2].velocity = PIRATE_SLOWED_SPEED;
			}
			else
			{
				//set normal speed
				pirates[i2].velocity = PIRATE_NORMAL_SPEED;
			}
		}
		if (expiring) break;
	}

	//Chests - Pirates
	i = 0;
	for (o_it = chests.begin(); o_it != chests.end(); ++o_it, ++i)//for every chest
	{
		i2 = 0;
		for (p_it = pirates.begin(); p_it != pirates.end(); ++p_it, ++i2) //for every active enemy
		{
			if (testCollision(chests[i].getPosition(), pirates[i2].getPosition(), 12.0284 * 0.12, 12.87075 * 0.09))
			{
				chests[i].ReduceCoins();

				cout << "\n";
				cout << "You lost 10 coins! Coins left:  " << getTotalCoins() << "\n";//msg

				// Emit Particles
				m_particle_emitter.Emit(chests[i].getPosition());

				pirates.erase(p_it); //remove pirate
				pirates.shrink_to_fit();
				break; // only 1 pirate per frame can steal coins
			}
		}
		//remove chest if empty
		if (chests[i].coins == 0)
		{
			chests.erase(o_it);
			chests.shrink_to_fit();
			break;//no need to check other chests in this frame
		}
	}

	//Update Particles
	if(m_particle_emitter.active) m_particle_emitter.Update(dt);
}


void Renderer::RenderGeometry()
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Bind the Intermediate framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glViewport(0, 0, m_screen_width, m_screen_height);
	GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawbuffers);

	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// render only triangles
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Bind the shader program
	m_geometry_rendering_program.Bind();

	// pass the camera properties
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
	glUniform3f(m_geometry_rendering_program["uniform_camera_position"], m_camera_position.x, m_camera_position.y, m_camera_position.z);

	// pass the light source parameters to uniforms
	glm::vec3 light_position = m_spotlight_node.GetPosition();
	glm::vec3 light_direction = m_spotlight_node.GetDirection();
	glm::vec3 light_color = m_spotlight_node.GetColor();
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_light_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));
	glUniform3f(m_geometry_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
	glUniform1f(m_geometry_rendering_program["uniform_light_umbra"], m_spotlight_node.GetUmbra());
	glUniform1f(m_geometry_rendering_program["uniform_light_penumbra"], m_spotlight_node.GetPenumbra());
	glUniform1i(m_geometry_rendering_program["uniform_cast_shadows"], (m_spotlight_node.GetCastShadowsStatus()) ? 1 : 0);
	// Set the sampler2D uniform to use texture unit 1
	glUniform1i(m_geometry_rendering_program["shadowmap_texture"], 1);
	// Bind the shadow map texture to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, (m_spotlight_node.GetCastShadowsStatus()) ? m_spotlight_node.GetShadowMapDepthTexture() : 0);


	//passing the second light source parameters.
	light_position = m_spotlight_node2.GetPosition();
	light_direction = m_spotlight_node2.GetDirection();
	light_color = m_spotlight_node2.GetColor();
	glUniform3f(m_geometry_rendering_program["uniform_light_position"], light_position.x, light_position.y, light_position.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_direction"], light_direction.x, light_direction.y, light_direction.z);
	glUniform3f(m_geometry_rendering_program["uniform_light_color"], light_color.x, light_color.y, light_color.z);
	glUniform1f(m_geometry_rendering_program["uniform_light_umbra"], m_spotlight_node2.GetUmbra());
	glUniform1f(m_geometry_rendering_program["uniform_light_penumbra"], m_spotlight_node2.GetPenumbra());

	// Enable Texture Unit 0
	glUniform1i(m_geometry_rendering_program["uniform_diffuse_texture"], 0);
	glActiveTexture(GL_TEXTURE0);

	//Static Drawings

	//Drawing the terrain

	//we dont have an object instance of terrain, so we draw it directly
	glBindVertexArray(terrain->m_vao);
	terrain_transformation_matrix = glm::scale(glm::mat4(1.0), glm::vec3(20));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(terrain_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(terrain_transformation_normal_matrix));
	for (int j = 0; j < terrain->parts.size(); j++)
	{
		glm::vec3 diffuseColor = terrain->parts[j].diffuseColor;
		glm::vec3 specularColor = terrain->parts[j].specularColor;
		float shininess = terrain->parts[j].shininess;
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (terrain->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glUniform1f(m_geometry_rendering_program["uniform_alpha"], 1.0);
		glBindTexture(GL_TEXTURE_2D, terrain->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, terrain->parts[j].start_offset, terrain->parts[j].count);
	}

	// Drawing the road
	DrawRoadPath(glm::vec3(-18, 0.05, -18));


	//Dynamic Drawings

	std::vector<Object>::iterator o_it;//iterator for Objects
	int i2 = 0;


	//Drawing the chests
	i2 = 0;
	for (o_it = chests.begin(); o_it != chests.end(); ++o_it, ++i2)
	{
		RenderObject(&chests[i2], chests[i2].getPosition(), glm::vec3(0), glm::vec3(0.09), 1.0);
	}


	//Drawing the pirates
	
	//We start the dynamic spawn of waves by 20 seconds.
	spawn_time += my_dt;
	if (spawn_time > PIRATE_WAVE_SPAWN_TIME && flagwave == false) //if it's time to spawn a new wave
	{
		spawn_time = 0; //start the timer from the start
		flagwave = true; //activate the spawn
	}

	if (flagwave) 
	{
		interval = 0;
		for (int i = 0; i < 5; i++) //create the next 5 enemies
		{
			glm::mat4 model_matrix;
			model_matrix = glm::translate(glm::mat4(1.f), glm::vec3(-18, 0.1, -18)) * glm::rotate(glm::mat4(1.f), glm::radians(180.0f), glm::vec3(0, 1, 0));

			Pirate p;
			p.velocity = PIRATE_NORMAL_SPEED;
			p.setGeometricTransformationMatrix(model_matrix);
			p.setNormalTransformationMatrix();
			p.setHealth(pirate_max_health);
			p.SaveSpawnTime(m_continous_time + interval); //we add an interval so that they dont ger drawn at the same time
			interval += 3; //the next enemy should spawn after an added interval (every 3 seconds)

			pirates.push_back(p); //putting the enemy in the vector with the active enemies.
		}
		flagwave = false;
	}

	int i = 0;
	std::vector<Pirate>::iterator it;
	for (it = pirates.begin(); it != pirates.end(); ++it, ++i) //for every active enemy
	{
		glm::mat4 model_matrix = pirates[i].getGeometricTransformationMatrix();

		if (m_continous_time >= pirates[i].getSpawnTime())// if the time to be rendered has passed, render and animate him.
		{
			pirates[i].movePirate(model_matrix, pos_of_roads, m_continous_time, my_dt);
			DrawPirate(model_matrix);
		}
		pirates[i].setGeometricTransformationMatrix(model_matrix); //updating the model and normal matrix
		pirates[i].setNormalTransformationMatrix();
	}


	//Draw the Towers
	i2 = 0;
	for(o_it = towers.begin(); o_it != towers.end(); ++o_it, ++i2)
	{
		RenderObject(&towers[i2], towers[i2].getPosition(), glm::vec3(0.0), glm::vec3(0.4), 1.0);
	}

	 
	//Draw / Animate all the projectiles
	i2 = 0;
	for (o_it = projectiles.begin(); o_it != projectiles.end(); ++o_it, ++i2)
	{
		projectiles[i2].setPosition( glm::mix(projectiles[i2].getPosition(), projectiles[i2].getDirection(), glm::fract(0.09)) );
	
		RenderObject(&projectiles[i2], projectiles[i2].getPosition(), glm::vec3(0.0), glm::vec3(0.1), 1.0);
	}


	//Draw the Traps
	i2 = 0;
	for (o_it = traps.begin(); o_it != traps.end(); ++o_it, ++i2)
	{
		RenderObject(&traps[i2], traps[i2].getPosition(), glm::vec3(0, 0.01, 0), glm::vec3(2), 1.0);
	}

	//Draw selection
	if (draw_selection) RenderObject(&selected_tile, selected_position, glm::vec3(0, 0.02, 0), glm::vec3(2), 0.6);


	// unbind the vao
	glBindVertexArray(0);
	// unbind the shader program
	m_geometry_rendering_program.Unbind();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	// Render Particles
	if (m_particle_emitter.active)
	{
		m_particle_rendering_program.Bind();
		glUniformMatrix4fv(m_particle_rendering_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_projection_matrix));
		glUniformMatrix4fv(m_particle_rendering_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_view_matrix));
		m_particle_emitter.Render();
	}

	//glDisable(GL_DEPTH_TEST);
	glPointSize(1.0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}


//************************************//

//Drawing Methods
void Renderer::RenderShadowMaps()
{
	// if the light source casts shadows
	if (m_spotlight_node.GetCastShadowsStatus())
	{
		int m_depth_texture_resolution = m_spotlight_node.GetShadowMapResolution();

		// bind the framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_spotlight_node.GetShadowMapFBO());
		glViewport(0, 0, m_depth_texture_resolution, m_depth_texture_resolution);
		GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawbuffers);

		// clear depth buffer
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		// Bind the shadow mapping program
		m_spot_light_shadow_map_program.Bind();

		// pass the projection and view matrix to the uniforms
		glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_projection_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetProjectionMatrix()));
		glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_view_matrix"], 1, GL_FALSE, glm::value_ptr(m_spotlight_node.GetViewMatrix()));

		/*
		DrawGeometryNodeToShadowMap(pirate_body, pirate_model_matrix*glm::scale(glm::mat4(1.0), glm::vec3(0.22)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix)))));
		DrawGeometryNodeToShadowMap(pirate_arm, &pirate_model_matrix*glm::scale(glm::mat4(1.0), glm::vec3(0.44)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix)))));
		DrawGeometryNodeToShadowMap(pirate_left_foot, pirate_model_matrix, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix)))));
		DrawGeometryNodeToShadowMap(pirate_right_foot, pirate_model_matrix, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix)))));

		DrawGeometryNodeToShadowMap(pirate_body, pirate_model_matrix2*glm::scale(glm::mat4(1.0), glm::vec3(0.22)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix2)))));
		DrawGeometryNodeToShadowMap(pirate_arm, pirate_model_matrix2*glm::scale(glm::mat4(1.0), glm::vec3(0.44)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix2)))));
		DrawGeometryNodeToShadowMap(pirate_left_foot, pirate_model_matrix2, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix2)))));
		DrawGeometryNodeToShadowMap(pirate_right_foot, pirate_model_matrix2, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix2)))));

		DrawGeometryNodeToShadowMap(pirate_body, pirate_model_matrix3*glm::scale(glm::mat4(1.0), glm::vec3(0.22)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix3)))));
		DrawGeometryNodeToShadowMap(pirate_arm, pirate_model_matrix3*glm::scale(glm::mat4(1.0), glm::vec3(0.44)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix3)))));
		DrawGeometryNodeToShadowMap(pirate_left_foot, pirate_model_matrix3, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix3)))));
		DrawGeometryNodeToShadowMap(pirate_right_foot, pirate_model_matrix3, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix3)))));

		DrawGeometryNodeToShadowMap(pirate_body, pirate_model_matrix4*glm::scale(glm::mat4(1.0), glm::vec3(0.22)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix4)))));
		DrawGeometryNodeToShadowMap(pirate_arm, pirate_model_matrix4*glm::scale(glm::mat4(1.0), glm::vec3(0.44)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix4)))));
		DrawGeometryNodeToShadowMap(pirate_left_foot, pirate_model_matrix4, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix4)))));
		DrawGeometryNodeToShadowMap(pirate_right_foot, pirate_model_matrix4, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix4)))));

		DrawGeometryNodeToShadowMap(pirate_body, pirate_model_matrix5*glm::scale(glm::mat4(1.0), glm::vec3(0.22)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix5)))));
		DrawGeometryNodeToShadowMap(pirate_arm, pirate_model_matrix5*glm::scale(glm::mat4(1.0), glm::vec3(0.44)), glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix5)))));
		DrawGeometryNodeToShadowMap(pirate_left_foot, pirate_model_matrix5, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix5)))));
		DrawGeometryNodeToShadowMap(pirate_right_foot, pirate_model_matrix5, glm::mat4(glm::transpose(glm::inverse(glm::mat3(pirate_model_matrix5)))));



		for (Pirate p : pirates) {
			DrawGeometryNodeToShadowMap(pirate_body, p.getGeometricTransformationMatrix()*glm::scale(glm::mat4(1.0), glm::vec3(0.22)), p.getGeometricTransformationNormalMatrix());
			DrawGeometryNodeToShadowMap(pirate_arm, p.getGeometricTransformationMatrix()*glm::scale(glm::mat4(1.0), glm::vec3(0.44)), p.getGeometricTransformationNormalMatrix());
			DrawGeometryNodeToShadowMap(pirate_left_foot, p.getGeometricTransformationMatrix(), p.getGeometricTransformationNormalMatrix());
			DrawGeometryNodeToShadowMap(pirate_right_foot, p.getGeometricTransformationMatrix(), p.getGeometricTransformationNormalMatrix());

		}
		*/

		/*
		for (Object t : towers) 
		{
			DrawGeometryNodeToShadowMap(t.getGeometricNode(), t.getTransformationMatrix(), t.getNormalMatrix());
		}
		*/

		int i2 = 0;
		std::vector<Object>::iterator o_it;
		for (o_it = towers.begin(); o_it != towers.end(); ++o_it, ++i2)
		{
			DrawGeometryNodeToShadowMap(towers[i2].getGeometricNode(), towers[i2].getTransformationMatrix(), towers[i2].getNormalMatrix());
		}
			   
		glBindVertexArray(0);

		// Unbind shadow mapping program
		m_spot_light_shadow_map_program.Unbind();

		glDisable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

void Renderer::DrawGeometryNodeToShadowMap(GeometryNode* node, glm::mat4* model_matrix, glm::mat4* normal_matrix)
{
	glBindVertexArray(node->m_vao);
	glUniformMatrix4fv(m_spot_light_shadow_map_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(*model_matrix));
	for (int j = 0; j < node->parts.size(); j++)
	{
		glDrawArrays(GL_TRIANGLES, node->parts[j].start_offset, node->parts[j].count);
	}
}

void Renderer::RenderToOutFB()
{
	// Bind the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, m_screen_width, m_screen_height);

	// clear the color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// disable depth testing and blending
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND);

	// bind the post processing program
	m_postprocess_program.Bind();

	glBindVertexArray(m_vao_fbo);
	// Bind the intermediate color image to texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
	glUniform1i(m_postprocess_program["uniform_texture"], 0);
	// Bind the intermediate depth buffer to texture unit 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_fbo_depth_texture);
	glUniform1i(m_postprocess_program["uniform_depth"], 1);

	glUniform1f(m_postprocess_program["uniform_time"], m_continous_time);
	glm::mat4 projection_inverse_matrix = glm::inverse(m_projection_matrix);
	glUniformMatrix4fv(m_postprocess_program["uniform_projection_inverse_matrix"], 1, GL_FALSE, glm::value_ptr(projection_inverse_matrix));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindVertexArray(0);

	// Unbind the post processing program
	m_postprocess_program.Unbind();
}


//Helping method that draws the road path
void Renderer::DrawRoadPath(glm::vec3 initial_pos)
{
	//drawing first tile
	RenderObject(&roadTile, initial_pos, glm::vec3(0, 0, 0), glm::vec3(2), 1.0);

	//we save the position of the first road as our starting point to draw the rest.
	pos_of_roads[0] = initial_pos;

	//z
	for (int i = 0; i < 5; i++)
	{
		//We use the position of the i road to create the i+1 and save it to the array.
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(0, 0, 4), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(0, 0, 4); //the (0,0,4) is the offset we need to add to draw the next road to its proper position.
	}
	//x
	for (int i = 5; i < 7; i++) {
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(4, 0, 0), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(4, 0, 0);
	}
	//-z
	for (int i = 7; i < 10; i++)
	{
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(0, 0, -4), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(0, 0, -4);
	}
	//x
	for (int i = 10; i < 12; i++) {
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(4, 0, 0), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(4, 0, 0);
	}
	//z
	for (int i = 12; i < 17; i++)
	{
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(0, 0, 4), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(0, 0, 4);
	}
	//x
	for (int i = 17; i < 20; i++) {
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(4, 0, 0), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(4, 0, 0);
	}
	//-z
	for (int i = 20; i < 26; i++)
	{
		RenderObject(&roadTile, pos_of_roads[i], glm::vec3(0, 0, -4), glm::vec3(2), 1.0);
		pos_of_roads[i + 1] = pos_of_roads[i] + glm::vec3(0, 0, -4);
	}
	//2 extra tiles that are not drawn, just exist to guide the pirates to the chests
	pos_of_roads[26] = pos_of_roads[25] + glm::vec3(0, 0, -5);
	pos_of_roads[27] = pos_of_roads[26] + glm::vec3(0, 0, -5);
}

//Helping method that calls the method that draws the Pirate. 
void Renderer::DrawPirate(glm::mat4 &pirate_model_matrix) {
	//We draw the enemy.

	//Body
	RenderPiratePartsGeometryNode(parts_of_pirate[0].getGeometricNode(), parts_of_pirate[0].getGeometricTransformationMatrix(), parts_of_pirate[0].getGeometricTransformationNormalMatrix(), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 0, pirate_model_matrix);
	//Arm
	RenderPiratePartsGeometryNode(parts_of_pirate[1].getGeometricNode(), parts_of_pirate[1].getGeometricTransformationMatrix(), parts_of_pirate[1].getGeometricTransformationNormalMatrix(), glm::vec3(0.4, 0.85, 0), glm::vec3(0, 0, 0), 1, pirate_model_matrix);
	//left foot
	RenderPiratePartsGeometryNode(parts_of_pirate[2].getGeometricNode(), parts_of_pirate[2].getGeometricTransformationMatrix(), parts_of_pirate[2].getGeometricTransformationNormalMatrix(), glm::vec3(0.2, 0.1, 0), glm::vec3(0, 0, 0), 2, pirate_model_matrix);
	//Right foot
	RenderPiratePartsGeometryNode(parts_of_pirate[3].getGeometricNode(), parts_of_pirate[3].getGeometricTransformationMatrix(), parts_of_pirate[3].getGeometricTransformationNormalMatrix(), glm::vec3(-0.2, 0.1, 0), glm::vec3(0, 0, 0), 3, pirate_model_matrix);
}

//Method that renders the geometry that is given from a starting initial_pos.This is a helping method specifically for the pirate parts objects.
void Renderer::RenderPiratePartsGeometryNode(GeometryNode* g, glm::mat4 g_transformation_matrix, glm::mat4 g_normal_matrix, glm::vec3 initial_pos, glm::vec3 offset, int i, glm::mat4& model_matrix) {

	glBindVertexArray(g->m_vao);
	if (i == 1) //Arm
	{
		g_transformation_matrix = model_matrix * glm::translate(glm::mat4(1.f), initial_pos + offset) * glm::scale(glm::mat4(1.0), glm::vec3(0.09)) * glm::rotate(glm::mat4(1.0f), 0.8f * sin(4.0f * m_continous_time), glm::vec3(1, 0, 0));
		g_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(g_transformation_matrix))));
	}
	else if (i == 2) //Left Foot
	{
		g_transformation_matrix = model_matrix * glm::translate(glm::mat4(1.f), initial_pos + offset) * glm::scale(glm::mat4(1.0), glm::vec3(0.09)) * glm::rotate(glm::mat4(1.0f), 0.4f * sin(3.5f * m_continous_time), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(-10.f), glm::vec3(0, 1, 0));
		g_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(g_transformation_matrix))));
	}
	else if (i == 3) //Right Foot
	{
		g_transformation_matrix = model_matrix * glm::translate(glm::mat4(1.f), initial_pos + offset) * glm::scale(glm::mat4(1.0), glm::vec3(0.09)) * glm::rotate(glm::mat4(1.0f), 0.4f * sin(-3.5f * m_continous_time), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(10.f), glm::vec3(0, 1, 0));
		g_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(g_transformation_matrix))));
	}
	else //0 is for body
	{
		g_transformation_matrix = model_matrix * glm::translate(glm::mat4(1.f), initial_pos + offset) * glm::scale(glm::mat4(1.0), glm::vec3(0.09));
		g_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(g_transformation_matrix))));
	}

	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(g_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(g_normal_matrix));
	for (int j = 0; j < g->parts.size(); j++)
	{
		glm::vec3 diffuseColor = g->parts[j].diffuseColor;
		glm::vec3 specularColor = g->parts[j].specularColor;
		float shininess = g->parts[j].shininess;
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (g->parts[j].textureID > 0) ? 1.0f : 0.0f);
		glUniform1f(m_geometry_rendering_program["uniform_alpha"], 1.0);
		glBindTexture(GL_TEXTURE_2D, g->parts[j].textureID);
		glDrawArrays(GL_TRIANGLES, g->parts[j].start_offset, g->parts[j].count);
	}

}

//Render an Object
//Note: Objects have a Position field, but we specify a new position here for them to be drawn
void Renderer::RenderObject(Object* o, glm::vec3 position, glm::vec3 offset, glm::vec3 scaling, float alpha)
{

	GeometryNode* g = o->getGeometricNode();
	glm::mat4* g_transformation_matrix = o->getTransformationMatrix();
	glm::mat4* g_normal_matrix = o->getNormalMatrix();


	glBindVertexArray(g->m_vao);
	*g_transformation_matrix = glm::translate(glm::mat4(1.f), position + offset) * glm::scale(glm::mat4(1.0), scaling);
	*g_normal_matrix = glm::mat4(glm::transpose(glm::inverse(glm::mat3(*g_transformation_matrix))));


	glUniformMatrix4fv(m_geometry_rendering_program["uniform_model_matrix"], 1, GL_FALSE, glm::value_ptr(*g_transformation_matrix));
	glUniformMatrix4fv(m_geometry_rendering_program["uniform_normal_matrix"], 1, GL_FALSE, glm::value_ptr(*g_normal_matrix));

	for (int j = 0; j < g->parts.size(); j++)
	{
		glm::vec3 diffuseColor = g->parts[j].diffuseColor;
		glm::vec3 specularColor = g->parts[j].specularColor;
		float shininess = g->parts[j].shininess;
		glUniform3f(m_geometry_rendering_program["uniform_diffuse"], diffuseColor.r, diffuseColor.g, diffuseColor.b);
		glUniform3f(m_geometry_rendering_program["uniform_specular"], specularColor.r, specularColor.g, specularColor.b);
		glUniform1f(m_geometry_rendering_program["uniform_shininess"], shininess);
		glUniform1f(m_geometry_rendering_program["uniform_has_texture"], (g->parts[j].textureID > 0) ? 1.0f : 0.0f);

		glUniform1f(m_geometry_rendering_program["uniform_alpha"], alpha);//setting transparency of object

		glBindTexture(GL_TEXTURE_2D, g->parts[j].textureID);

		glDrawArrays(GL_TRIANGLES, g->parts[j].start_offset, g->parts[j].count);
	}
}


//Gameplay Methods


//Methods Called by main

//Adds tower at selected position
void Renderer::AddTower()
{
	//Add new tower if allowed in that location
	if (selected_grass)
	{
		if (available_towers > 0)
		{
			//create new tower
			Object new_tower;
			new_tower.setGeometricNode(tower);
			new_tower.setPosition(selected_position);

			//add to tower vector
			towers.push_back(new_tower);
			available_towers--; // decreasing value of available towers
		}
		else
		{
			cout << "\n";
			cout << "No more available towers yet!\n"; //msg
		}
	}
	else
	{
		cout << "\n";
		cout << "You can't place a tower there.\n"; //msg
	}	
}

//Adds trap at selected position
void Renderer::AddTrap()
{
	//Add new trap (if asked) and if allowed in that location
	//Traps are allowed only on road tiles

	if (!selected_grass) //if not on grass, it's on a road tile
	{
		if (available_traps > 0)
		{
			//Add to traps vector
			Object new_trap;
			new_trap.setGeometricNode(trap);
			new_trap.setPosition(selected_position);
			new_trap.used = (float)m_continous_time;

			traps.push_back(new_trap);

			available_traps--;
		}
		else
		{
			cout << "\n";
			cout << "No more available traps yet!\n"; //msg
		}
	}
	else
	{
		cout << "\n";
		cout << "You can't place a trap there.\n"; //msg
	}
}

//Returns if there is a tower placed at the current selected position
bool Renderer::TowerAtSelectedPos()
{
	int i = 0;
	std::vector<Object>::iterator it;
	for (it = towers.begin(); it != towers.end(); ++it, ++i)
	{
		if (towers[i].getPosition() == selected_position)
			return true;
	}
	return false;
}

// Returns if there is a trap placed at the current selected position
bool Renderer::TrapAtSelectedPos()
{
	int i = 0;
	std::vector<Object>::iterator it;
	for (it = traps.begin(); it != traps.end(); ++it, ++i)
	{
		if (traps[i].getPosition() == selected_position)
			return true;
	}
	return false;
}

//Removes the tower that exists at current selected position
void Renderer::RemoveTower()
{
	int i = 0;
	std::vector<Object>::iterator it;
	for (it = towers.begin(); it != towers.end(); ++it, ++i)
	{
		if (towers[i].getPosition() == selected_position)
		{
			towers.erase(it);
			towers.shrink_to_fit();
			available_towers++; //take tower back to replace it later
			break;
		}
	}
}

//Gives an aditional tower
void Renderer::GiveNewTower()
{
	available_towers++ ;
	cout << "\n";
	cout << "You gained a new tower! Total: " << available_towers << "\n";//msg
}

//Gives an additional trap (up to 5)
void Renderer::GiveNewTrap()
{
	if (available_traps < MAX_AVAILABLE_TRAPS)
	{
		available_traps++;
		cout << "\n";
		cout << "You gained a new trap! Total: " << available_traps << "\n";//msg
	}
}

//Levels-Up Pirates (+1 health)
void Renderer::IncreasePirateHealth()
{
	pirate_max_health++;
	cout << "\n";
	cout <<"Pirate Health has increased to " << pirate_max_health << "!\n";//msg
}

//Resets the selection's position
void Renderer::ResetSelectedPos()
{
	selected_position = glm::vec3(-2, 0, 2);
}


//Returns current number of player's coins
int Renderer::getTotalCoins()
{
	int total=0;
	int i = 0;
	std::vector<Object>::iterator o_it;
	for(o_it = chests.begin(); o_it != chests.end(); ++o_it, ++i)//for every chest
	{
		total += chests[i].coins;
	}
	return total;
}

//Fires a cannonball from tower t to direction dir
void Renderer::Fire(Object t, glm::vec3 dir)
{
	Object new_proj;
	new_proj.setGeometricNode(cannonball);
	new_proj.setPosition(t.getPosition()+glm::vec3(0, 3.5 ,0)); //initial position is tower's position and a little higher
	new_proj.setDirection(dir);

	projectiles.push_back(new_proj); //add to projectiles vector
}

//Tests wether or not 2 objects are collided based on their hit-sphere's radii
bool Renderer::testCollision(glm::vec3 pos1, glm::vec3 pos2, float rad1, float rad2)
{
	float max_dist = abs(rad1 + rad2);
	glm::vec3 dist = abs(pos1 - pos2);
	if (glm::length(dist) <= max_dist) 
		return true;
	else
		return false;
}