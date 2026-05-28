#include <SDL3/SDL_main.h>
#include "settings_window.h"

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif

bool gQuit = false;

void InitializeProgram(){
	if(!SDL_Init(SDL_INIT_GAMEPAD)) {
        printf("Error: %s\n", SDL_GetError());
        exit(1);
    }
	
	createSettingsWindow();

	loadTabs();
}

void Input(){
	glfwPollEvents();
	
	settings_window_input(gQuit);
	controller_window_input();
}

void Draw(){
	drawSettingsWindow();
	drawControllerWindows();
}

void MainLoop(){
	while(!gQuit){
		Input();

		Draw();
	}
}

void Cleanup(){
	saveTabs();

	removeSettingsWindow();
	
	destroyWindows();
	
	SDL_Quit();

	glfwTerminate();
}

int main(int argc, char *argv[]) {
	InitializeProgram();
	
	MainLoop();

	Cleanup();

	return 0;
}
