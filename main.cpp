#include "SDL2/SDL.h"
#include <iostream>
#include <chrono>
#include "GLEW\glew.h"
#include "Renderer.h"
#include <iostream>
#include <cstdio>
#include <ctime>
#include <thread>         // std::this_thread::sleep_for

using namespace std;

//Screen attributes
SDL_Window * window;

//OpenGL context 
SDL_GLContext gContext;
const int SCREEN_WIDTH = 1380;	//800;	//640;
const int SCREEN_HEIGHT = 1024;	//600;	//480;

//Event handler
SDL_Event event;

Renderer * renderer = nullptr;

//Initialize clocks, etc.

bool selection_pressed = false;

//clock mechanism for tower replacement
std::clock_t start_t;
double duration_t;

//clock mechanism for tower recharge
std::clock_t start_tr;
double duration_tr;

//clock mechanism for trap recharge
std::clock_t start_tp;
double duration_tp;

//clock mechanism for pirate's scaling health
std::clock_t start_ph;
double duration_ph;

//clock mechanism for gameplay time
std::clock_t start_gp;
int duration_gp;


void func()
{
	system("pause");
}

// initialize SDL and OpenGL
bool init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		return false;
	}

	// use Double Buffering
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
		cout << "Error: No double buffering" << endl;

	// set OpenGL Version (3.3)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	// Create Window
	window = SDL_CreateWindow("Greedy Pirates", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return false;
	}

	//Create OpenGL context
	gContext = SDL_GL_CreateContext(window);
	if (gContext == NULL)
	{
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Disable Vsync
	if (SDL_GL_SetSwapInterval(0) == -1)
		printf("Warning: Unable to disable VSync! SDL Error: %s\n", SDL_GetError());

	// Initialize GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error loading GLEW\n");
		return false;
	}
	// some versions of glew may cause an opengl error in initialization
	glGetError();

	renderer = new Renderer();
	bool engine_initialized = renderer->Init(SCREEN_WIDTH, SCREEN_HEIGHT);

	//atexit(func);
	
	return engine_initialized;
}

void clean_up()
{
	delete renderer;

	SDL_GL_DeleteContext(gContext);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char *argv[])
{
	//Initialize
	if (init() == false)
	{
		system("pause");
		return EXIT_FAILURE;
	}

	cout << "\n" <<"--------------Welcome!--------------\n";
	cout << "Controls: \n";
	cout << "Q + Arrows -> Navigate to select a tile\n";
	cout << "Space -> Place or replace a tower\n";
	cout << "F -> Place a trap\n";
	cout << "Esc -> Exit game\n";
	cout << "Don't let the pirates steal your treasure!\n" << "\n";

	//Quit flag
	bool quit = false;
	bool mouse_button_pressed = false;
	glm::vec2 prev_mouse_position(0);

	auto simulation_start = chrono::steady_clock::now();

	start_t = std::clock();	//start clock for tower replacement
	start_tr = std::clock();//start clock for tower recharge
	start_tp = std::clock();//start clock for traps recharge
	start_ph = std::clock();//start clock for pirate health
	start_gp = std::clock();//start clock for gameplay time


	// Wait for user exit
	while (quit == false)
	{
		
		//check if it's time for tower recharge
		duration_tr = (std::clock() - (double)start_tr) / (double)CLOCKS_PER_SEC;
		if (duration_tr >= renderer->TOWER_RECHARGE_CD)
		{
			renderer->GiveNewTower();
			start_tr = std::clock();//restart timer for tower recharge
		}

		//check if it's time for trap recharge
		duration_tp = (std::clock() - (double)start_tp) / (double)CLOCKS_PER_SEC;
		if (duration_tp >= renderer->TRAP_RECHARGE_CD)
		{
			renderer->GiveNewTrap();
			start_tp = std::clock();//restart timer for trap recharge
		}

		//check if it's time for pirate health increase
		duration_ph = (double)(std::clock() - (double)start_ph) / (double)CLOCKS_PER_SEC;
		if (duration_ph >= renderer->PIRATE_POWER_UP_тиле)
		{
			renderer->IncreasePirateHealth();
			start_ph = std::clock();//restart timer for trap recharge
		}

		// While there are events to handle
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
			// Key-down events************************//
			else if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE) 
				{
					quit = true;
				}
				else if (event.key.keysym.sym == SDLK_r)
				{
					renderer->ReloadShaders();
				}

				else if (event.key.keysym.sym == SDLK_w )
				{
					renderer->CameraMoveForward(true);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					renderer->CameraMoveBackWard(true);
				}	
				else if (event.key.keysym.sym == SDLK_a)
				{
					renderer->CameraMoveLeft(true);
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					renderer->CameraMoveRight(true);
				}

				else if (event.key.keysym.sym == SDLK_UP)
				{
					//nothing
				}
				else if (event.key.keysym.sym == SDLK_DOWN)
				{
					//nothing
				}
				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					//nothing
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					//nothing
				}

				else if (event.key.keysym.sym == SDLK_q)
				{
					//nothing
				}
				else if (event.key.keysym.sym == SDLK_SPACE)
				{
					//nothing
				}
				else if (event.key.keysym.sym == SDLK_f)
				{
					//nothing
				}

			}

			// Key-up events************************//
			else if (event.type == SDL_KEYUP)
			{
				if (event.key.keysym.sym == SDLK_w)
				{
					renderer->CameraMoveForward(false);
				}
				else if (event.key.keysym.sym == SDLK_s)
				{
					renderer->CameraMoveBackWard(false);
				}
				else if (event.key.keysym.sym == SDLK_a)
				{
					renderer->CameraMoveLeft(false);
				}
				else if (event.key.keysym.sym == SDLK_d)
				{
					renderer->CameraMoveRight(false);
				}

				else if (event.key.keysym.sym == SDLK_UP)
				{
					renderer->move_up = true;
				}
				else if (event.key.keysym.sym == SDLK_DOWN)
				{
					renderer->move_down = true;
				}
				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					renderer->move_left = true;
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					renderer->move_right = true;
				}
				else if (event.key.keysym.sym == SDLK_q)
				{
					renderer->draw_selection = !renderer->draw_selection;
					selection_pressed = renderer->draw_selection;
					renderer->ResetSelectedPos();
				}

				else if (event.key.keysym.sym == SDLK_SPACE) //when space is pressed
				{
					if (selection_pressed) //if q hasn't been pressed, space does nothing
					{
						if (renderer->TowerAtSelectedPos()) //pressed space on already placed tower
						{
							duration_t = (std::clock() - (double)start_t) / (double)CLOCKS_PER_SEC; //calculate time passed since last tower movement
							
							if (duration_t >= renderer->TOWER_REPLACE_CD)
							{
								renderer->RemoveTower(); //delete tower
								start_t = std::clock(); // restart timer for replacement
							}
							else
							{
								cout << "\n";
								cout << "Tower replacement is on cool-down. \n";
							}
						}
						else //pressed space on empty tile
						{
							renderer->AddTower(); //"request" to add tower
						}
					}
				}
				else if (event.key.keysym.sym == SDLK_f)
				{
					if (selection_pressed) //if q hasn't been pressed, f does nothing
					{
						if(!renderer->TrapAtSelectedPos())//if there is no trap already on that tile
							renderer->AddTrap(); //"request" to add trap
					}
				}
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				int x = event.motion.x;
				int y = event.motion.y;
				if (mouse_button_pressed)
				{
					renderer->CameraLook(glm::vec2(x, y) - prev_mouse_position);
					prev_mouse_position = glm::vec2(x, y);
				}
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					int x = event.button.x;
					int y = event.button.y;
					mouse_button_pressed = (event.type == SDL_MOUSEBUTTONDOWN);
					prev_mouse_position = glm::vec2(x, y);
				}
			}
			else if (event.type == SDL_MOUSEWHEEL)
			{
				int x = event.wheel.x;
				int y = event.wheel.y;
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					renderer->ResizeBuffers(event.window.data1, event.window.data2);
				}
			}
			else
			{
				//nothing
			}
		}

		//Check if game is over
		if (renderer->game_over)
		{
			duration_gp =(int)( (double)(std::clock() - (double)start_gp) / (double)CLOCKS_PER_SEC );

			int minutes = duration_gp / 60;
			int seconds = duration_gp % 60;

			cout << "\n" << "\n";	
			cout << "******************************************\n";
			cout << "*		GAME OVER		 *\n";
			cout << "*	YOU SURVIVED FOR " <<minutes<< "m "<<seconds<<"s		 *\n";
			cout << "******************************************\n";
			cout << "\n" << "\n";

			exit(1);			
		}
		else
		{
			// Compute the ellapsed time
			auto simulation_end = chrono::steady_clock::now();
			float dt = chrono::duration <float>(simulation_end - simulation_start).count(); // in seconds
			simulation_start = chrono::steady_clock::now();

			// Update
			renderer->Update(dt);
			// Draw
			renderer->Render();
			//Update screen (swap buffer for double buffering)
			SDL_GL_SwapWindow(window);
		}

	}//Quit Loop End

	//Clean up
	clean_up();

	return 0;
}
