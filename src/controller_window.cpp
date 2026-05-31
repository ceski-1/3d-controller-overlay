#include "window_icon.h"
#include "controller_window.h"

#include "cube_info.h"
#include "shaders.h"

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif

float grid_vertices[] = {
    -1.0f,  0.0f,  0.0f,  
     1.0f,  0.0f,  0.0f,
	   
     1.0f,  0.0f,  0.0f,  
     0.0f,  0.0f,  1.0f,  
     
	 0.0f,  0.0f,  1.0f,  
     1.0f,  0.0f,  0.0f,  
     
	-1.0f,  0.0f,  1.0f,  
     1.0f,  0.0f,  1.0f
};

unsigned int defaultWidth = 640;
unsigned int defaultHeight = 480;

std::vector<controller_window> windows;

void clearGripSense(controller_window &w) {
	w.num_gripsense = 0;
	w.has_gripsense[0] = false;
	w.has_gripsense[1] = false;
	w.model.meshes[(int)mesh_idx::left_gripsense].visible = false;
	w.model.meshes[(int)mesh_idx::right_gripsense].visible = false;
}

void configureGripSense(controller_window &w) {
	SDL_GamepadCapSenseType gripsenses[] = {
		SDL_GAMEPAD_CAPSENSE_LEFT_GRIP,
		SDL_GAMEPAD_CAPSENSE_RIGHT_GRIP,
	};

	w.num_gripsense = 0;
	for (size_t i = 0; i < SDL_arraysize(gripsenses); i++) {
		w.has_gripsense[i] = SDL_GamepadHasCapSense(w.sdl_controller, gripsenses[i]);
		if (w.has_gripsense[i]) {
			w.num_gripsense++;
		}
	}
}

void clearStickSense(controller_window &w) {
	w.num_sticksense = 0;
	w.has_sticksense[0] = false;
	w.has_sticksense[1] = false;
	w.model.meshes[(int)mesh_idx::left_stick_cap].released_value = 0.0f;
	w.model.meshes[(int)mesh_idx::right_stick_cap].released_value = 0.0f;
}

void configureStickSense(controller_window &w) {
	SDL_GamepadCapSenseType sticksenses[] = {
		SDL_GAMEPAD_CAPSENSE_LEFT_STICK,
		SDL_GAMEPAD_CAPSENSE_RIGHT_STICK,
	};

	w.num_sticksense = 0;
	for (size_t i = 0; i < SDL_arraysize(sticksenses); i++) {
		w.has_sticksense[i] = SDL_GamepadHasCapSense(w.sdl_controller, sticksenses[i]);
		if (w.has_sticksense[i]) {
			w.num_sticksense++;
		}
	}
}

void clearTouchpads(controller_window &w) {
	w.num_touchpads = 0;
	w.num_fingers[0] = 0;
	w.num_fingers[1] = 0;

	Mesh *tpoint_meshes[] = {
		&w.model.meshes[(int)mesh_idx::touch_point1],
		&w.model.meshes[(int)mesh_idx::touch_point2],
	};

	for (size_t i = 0; i < SDL_arraysize(tpoint_meshes); i++) {
		tpoint_meshes[i]->touch_state = false;
		tpoint_meshes[i]->visible = false;
	}
}

void configureTouchpads(controller_window &w) {
	bool has_touchpads = false;
	w.num_touchpads = SDL_GetNumGamepadTouchpads(w.sdl_controller);
	w.num_touchpads = SDL_clamp(w.num_touchpads, 0, 2);
	if (w.num_touchpads > 0) {
		// Two configurations supported:
		// 1. One touchpad, up to two fingers simultaneously.
		// 2. Two trackpads, only one finger per trackpad.
		const int max_fingers = (w.num_touchpads == 1 ? 2 : 1);
		for (int i = 0; i < w.num_touchpads; i++) {
			w.num_fingers[i] = SDL_GetNumGamepadTouchpadFingers(w.sdl_controller, i);
			w.num_fingers[i] = SDL_clamp(w.num_fingers[i], 0, max_fingers);
			if (w.num_fingers[i] > 0) {
				has_touchpads = true;
			}
		}
	}
	if (!has_touchpads) {
		clearTouchpads(w);
	}
}

void createControllerWindow(std::string title, std::string model_path){
    controller_window w;

    // Set number of samples for multi-sampling
    glfwWindowHint(GLFW_SAMPLES, 4);
    w.glfw_window = glfwCreateWindow(defaultWidth, defaultHeight, title.c_str(), NULL, NULL);
    if (w.glfw_window == NULL){
        std::cout << "Failed to create controller indow" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(w.glfw_window);
    // Enable multi-sampling
    glEnable(GL_MULTISAMPLE);

	GLFWimage images[1];
	unsigned char pixels[WINDOW_ICON_WIDTH * WINDOW_ICON_HEIGHT * WINDOW_ICON_BYTES_PER_PIXEL];
	SDL_memcpy(pixels, window_icon, sizeof(pixels));
	images[0].pixels = pixels;
	images[0].width = WINDOW_ICON_WIDTH;
	images[0].height = WINDOW_ICON_HEIGHT;
	glfwSetWindowIcon(w.glfw_window, 1, images); 

	//glfwSetFramebufferSizeCallback(w.glfw_window, controller_framebuffer_size_callback);
	//glfwSetWindowSizeCallback(w.glfw_window, controller_window_size_callback);
	glfwSetScrollCallback(w.glfw_window, controller_window_scroll_callback);
	//glfwSetWindowIconifyCallback(w.glfw_window, controller_window_iconify_callback);

	//GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
	//const GLFWvidmode* vid_mode = glfwGetVideoMode(primary_monitor);
	//w.frame_cap = vid_mode->refreshRate;

	w.lastFrame = glfwGetTime();

	make_grid(w);

	lightingSpecification(w);

	//* shaders from header file
	createShader(w.shader, 
				 vertex_shader_code.c_str(),
				 fragment_shader_code.c_str());
	createShader(w.grid_shader, 
				 grid_vertex_shader_code.c_str(), 
				 grid_fragment_shader_code.c_str());
	createShader(w.light_source_shader, 
				 light_source_vertex_shader_code.c_str(),
				 light_source_fragment_shader_code.c_str());
	//*/
	/* shaders from text files
	createShader(w.shader, 
				 GetShaderSource("shaders/vertex.glsl").c_str(), 
				 GetShaderSource("shaders/fragment.glsl").c_str());
	createShader(w.grid_shader, 
				 GetShaderSource("shaders/grid_vertex.glsl").c_str(), 
				 GetShaderSource("shaders/grid_fragment.glsl").c_str());
	createShader(w.light_source_shader, 
				 GetShaderSource("shaders/light_source_vertex.glsl").c_str(),
				 GetShaderSource("shaders/light_source_fragment.glsl").c_str());
	//*/

	direct_light d;
	w.direct_lights.push_back(d);
	
	loadModel(w.model, model_path);
	w.model_name = get_top_folder(model_path);

	w.sdl_controller = NULL;
	w.sdl_id = 0;
	clearTouchpads(w);
	clearStickSense(w);
	clearGripSense(w);
	SDL_JoystickID *ids = SDL_GetGamepads(NULL);
	if (ids) {
		w.sdl_controller = SDL_OpenGamepad(ids[0]);
		if (w.sdl_controller != NULL) {
			w.sdl_id = ids[0];
			configureTouchpads(w);
			configureStickSense(w);
			configureGripSense(w);
		}
		SDL_free(ids);
	}

	if(w.sdl_controller != NULL){
		char *default_mapping = SDL_GetGamepadMapping(w.sdl_controller);
		if (default_mapping != NULL) {
			w.default_mapping = default_mapping;
			SDL_free(default_mapping);
		} else {
			w.default_mapping = "";
		}
		if (SDL_GamepadHasSensor(w.sdl_controller, SDL_SENSOR_GYRO)){
			SDL_SetGamepadSensorEnabled(w.sdl_controller, SDL_SENSOR_GYRO, true);
		}
		w.gyro_matrix = glm::mat4(1.0f);
	}else{
		std::cout << "couldn't open sdl controller." << std::endl;
		std::cout << SDL_GetError() << std::endl;
	}
	
	windows.push_back(w);
}

void lightingSpecification(controller_window &w){
	glGenVertexArrays(1, &w.lighting_vao);
	glGenBuffers(1, &w.lighting_vertex_data);
	glGenBuffers(1, &w.lighting_normal_data);
	glGenBuffers(1, &w.lighting_texture_data);

	glBindVertexArray(w.lighting_vao);

	glBindBuffer(GL_ARRAY_BUFFER, w.lighting_vertex_data);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cube_vertices) * sizeof(GLfloat),
                 cube_vertices,
                 GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, w.lighting_normal_data);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cube_normals) * sizeof(GLfloat),
                 cube_normals,
                 GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, w.lighting_texture_data);
	glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cube_tex_coords) * sizeof(GLfloat),
                 cube_tex_coords,
                 GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void createShader(GLuint &shader_id, const char* vs_source, const char* fs_source){
	shader_id = CreateShaderProgram(vs_source, fs_source);
}

void controller_framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

void controller_window_size_callback(GLFWwindow* window, int width, int height){
	
}

void controller_window_scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	for(unsigned i = 0; i<windows.size(); ++i){
		if(windows[i].glfw_window == window && windows[i].scroll_to_resize){
			int w = 0;
			int h = 0;
			glfwGetWindowSize(window, &w, &h);
			if(yoffset > 0){
				w = (int)(w * 1.05f);
				h = (int)(h * 1.05f);
				if (w > get_vid_mode()->width)
					w = get_vid_mode()->width;
				if (h > get_vid_mode()->height)
					h = get_vid_mode()->height;
			}else if(yoffset < 0){
				w = (int)(w * 0.95f);
				h = (int)(h * 0.95f);
				if (w < 10)
					w = 10;
				if (h < 10)
					h = 10;
			}
			glfwSetWindowSize(window, w, h);
			break;
		}
	}
}

void controller_window_iconify_callback(GLFWwindow* window, int iconified){
	if (iconified){
        std::cout << "the controller window has been iconified" << std::endl;
    }else{
        std::cout << "the controller window has been restored" << std::endl;
    }
}

controller_window* getLastWindow(){
	return &windows.back();
}

controller_window* getControllerWindow(unsigned ID){
	for(unsigned i=0; i<windows.size(); ++i){
		if(windows[i].ID == ID){
			return &windows[i];
		}
	}
	return nullptr;
}

static void updateGyroState(controller_window &w) {
	if (!w.gyro_enabled) {
		return;
	}

	if (w.gyro_time == 0 || (w.gyro_data[0] == 0.0f && w.gyro_data[1] == 0.0f && w.gyro_data[2] == 0.0f)) {
		return;
	}

	if (w.gyro_toggled) {
		w.last_gyro_time = w.gyro_time;
		w.gyro_toggled = false;
		return;
	}

	if (w.last_gyro_time == 0) {
		w.last_gyro_time = w.gyro_time;
		return;
	}

	const float delta_time = (float)(w.gyro_time - w.last_gyro_time) / SDL_NS_PER_SECOND;
	if (delta_time >= 0.0001f) {
		w.gyro_matrix = glm::rotate(w.gyro_matrix, w.gyro_data[0] * delta_time, glm::vec3(1.0f, 0.0f, 0.0f));
		w.gyro_matrix = glm::rotate(w.gyro_matrix, w.gyro_data[1] * delta_time, glm::vec3(0.0f, 1.0f, 0.0f));
		w.gyro_matrix = glm::rotate(w.gyro_matrix, w.gyro_data[2] * delta_time, glm::vec3(0.0f, 0.0f, 1.0f));
		w.last_gyro_time = w.gyro_time;

		// Gyro correction
		if (w.gyro_correction > 0) {
			glm::vec3 controller_up = glm::vec3(0.0f, 1.0f, 0.0f) * glm::mat3(w.gyro_matrix);
			glm::vec3 up_error_axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(controller_up));
			w.gyro_matrix = glm::rotate(w.gyro_matrix, w.gyro_correction * 0.0001f, up_error_axis);
			
			glm::vec3 controller_right = glm::vec3(1.0f, 0.0f, 0.0f) * glm::mat3(w.gyro_matrix);
			glm::vec3 right_error_axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), controller_right);
			w.gyro_matrix = glm::rotate(w.gyro_matrix, w.gyro_correction * 0.0001f, right_error_axis);
		}
	}

	// Reset gyro button combo
	if (w.sdl_controller != NULL) {
		if (w.reset_gyro_button1 > (int)input_idx::none && w.reset_gyro_button1 < (int)input_idx::num_input
		    && w.reset_gyro_button2 > (int)input_idx::none && w.reset_gyro_button2 < (int)input_idx::num_input
		    && SDL_GetGamepadButton(w.sdl_controller, (SDL_GamepadButton)(w.reset_gyro_button1))
		    && SDL_GetGamepadButton(w.sdl_controller, (SDL_GamepadButton)(w.reset_gyro_button2))) {
			w.gyro_matrix = glm::mat4(1.0f);
		}
	}
}

static Sint16 getGamepadAxisValue(controller_window &w, SDL_GamepadAxis sdl_axis) {
	return (w.sdl_controller != NULL ? SDL_GetGamepadAxis(w.sdl_controller, sdl_axis) : 0);
}

static void updateStickValues(controller_window &w, SDL_GamepadAxis x_axis, SDL_GamepadAxis y_axis,
                              mesh_idx base, mesh_idx ring, mesh_idx cap) {
	const float x_value = (float)getGamepadAxisValue(w, x_axis);
	const float y_value = (float)getGamepadAxisValue(w, y_axis);
	w.model.meshes[(int)base].stick_X = x_value;
	w.model.meshes[(int)base].stick_Y = y_value;
	w.model.meshes[(int)ring].stick_X = x_value;
	w.model.meshes[(int)ring].stick_Y = y_value;
	w.model.meshes[(int)cap].stick_X = x_value;
	w.model.meshes[(int)cap].stick_Y = y_value;

	Mesh &ring_mesh = w.model.meshes[(int)ring];
	const float ring_x = abs(ring_mesh.stick_X / 32767.0f);
	const float ring_y = abs(ring_mesh.stick_Y / 32767.0f);
	if (ring_x > ring_mesh.ring_highlight_deadzone * 0.01f ||
		ring_y > ring_mesh.ring_highlight_deadzone * 0.01f) {
		ring_mesh.highlight_value = std::max(ring_x, ring_y) * 1.2f;
	} else {
		ring_mesh.highlight_value = 0.0f;
	}
}

static void updateStickState(controller_window &w) {
	updateStickValues(w, SDL_GAMEPAD_AXIS_LEFTX, SDL_GAMEPAD_AXIS_LEFTY,
	                  mesh_idx::left_stick_base, mesh_idx::left_stick_ring,
	                  mesh_idx::left_stick_cap);

	updateStickValues(w, SDL_GAMEPAD_AXIS_RIGHTX, SDL_GAMEPAD_AXIS_RIGHTY,
	                  mesh_idx::right_stick_base, mesh_idx::right_stick_ring,
	                  mesh_idx::right_stick_cap);
}

static void updateTriggerValues(controller_window &w, SDL_GamepadAxis trigger_axis, mesh_idx trigger) {
	Mesh &trigger_mesh = w.model.meshes[(int)trigger];
	trigger_mesh.pull = (float)getGamepadAxisValue(w, trigger_axis);
	trigger_mesh.highlight_value = trigger_mesh.pull / 32767.0f;
	trigger_mesh.press = trigger_mesh.highlight_value;
}

static void updateTriggerState(controller_window &w) {
	updateTriggerValues(w, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, mesh_idx::left_trigger);
	updateTriggerValues(w, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, mesh_idx::right_trigger);
}

static void updateButtonState(controller_window &w) {
	const int b_start = (int)mesh_idx::south_button;
	for (int b = b_start; b < (int)mesh_idx::num_mesh; b++) {
		Mesh &button_mesh = w.model.meshes[b];
		if (w.sdl_controller != NULL && SDL_GetGamepadButton(w.sdl_controller, (SDL_GamepadButton)(b - b_start))) {
			button_mesh.press = 1.0f;
			button_mesh.highlight_value = 1.0f;
		} else {
			button_mesh.press = 0.0f;
			button_mesh.highlight_value = button_mesh.released_value;
		}
	}
}

static void updateTouchPointState(controller_window &w) {
	if (w.num_touchpads < 1) {
		return;
	}

	Mesh *tpoint_meshes[] = {
		&w.model.meshes[(int)mesh_idx::touch_point1],
		&w.model.meshes[(int)mesh_idx::touch_point2],
	};

	if (w.sdl_controller == NULL) {
		for (size_t i = 0; i < SDL_arraysize(tpoint_meshes); i++) {
			tpoint_meshes[i]->touch_state = false;
			tpoint_meshes[i]->visible = false;
		}
		return;
	}

	const bool touchpad_press[] = {
		w.model.meshes[(int)mesh_idx::touchpad].press > 0.0f,
		w.model.meshes[(int)mesh_idx::misc2].press > 0.0f,
	};

	int tpoint_idx = 0;
	for (int touchpad_idx = 0; touchpad_idx < w.num_touchpads; touchpad_idx++) {
		for (int finger_idx = 0; finger_idx < w.num_fingers[touchpad_idx]; finger_idx++) {
			Mesh *tpoint_mesh = tpoint_meshes[tpoint_idx];
			float pressure = 0.0f;
			SDL_GetGamepadTouchpadFinger(w.sdl_controller,
			                             touchpad_idx,
			                             finger_idx,
			                             &tpoint_mesh->touch_state,
			                             &tpoint_mesh->touch_X,
			                             &tpoint_mesh->touch_Y,
			                             &pressure);
			if (tpoint_mesh->touch_state) {
				tpoint_mesh->highlight_value = 0.3f + 10.0f * pressure * pressure;
				if (touchpad_press[touchpad_idx]) {
					tpoint_mesh->highlight_value = 1.0f - tpoint_mesh->highlight_value;
				}
				tpoint_mesh->highlight_value = SDL_clamp(tpoint_mesh->highlight_value, 0.0f, 1.0f);
				tpoint_mesh->visible = true;
			} else {
				tpoint_mesh->highlight_value = 0.0f;
				tpoint_mesh->visible = false;
			}
			tpoint_idx++;
		}
	}
}

static void updateStickSenseState(controller_window &w) {
	if (w.num_sticksense < 1) {
		return;
	}

	Mesh *sticksense_meshes[] = {
		&w.model.meshes[(int)mesh_idx::left_stick_cap],
		&w.model.meshes[(int)mesh_idx::right_stick_cap],
	};

	if (w.sdl_controller == NULL) {
		for (size_t i = 0; i < SDL_arraysize(sticksense_meshes); i++) {
			sticksense_meshes[i]->released_value = 0.0f;
		}
		return;
	}

	const SDL_GamepadCapSenseType sticksenses[] = {
		SDL_GAMEPAD_CAPSENSE_LEFT_STICK,
		SDL_GAMEPAD_CAPSENSE_RIGHT_STICK,
	};

	for (size_t i = 0; i < SDL_arraysize(sticksense_meshes); i++) {
		if (w.has_sticksense[i]) {
			if (SDL_GetGamepadCapSense(w.sdl_controller, sticksenses[i])) {
				sticksense_meshes[i]->released_value = 0.25f;
			} else {
				sticksense_meshes[i]->released_value = 0.0f;
			}
		}
	}
}

static void updateGripSenseState(controller_window &w) {
	if (w.num_gripsense < 1) {
		return;
	}

	Mesh *gripsense_meshes[] = {
		&w.model.meshes[(int)mesh_idx::left_gripsense],
		&w.model.meshes[(int)mesh_idx::right_gripsense],
	};

	if (w.sdl_controller == NULL) {
		for (size_t i = 0; i < SDL_arraysize(gripsense_meshes); i++) {
			gripsense_meshes[i]->visible = false;
		}
		return;
	}

	const SDL_GamepadCapSenseType gripsenses[] = {
		SDL_GAMEPAD_CAPSENSE_LEFT_GRIP,
		SDL_GAMEPAD_CAPSENSE_RIGHT_GRIP,
	};

	for (size_t i = 0; i < SDL_arraysize(gripsense_meshes); i++) {
		if (w.has_gripsense[i]) {
			gripsense_meshes[i]->visible = SDL_GetGamepadCapSense(w.sdl_controller, gripsenses[i]);
			gripsense_meshes[i]->highlight_value = gripsense_meshes[i]->visible ? 0.4f : 0.0f;
		}
	}
}

static void updateControllerState() {
	for (size_t i = 0; i < windows.size(); i++) {
		controller_window &w = windows[i];
		updateGyroState(w);
		updateStickState(w);
		updateTriggerState(w);
		updateStickSenseState(w);
		updateButtonState(w);
		updateTouchPointState(w);
		updateGripSenseState(w);
	}
}

static void gamepadAdded(const SDL_Event *event) {
	std::cout << "game controller added." << std::endl;
	
	int game_controllers = 0;
	SDL_JoystickID *ids = SDL_GetGamepads(&game_controllers);

	if (ids && game_controllers == 1) {
		for (size_t i = 0; i < windows.size(); i++) {
			controller_window &w = windows[i];
			w.sdl_id = 0;
			clearTouchpads(w);
			clearStickSense(w);
			clearGripSense(w);
			w.sdl_controller = SDL_OpenGamepad(ids[0]);
			if (w.sdl_controller != NULL) {
				w.sdl_id = ids[0];
				configureTouchpads(w);
				configureStickSense(w);
				configureGripSense(w);
				char *default_mapping = SDL_GetGamepadMapping(w.sdl_controller);
				if (default_mapping != NULL) {
					w.default_mapping = default_mapping;
					SDL_free(default_mapping);
				} else {
					w.default_mapping = "";
				}
				if (SDL_GamepadHasSensor(w.sdl_controller, SDL_SENSOR_GYRO)) {
					SDL_SetGamepadSensorEnabled(w.sdl_controller, SDL_SENSOR_GYRO, true);
				}
			} else {
				std::cout << "couldn't open sdl controller" << std::endl;
				std::cout << SDL_GetError() << std::endl;
			}
		}
	}

	SDL_free(ids);
}

static void handleSensorEvent(const SDL_Event *event) {
	for(size_t i = 0; i < windows.size(); i++)
	{
		controller_window &w = windows[i];
		if (w.gyro_enabled && event->gsensor.which == w.sdl_id && event->gsensor.sensor == SDL_SENSOR_GYRO) {
			// TODO: This should accumulate. Just grab the latest value for now.
			w.gyro_time = event->gsensor.timestamp;
			SDL_memcpy(w.gyro_data, event->gsensor.data, sizeof(w.gyro_data));
		}
	}
}

static void handleEvents() {
	static SDL_Event sdlevents[64];
	while (true) {
		const int num_events = SDL_PeepEvents(sdlevents, 64, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST);
		if (num_events < 1) {
			break;
		}

		for (int i = 0; i < num_events; i++) {
			const SDL_Event *event = &sdlevents[i];
			switch (event->type) {
				case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
					handleSensorEvent(event);
					break;
				case SDL_EVENT_GAMEPAD_ADDED:
					gamepadAdded(event);
					break;
				default:
					break;
			}
		}
	}
}

void controller_window_input() {
	SDL_PumpEvents();
	handleEvents();
	updateControllerState();

	for(unsigned i = 0; i<windows.size(); ++i){
		if(glfwGetMouseButton(windows[i].glfw_window, GLFW_MOUSE_BUTTON_1)){
			if(windows[i].drag_to_move){
				int x = 0;
				int y = 0;
				glfwGetWindowPos(windows[i].glfw_window, &x, &y);

				double wx = 0;
				double wy = 0;
				glfwGetCursorPos(windows[i].glfw_window, &wx, &wy);

				if(windows[i].left_click == false){
					windows[i].left_click_x = wx;
					windows[i].left_click_y = wy;
				}
				windows[i].left_click = true;

				int cx = (int)(x + wx);
				int cy = (int)(y + wy);

				glfwSetWindowPos(windows[i].glfw_window, (int)(cx - windows[i].left_click_x), (int)(cy - windows[i].left_click_y));
			}
		}else{
			if(windows[i].left_click){
				windows[i].left_click = false;
			}
		}

		if(glfwGetMouseButton(windows[i].glfw_window, GLFW_MOUSE_BUTTON_2)){
			double x = 0;
			double y = 0;
			glfwGetCursorPos(windows[i].glfw_window, &x, &y);
			
			if(windows[i].right_click == false){
				windows[i].right_click_x = x;
				windows[i].right_click_y = y;
			}
			windows[i].right_click = true;
			
			double dx = windows[i].right_click_x - x;
			double dy = windows[i].right_click_y - y;

			windows[i].right_click_x = x;
			windows[i].right_click_y = y;

			if(windows[i].freelook){
				windows[i].freelook_yaw += (float)(dx * windows[i].turn_speed * 0.02f);
				windows[i].freelook_pitch += (float)(dy * windows[i].turn_speed * 0.02f);
				if(windows[i].freelook_pitch > 90)
					windows[i].freelook_pitch = 89.999f;
				if(windows[i].freelook_pitch < -90)
					windows[i].freelook_pitch = -89.999f;
			}
		}else{
			if(windows[i].right_click){
				windows[i].right_click = false;
			}
		}
		
		if(glfwWindowShouldClose(windows[i].glfw_window))
			close_window(windows[i].ID);
		
		if (windows[i].freelook){
			const float move_speed = (float)(windows[i].move_speed * 0.5f * windows[i].deltaTime);
			const float turn_speed = (float)(windows[i].turn_speed * 10 * windows[i].deltaTime);

			glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
			glm::vec3 forward = windows[i].freelook_direction;
			forward.y = 0;
			forward = glm::normalize(forward);

			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_W) == GLFW_PRESS)
				windows[i].freelook_position += forward * move_speed;
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_S) == GLFW_PRESS)
				windows[i].freelook_position -= forward * move_speed;
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_A) == GLFW_PRESS)
				windows[i].freelook_position -= glm::normalize(glm::cross(forward, up)) * move_speed;
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_D) == GLFW_PRESS)
				windows[i].freelook_position += glm::normalize(glm::cross(forward, up)) * move_speed;
			
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_SPACE) == GLFW_PRESS)
				windows[i].freelook_position += up * move_speed;
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
				windows[i].freelook_position -= up * move_speed;

			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_LEFT) == GLFW_PRESS){
				windows[i].freelook_yaw += turn_speed;
			}
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_RIGHT) == GLFW_PRESS){
				windows[i].freelook_yaw -= turn_speed;
			}

			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_UP) == GLFW_PRESS){
				windows[i].freelook_pitch += turn_speed;
				if(windows[i].freelook_pitch > 90)
					windows[i].freelook_pitch = 89.999f;
			}
			if (glfwGetKey(windows[i].glfw_window, GLFW_KEY_DOWN) == GLFW_PRESS){
				windows[i].freelook_pitch -= turn_speed;
				if(windows[i].freelook_pitch < -90)
					windows[i].freelook_pitch = -89.999f;
			}
		}
	}
}

void removeControllerWindow(unsigned ID){
	for(unsigned i=0; i<windows.size(); ++i){
		if(windows[i].ID == ID){
			glfwDestroyWindow(windows[i].glfw_window);
			windows.erase(windows.begin() + i);
			break;
		}
	}
}

void destroyWindows(){
	for(controller_window w : windows){
		glfwDestroyWindow(w.glfw_window);
	}
}

void make_grid(controller_window &w){
	std::vector<glm::vec3> vertices;
	std::vector<glm::uvec4> indices;

	int slices = 100;

	for(int j=0; j<=slices; ++j) {
		for(int i=0; i<=slices; ++i) {
			float x = (float)i/(float)slices;
			float y = 0;
			float z = (float)j/(float)slices;
			vertices.push_back(glm::vec3(x, y, z));
		}
	}

	for(int j=0; j<slices; ++j) {
		for(int i=0; i<slices; ++i) {

			int row1 =  j    * (slices+1);
			int row2 = (j+1) * (slices+1);

			indices.push_back(glm::uvec4(row1+i, row1+i+1, row1+i+1, row2+i+1));
			indices.push_back(glm::uvec4(row2+i+1, row2+i, row2+i, row1+i));

		}
	}

	glGenVertexArrays( 1, &w.grid_vao);
	glBindVertexArray( w.grid_vao );

	glGenBuffers( 1, &w.grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, w.grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	glGenBuffers( 1, &w.grid_ibo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, w.grid_ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(glm::uvec4), glm::value_ptr(indices[0]), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	w.grid_length = (GLuint)indices.size()*4;
}

void update_camera(controller_window &w, GLuint &shader, int window_width, int window_height){
	glUseProgram(shader);

	if(w.freelook){
		w.freelook_direction.x = cos(glm::radians(w.freelook_pitch)) * sin(glm::radians(w.freelook_yaw));
		w.freelook_direction.y = sin(glm::radians(w.freelook_pitch));
		w.freelook_direction.z = cos(glm::radians(w.freelook_pitch)) * cos(glm::radians(w.freelook_yaw));
		
		glm::vec3 front = w.freelook_position + w.freelook_direction;
		
		w.view_matrix = glm::lookAt(w.freelook_position, front, glm::vec3(0.0f, 1.0f, 0.0f));
		}else{
		w.camera_position.x = cos(glm::radians(w.camera_pitch)) * sin(glm::radians(w.camera_yaw)) * w.camera_distance;
		w.camera_position.y = sin(glm::radians(w.camera_pitch)) * w.camera_distance;
		w.camera_position.z = cos(glm::radians(w.camera_pitch)) * cos(glm::radians(w.camera_yaw)) * w.camera_distance;

		glm::vec3 front = glm::normalize(glm::vec3(0.0, 0.0, 0.0) - w.camera_position);
		glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::cross(right, front);

		glm::mat4 roll_mat = glm::mat4(1.0f);
		roll_mat = glm::rotate(roll_mat, glm::radians(w.camera_roll), front);
		up = glm::vec3(roll_mat * glm::vec4(up, 1.0));
		
		w.view_matrix = glm::lookAt(glm::vec3(w.camera_position), glm::normalize(front), up);
	}
	
	shaderUniformMat4(shader, "view", w.view_matrix);
	
	w.projection_matrix = glm::perspective(glm::radians(45.0f), (float)window_width/window_height, 0.1f, 100.0f);
	shaderUniformMat4(shader, "projection", w.projection_matrix);

	glUseProgram(0);
}

void drawControllerWindows(){
	for(controller_window &w : windows){
		if(!glfwGetWindowAttrib(w.glfw_window, GLFW_ICONIFIED)){
			glfwMakeContextCurrent(w.glfw_window);
			glfwSwapInterval(w.swap_interval);

			w.deltaTime = glfwGetTime() - w.lastTime;
			w.lastTime = glfwGetTime();

			int width = 0;
			int height = 0;
			glfwGetWindowSize(w.glfw_window, &width, &height);
			glViewport(0, 0, width, height);
			//std::cout << "width = " << width << std::endl;
			//std::cout << "height = " << height << std::endl;
			
			update_camera(w, w.shader, width, height);
			update_camera(w, w.light_source_shader, width, height);
			update_camera(w, w.grid_shader, width, height);
			
			glEnable(GL_DEPTH_TEST);
			//glDisable(GL_CULL_FACE);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			if(w.wireframe)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glClearColor(w.bg_color[0] * w.bg_color[3], 
						w.bg_color[1] * w.bg_color[3], 
						w.bg_color[2] * w.bg_color[3], 
						1.0f * w.bg_color[3]);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			//Draw the Grid
			if (w.grid){
				glBindVertexArray(w.grid_vao);
				glUseProgram(w.grid_shader);	
				glEnableVertexAttribArray(0);
				glm::mat4 grid_model = glm::mat4(1.0f);
				grid_model = glm::translate(grid_model, glm::vec3(-50.0f, 0.0f, -50.0f));
				grid_model = glm::scale(grid_model, glm::vec3(100.0f, 0.0f, 100.0f));
				shaderUniformMat4(w.grid_shader, "model", grid_model);
				glDrawElements(GL_LINES, w.grid_length, GL_UNSIGNED_INT, NULL);
			}

			glBindVertexArray(w.lighting_vao);
			
			glUseProgram(w.light_source_shader);
			
			//Draw Point Light Source
			for(point_light p : w.point_lights){
				if(!p.hide){
					shaderUniformVec3(w.light_source_shader, "lightColor", glm::vec3(p.color[0], p.color[1], p.color[2]));
					glm::mat4 light_source_model = glm::mat4(1.0f);
					light_source_model = glm::translate(light_source_model, p.position); 
					light_source_model = glm::scale(light_source_model, glm::vec3(0.2f)); 
					shaderUniformMat4(w.light_source_shader, "model", light_source_model);	
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}

			//Draw Spot Light Source
			for(spot_light s : w.spot_lights){
				if(!s.hide){
					shaderUniformVec3(w.light_source_shader, "lightColor", glm::vec3(s.color[0], s.color[1], s.color[2]));
					glm::mat4 light_source_model = glm::mat4(1.0f);
					light_source_model = glm::translate(light_source_model, s.position); 
					light_source_model = glm::rotate(light_source_model, glm::radians(s.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
					light_source_model = glm::rotate(light_source_model, glm::radians(s.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
					light_source_model = glm::scale(light_source_model, glm::vec3(0.1f, 0.1f, 0.3f)); 
					shaderUniformMat4(w.light_source_shader, "model", light_source_model);	
					glDrawArrays(GL_TRIANGLES, 0, 36);
				}
			}

			//Draw Lighting Subject Cubes
			glUseProgram(w.shader);
			
			if(w.freelook)
				shaderUniformVec3(w.shader, "viewPos", w.freelook_position);
			else
				shaderUniformVec3(w.shader, "viewPos", w.camera_position);

			shaderUniformFloat(w.shader, "time", (float)glfwGetTime());

			//direct lights
			shaderUniformInt(w.shader, "direct_lights", (int)w.direct_lights.size());
			for (unsigned i=0; i<w.direct_lights.size(); i++){
				std::string name = "dirLights[";
				name.append(std::to_string(i));
				name.append("]");
				shaderUniformVec3(w.shader, std::string(name).append(".direction").c_str(), w.direct_lights[i].direction);
				shaderUniformVec3(w.shader, std::string(name).append(".ambient").c_str(), 
				glm::vec3(w.direct_lights[i].color[0] * w.direct_lights[i].ambient,
						w.direct_lights[i].color[1] * w.direct_lights[i].ambient,
						w.direct_lights[i].color[2] * w.direct_lights[i].ambient));
				shaderUniformVec3(w.shader, std::string(name).append(".diffuse").c_str(), 
				glm::vec3(w.direct_lights[i].color[0] * w.direct_lights[i].diffuse,
						w.direct_lights[i].color[1] * w.direct_lights[i].diffuse,
						w.direct_lights[i].color[2] * w.direct_lights[i].diffuse));
				shaderUniformVec3(w.shader, std::string(name).append(".specular").c_str(), 
				glm::vec3(w.direct_lights[i].color[0] * w.direct_lights[i].specular,
						w.direct_lights[i].color[1] * w.direct_lights[i].specular,
						w.direct_lights[i].color[2] * w.direct_lights[i].specular));
			}

			//point lights
			shaderUniformInt(w.shader, "point_lights", (int)w.point_lights.size());
			for (unsigned i=0; i<w.point_lights.size(); i++){
				std::string name = "pointLights[";
				name.append(std::to_string(i));
				name.append("]");
				shaderUniformFloat(w.shader, std::string(name).append(".constant").c_str(),  w.point_lights[i].constant - w.point_lights[i].intensity);
				shaderUniformFloat(w.shader, std::string(name).append(".linear").c_str(), 	 w.point_lights[i].linear);
				shaderUniformFloat(w.shader, std::string(name).append(".quadratic").c_str(), w.point_lights[i].quadratic);	
				shaderUniformVec3(w.shader,  std::string(name).append(".position").c_str(),  w.point_lights[i].position);
				shaderUniformVec3(w.shader,  std::string(name).append(".ambient").c_str(),   w.point_lights[i].ambient);
				shaderUniformVec3(w.shader,  std::string(name).append(".diffuse").c_str(),   w.point_lights[i].diffuse);
				shaderUniformVec3(w.shader,  std::string(name).append(".specular").c_str(),  w.point_lights[i].specular);
			}
			
			//spot lights
			shaderUniformInt(w.shader, "spot_lights", (int)w.spot_lights.size());
			for (unsigned i=0; i<w.spot_lights.size(); i++){
				std::string name = "spotLights[";
				name.append(std::to_string(i));
				name.append("]");
				shaderUniformVec3(w.shader,  std::string(name).append(".position").c_str(),     w.spot_lights[i].position);
				shaderUniformVec3(w.shader,  std::string(name).append(".direction").c_str(),    w.spot_lights[i].direction);
				shaderUniformFloat(w.shader, std::string(name).append(".cutoff").c_str(),       glm::cos(glm::radians(w.spot_lights[i].cutoff)));
				shaderUniformFloat(w.shader, std::string(name).append(".outer_cutoff").c_str(), glm::cos(glm::radians(w.spot_lights[i].cutoff + (w.spot_lights[i].outer_cutoff * 0.2f))));
				shaderUniformFloat(w.shader, std::string(name).append(".constant").c_str(),     w.spot_lights[i].constant - w.spot_lights[i].intensity);
				shaderUniformFloat(w.shader, std::string(name).append(".linear").c_str(), 	    w.spot_lights[i].linear);
				shaderUniformFloat(w.shader, std::string(name).append(".quadratic").c_str(),    w.spot_lights[i].quadratic);	
				shaderUniformVec3(w.shader,  std::string(name).append(".ambient").c_str(),      w.spot_lights[i].ambient);
				shaderUniformVec3(w.shader,  std::string(name).append(".diffuse").c_str(),      w.spot_lights[i].diffuse);
				shaderUniformVec3(w.shader,  std::string(name).append(".specular").c_str(),     w.spot_lights[i].specular);
			}
			
			w.model.motion_matrix = w.gyro_matrix;
			
			drawModel(w.model, w.shader);
			
			glUseProgram(0);
			
			glfwSwapBuffers(w.glfw_window);
		}
	}
}
