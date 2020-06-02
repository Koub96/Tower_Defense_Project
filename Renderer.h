#ifndef BIM_ENGINE_RENDERER_H
#define BIM_ENGINE_RENDERER_H

#include "GLEW\glew.h"
#include "glm\glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "SpotlightNode.h"
#include "ParticleSystem.h"
#include "Pirate.h"
#include "Object.h"

class Renderer
{
public:
	enum RENDERING_MODE
	{
		TRIANGLES,
		LINES,
		POINTS
	};

	//Gameplay Mechanics
	const float TOWER_FIRE_FREQ = 4.0f; // tower firing frequency in seconds
	const float TOWER_FIRE_RANGE = 8.2f; // about 2 tiles range

	const int INIT_AVAILABLE_TOWERS = 3;// towers available at the start of the game
	const int TOWER_REPLACE_CD = 30; // tower replacement cooldown in seconds
	const int TOWER_RECHARGE_CD = 120; // tower recharge cooldown in seconds

	const int INIT_AVAILABLE_TRAPS = 3; // traps available at the start of the game
	const int MAX_AVAILABLE_TRAPS = 5; // maximum traps capacity
	const int TRAP_RECHARGE_CD = 30; // trap recharge cooldown in seconds
	const float TRAP_ACTIVE_DURATION = 10.0f; // trap duration in seconds

	const int INIT_PIRATE_HEALTH = 3; //Initial pirate's maximum health
	const int PIRATE_POWER_UP_тиле = 60; //Frequence of Pirate power-up in seconds
	const int PIRATE_WAVE_SPAWN_TIME = 35.0; //time between wave spawns in seconds.
											//Pirates spawn every 3 sec inside a wave. 
										   //So we add 15sec to count from the time the last enemy of last wave has spawned
	const float PIRATE_NORMAL_SPEED = 1.0;
	const float PIRATE_SLOWED_SPEED = 0.5;

	
	const int TOTAL_COINS = 300; //total number of coins at the start of the game (must be multiple of 100)
	const float COIN_CAPACITY = 100; // Coin capacity of each chest


	//some public bools to receive "requests" from main.cpp
	bool draw_selection = false;
	bool move_right = false;
	bool move_left = false;
	bool move_up = false;
	bool move_down = false;

	bool game_over = false;

protected:

	int												m_screen_width, m_screen_height;
	float											m_continous_time;

	glm::mat4										m_view_matrix;
	glm::mat4										m_projection_matrix;
	glm::vec3										m_camera_position;
	glm::vec3										m_camera_target_position;
	glm::vec3										m_camera_up_vector;
	glm::vec2										m_camera_movement;
	glm::vec2										m_camera_look_angle_destination;

	Pirate											parts_of_pirate[3*sizeof(Pirate)];

	glm::vec3										pos_of_roads[27*sizeof(glm::vec3)];



	// Lights
	SpotLightNode									m_spotlight_node;
	SpotLightNode									m_spotlight_node2;


	//Pirates	
	glm::mat4										pirate_transformation_matrix;
	glm::mat4										pirate_transformation_normal_matrix;
	
	class GeometryNode*								pirate_body;
	glm::mat4										pirate_body_transformation_matrix;
	glm::mat4										pirate_body_transformation_normal_matrix;
	class GeometryNode*								pirate_arm;
	glm::mat4										pirate_arm_transformation_matrix;
	glm::mat4										pirate_arm_transformation_normal_matrix;
	class GeometryNode*								pirate_left_foot;
	glm::mat4										pirate_left_foot_transformation_matrix;
	glm::mat4										pirate_left_foot_transformation_normal_matrix;
	class GeometryNode*								pirate_right_foot;
	glm::mat4										pirate_right_foot_transformation_matrix;
	glm::mat4										pirate_right_foot_transformation_normal_matrix;
	glm::mat4										pirate_model_matrix;
	glm::mat4										pirate_model_matrix2;
	glm::mat4										pirate_model_matrix3;
	glm::mat4										pirate_model_matrix4;
	glm::mat4										pirate_model_matrix5;

	class Pirate									pirateBody;
	class Pirate									pirateArm;
	class Pirate									pirateLeftFoot;
	class Pirate									pirateRightFoot;
	class Pirate									wholePirate;
	class Pirate									wholePirate2;
	class Pirate									wholePirate3;
	class Pirate									wholePirate4;
	class Pirate									wholePirate5;

	//Int variables for each pirate that indicates if he should be rendered at each frame.
	int												render_pirate_1;
	int												render_pirate_2;
	int												render_pirate_3;
	int												render_pirate_4;
	int												render_pirate_5;

	std::vector<Pirate>								pirates; //Vector to keep the dynamically generated new pirates.

	bool											flagwave; //a flag to trigger the spawn of a new wave.
	int												pirate_max_health;// Pirates' health
	float											my_dt;
	float											spawn_time; //the spawn time with which we set the spawn time of each pirate.
	float											interval; //the difference that each pirate should have from eachother when spawing.




	//Other objects

	class GeometryNode*								terrain;
	glm::mat4										terrain_transformation_matrix;
	glm::mat4										terrain_transformation_normal_matrix;

	class Object									roadTile;
	class GeometryNode*								road;

	class Object									selected_tile;
	class GeometryNode*								selected_terrain;
	class GeometryNode*								selected_road;

	class std::vector<Object>						chests;
	class GeometryNode*								treasure_chest;

	class std::vector<Object>						projectiles;
	class GeometryNode*								cannonball;

	class std::vector<Object> 						towers; //vector that holds all towers currently on the scene
	class GeometryNode*								tower;
	int												available_towers;

	class std::vector<Object>						traps;
	class GeometryNode*								trap;
	int												available_traps;

	glm::vec3										selected_position; //selected postition to place tower or trap
	bool											selected_grass; //if selected position is on a grass tile


	bool											InitRenderingTechniques();
	bool											InitIntermediateShaderBuffers();
	bool											InitCommonItems();
	bool											InitLightSources();
	bool											InitGeometricMeshes();

	ParticleEmitter									m_particle_emitter;
	ShaderProgram									m_particle_rendering_program;

	ShaderProgram									m_geometry_rendering_program;
	ShaderProgram									m_spot_light_shadow_map_program;
	ShaderProgram									m_postprocess_program;

	// Geometry Rendering Intermediate Buffer
	GLuint											m_fbo;
	GLuint											m_fbo_depth_texture;
	GLuint											m_fbo_texture;
	
	GLuint											m_vao_fbo, m_vbo_fbo_vertices;

public:
	Renderer();
	~Renderer();
	bool											Init(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	void											Update(float dt);
	bool											ResizeBuffers(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	bool											ReloadShaders();
	void											Render();
	void											RenderGeometry();
	
	// Camera Functions
	void											CameraMoveForward(bool enable);
	void											CameraMoveBackWard(bool enable);
	void											CameraMoveLeft(bool enable);
	void											CameraMoveRight(bool enable);
	void											CameraLook(glm::vec2 lookDir);	


	//Drawing methods
	void											DrawRoadPath(glm::vec3 initial_pos);
	void											DrawPirate(glm::mat4& model_matrix);
	void											RenderPiratePartsGeometryNode(GeometryNode* g, glm::mat4 g_transformation_matrix, glm::mat4 g_normal_matrix, glm::vec3 initial_pos, glm::vec3 offset, int i, glm::mat4& model_matrix);
	void											RenderObject(Object* o, glm::vec3 position, glm::vec3 offset, glm::vec3 scaling, float alpha);//Method that Draws any Object
	void											DrawGeometryNodeToShadowMap(GeometryNode* node, glm::mat4* model_matrix, glm::mat4* normal_matrix);
	void											RenderShadowMaps();
	void											RenderToOutFB();


	//Other helping methods
	bool											TowerAtSelectedPos();
	bool											TrapAtSelectedPos();
	void											RemoveTower();
	void											GiveNewTower();
	void											GiveNewTrap();
	void											ResetSelectedPos();
	void											IncreasePirateHealth();
	void											AddTrap();
	void											AddTower();

	void											Fire(Object t, glm::vec3 dir);
	bool											testCollision(glm::vec3 pos1, glm::vec3 pos2, float off1, float off2);

	int												getTotalCoins();
};
#endif