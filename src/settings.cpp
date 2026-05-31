#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <SDL3/SDL.h>
#include "settings.h"
#include "config.h"

static const char *pref_path = NULL;
std::filesystem::path file_path;
std::ofstream ofs;
std::ifstream ifs;	
	
void init_pref_path() {
    if (pref_path != NULL) {
        return;
    }
    pref_path = SDL_GetBasePath();
#if !defined(_WIN32)
    char *out_path = SDL_GetPrefPath("", PROJECT_SHORTNAME);
    if (out_path != NULL) {
        const char *dup_path = (const char *)SDL_strdup(out_path);
        if (dup_path != NULL) {
            pref_path = dup_path;
        }
        SDL_free(out_path);
    }
    SDL_CreateDirectory(pref_path);
#endif
}

void write_int(std::string label, int value){
    ofs << label.append("\n").c_str();
    ofs << std::to_string(value).append("\n").c_str();
}

void write_float(std::string label, float value){
    ofs << label.append("\n").c_str();
    ofs << std::to_string(value).append("\n").c_str();
}

void write_3_floats(std::string label, float value1, float value2, float value3){
    ofs << label.append("\n").c_str();
    ofs << std::to_string(value1).append("\n").c_str();
    ofs << std::to_string(value2).append("\n").c_str();
    ofs << std::to_string(value3).append("\n").c_str();
}

void write_string(std::string label, std::string value){
    label.append("\n");
    value.append("\n");
    ofs << label.c_str();
    ofs << value.c_str();
}

void write_line(std::string line){
    line.append("\n");
    ofs << line.c_str();
}

void open_ifstream(std::filesystem::path path){
    file_path = std::filesystem::path(pref_path);
	std::filesystem::path sub_path(path);
    file_path /= sub_path;
	std::filesystem::create_directory(file_path.parent_path());

	ifs = std::ifstream(file_path);
    if (!ifs){
        SDL_Log("Uh oh, file could not be opened for reading!");
    }
}

void open_ofstream(std::filesystem::path path){
    file_path = std::filesystem::path(pref_path);
	std::filesystem::path sub_path(path);
    file_path /= sub_path;
	std::filesystem::create_directory(file_path.parent_path());

	ofs = std::ofstream(file_path);
    if (!ofs){
        SDL_Log("Uh oh, file could not be opened for writing!");
    }
}

void read_file(std::vector<std::string> *lines){
    while (ifs){
        std::string line;
        std::getline(ifs, line);
        lines->push_back(line);
	}
}

void get_directory_contents(std::vector<std::filesystem::path> *files, std::string path){
    std::string dir_path = pref_path;
    dir_path.append("/");
    dir_path.append(path);
    
    std::string p_str = dir_path;
    struct stat sb;
    if (stat(dir_path.c_str(), &sb) == 0){
        for (const auto & entry : std::filesystem::directory_iterator(dir_path))
            files->push_back(entry.path());
    }else{
        std::filesystem::create_directory(dir_path);
    }
}

void list_directory(std::string path){
    std::string dir_path = pref_path;
    dir_path.append("/");
    dir_path.append(path);
    
    struct stat sb;
    if (stat(dir_path.c_str(), &sb) == 0){
        for (const auto & entry : std::filesystem::directory_iterator(dir_path))
            SDL_Log("%s", entry.path().c_str());
    }else{
        std::filesystem::create_directory(dir_path);
    }
}

void clear_directory(std::string dir)
{
    std::string dir_path = pref_path;
    dir_path.append("/");
    dir_path.append(dir);
    
    struct stat sb;
    if (stat(dir_path.c_str(), &sb) == 0){
        for (const auto& entry : std::filesystem::directory_iterator(dir_path)) 
            std::filesystem::remove_all(entry.path());
    }
}

void close_ifstream(){
    ifs.close();
}

void close_ofstream(){
    ofs.close();
}
