#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <SDL3/SDL.h>

#include <vector>
#include <filesystem>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/stat.h>

#include "settings.h"
#include "controller_window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui-filebrowser/imfilebrowser.h"

enum class input_idx {
    none = -1,
    south_button,
    east_button,
    west_button,
    north_button,
    back_button,
    guide_button,
    start_button,
    left_stick_click,
    right_stick_click,
    left_shoulder,
    right_shoulder,
    dpad_up,
    dpad_down,
    dpad_left,
    dpad_right,
    misc1,
    paddle1,
    paddle2,
    paddle3,
    paddle4,
    touchpad_click, // Left trackpad click
    misc2,          // Right trackpad click
    misc3,
    misc4,
    misc5,
    misc6,
    num_input
};

typedef struct controller_window_struct controller_window;

typedef struct my_tab{
	unsigned ID;
	std::string title;
}window_tab;

void createSettingsWindow();

GLFWwindow* getSettingsWindow();

void close_window(unsigned ID);

void removeTab(unsigned tab);

void saveTabs();

void loadTabs();

void removeSettingsWindow();

void drawSettingsWindow();

void settings_window_input(bool &quit);

void settings_framebuffer_size_callback(GLFWwindow* window, int width, int height);

void glfw_error_callback(int error, const char* description);

void GetOpenGLVersionInfo();

void OsOpenInShell(const char* path);

const GLFWvidmode* get_vid_mode();

bool check_filename_valid(const char* name);

std::string get_top_folder(std::string path);

std::string get_first_model();

std::vector<std::string> get_current_mapping(SDL_Gamepad* sdl_controller);

std::vector<std::string> get_binding(std::string b);

#endif
