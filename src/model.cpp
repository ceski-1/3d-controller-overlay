#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void loadModel(Model &m, std::string path){
    m.path = path;
    for (size_t i = m.meshes.size(); i > 0; i--) {
        m.meshes.pop_back();
    }
    for (size_t i = 0; i < SDL_arraysize(mesh_filenames); i++) {
        Mesh new_mesh;
        switch ((mesh_idx)i) {
            case mesh_idx::top_shell:
                new_mesh.material.color[0] = 0.8f;
                new_mesh.material.color[1] = 0.8f;
                new_mesh.material.color[2] = 0.8f;
                new_mesh.material.specular = 0.2f;
                break;
            case mesh_idx::left_stick_ring:
            case mesh_idx::right_stick_ring:
            case mesh_idx::left_stick_cap:
            case mesh_idx::right_stick_cap:
                new_mesh.material.specular = 0.05f;
                new_mesh.material.shininess = 10.0f;
                break;
            default:
                break;
        }
        std::string file_path = path;
        file_path.append("/");
        file_path.append(mesh_filenames[i]);
        loadMesh(new_mesh, file_path);
        m.meshes.push_back(new_mesh);
    }
    std::string info_file_path = path;
    info_file_path.append("/info.txt");
    readInfo(m, info_file_path);
}

bool isFloat(std::string myString){
    std::istringstream iss(myString);
    float f;
    iss >> std::noskipws >> f;
    return iss.eof() && !iss.fail(); 
}

void loadMesh(Mesh &m, std::string path){
    std::ifstream ifs = std::ifstream(path);

    std::vector<vertex_position> positions;
    std::vector<vertex_normal> normals;
    std::vector<vertex_texcoord> texcoords;

    std::vector<Vertex> vertices;
    std::vector<int> indices;
    
    while (ifs){
        std::vector<std::string> words;
        std::string line;
        std::string word;
        
        std::getline(ifs, line);
        std::stringstream line_stream(line);
        
        while (std::getline(line_stream, word, ' ')){
            words.push_back(word);
        }

        if(words.size() > 3 && words[0] == "v"){
            //std::cout << "line describes a position." << std::endl;
            vertex_position pos;
            pos.x =  std::stof(words[1]);
            pos.y =  std::stof(words[2]);
            pos.z =  std::stof(words[3]);
            positions.push_back(pos);
        }

        if(words.size() > 3 && words[0] == "vn"){
            //std::cout << "line describes a normal." << std::endl;
            vertex_normal norm;
            norm.x = std::stof(words[1]);
            norm.y = std::stof(words[2]);
            norm.z = std::stof(words[3]);
            normals.push_back(norm);
        }

        if(words.size() > 2 && words[0] == "vt"){
            //std::cout << "line describes a texture coordinate." << std::endl;
            vertex_texcoord tex;
            tex.x = std::stof(words[1]);
            tex.y = std::stof(words[2]);
            texcoords.push_back(tex);
        }

        if(words.size() > 3 && words[0] == "f"){
            //std::cout << "line describes a Face." << std::endl;
            for(unsigned long i = 1; i < (unsigned long)(words.size() - 2); i++) {
                int num_verts = (int)vertices.size();
                indices.push_back(num_verts);
                indices.push_back(num_verts + i);
                indices.push_back(num_verts + i + 1);
            }
            for(unsigned long i = 1; i < (unsigned long)words.size(); i++) {
                std::vector<int> ind;
                std::string value;
                std::stringstream word_stream(words[i]);
                while (std::getline(word_stream, value, '/')){
                    if(value == ""){
                        ind.push_back(-1);
                    }else{
                        ind.push_back(std::stoi(value) - 1);
                    }
                }
                Vertex v;
                v.position  = ind[0];
                v.texcoord = ind[1];
                v.normal = ind[2];
                vertices.push_back(v);
            }
        }
	}

    const size_t vertex_data_size = sizeof(GLfloat) * vertices.size() * 8;
    GLfloat *vertex_data = (GLfloat *)SDL_malloc(vertex_data_size);
    for(unsigned long i = 0; i<vertices.size(); i++){
        vertex_data[0 + (8 * i)] = positions[vertices[i].position].x;
        vertex_data[1 + (8 * i)] = positions[vertices[i].position].y;
        vertex_data[2 + (8 * i)] = positions[vertices[i].position].z;
        vertex_data[3 + (8 * i)] = normals[vertices[i].normal].x;
        vertex_data[4 + (8 * i)] = normals[vertices[i].normal].y;
        vertex_data[5 + (8 * i)] = normals[vertices[i].normal].z;
        vertex_data[6 + (8 * i)] = texcoords[vertices[i].texcoord].x;
        vertex_data[7 + (8 * i)] = texcoords[vertices[i].texcoord].y;
    }
    
    m.elements = (GLuint)indices.size();
    const size_t index_data_size = sizeof(GLuint) * m.elements;
    GLuint *index_data = (GLuint *)SDL_malloc(index_data_size);
    for(unsigned long i = 0; i<m.elements; i++){
        index_data[i] = indices[i];
    }
    
    glGenVertexArrays(1, &m.vao);
	glGenBuffers(1, &m.vbo);
	glBindVertexArray(m.vao);

	glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
	glBufferData(GL_ARRAY_BUFFER,
                 vertex_data_size,
                 vertex_data,
                 GL_STATIC_DRAW);
    //vertex attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);
    //normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3* sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
    //texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6* sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

    glGenBuffers(1, &m.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_data_size, index_data, GL_STATIC_DRAW);

	glBindVertexArray(0);

	SDL_free(vertex_data);
	SDL_free(index_data);
}

void readInfo(Model &m, std::string path){
    std::ifstream info_file = std::ifstream(path);
    if (!info_file){
        std::cout << "Uh oh, info file could not be opened for reading!" << std::endl;
    }else{    
        while (info_file){
            std::string line;
            std::getline(info_file, line);
            for (size_t i = 0; i < SDL_arraysize(mesh_filenames); i++) {
                if (line == mesh_filenames[i]) {
                    for(int p=0; p<3; p++){ 
                        std::getline(info_file, line);
                        if(isFloat(line))
                            m.meshes[i].position[p] = std::stof(line);
                    }
                    for(int t=0; t<3; t++){ 
                        std::getline(info_file, line);
                        if(isFloat(line))
                            m.meshes[i].travel[t] = std::stof(line);
                    }
                    for(int po=0; po<3; po++){ 
                        std::getline(info_file, line);
                        if(isFloat(line))
                            m.meshes[i].popup_offset[po] = std::stof(line);
                    }
                    for(int pr=0; pr<3; pr++){ 
                        std::getline(info_file, line);
                        if(isFloat(line))
                            m.meshes[i].popup_rotation[pr] = std::stof(line);
                    }
                    std::getline(info_file, line);
                    if(isFloat(line))
                        m.meshes[i].trigger_max = std::stof(line);
                    std::getline(info_file, line);
                    if(isFloat(line))
                        m.meshes[i].stick_max = std::stof(line);
                    std::getline(info_file, line);
                    if(isFloat(line))
                        m.meshes[i].touch_width = std::stof(line);
                    std::getline(info_file, line);
                    if(isFloat(line))
                        m.meshes[i].touch_height = std::stof(line);
                    for (int unused = 0; unused < 2; unused++) {
                        std::getline(info_file, line);
                    }
                }
            }
        }
    }
}

void writeInfo(Model &m, std::string path){
    std::string file_path = path;
    file_path.append("/info.txt");
    std::ofstream info_file = std::ofstream(file_path);
    for (int i = 0; i < SDL_arraysize(mesh_filenames); i++) {
        info_file << mesh_filenames[i].c_str() << "\n";
        for(int pos=0; pos<3; pos++){ 
            info_file << m.meshes[i].position[pos] << "\n";
        }
        for(int t=0; t<3; t++){ 
            info_file << m.meshes[i].travel[t] << "\n";
        }
        for(int po=0; po<3; po++){ 
            info_file << m.meshes[i].popup_offset[po] << "\n";
        }
        for(int pr=0; pr<3; pr++){ 
            info_file << m.meshes[i].popup_rotation[pr] << "\n";
        }
        info_file << m.meshes[i].trigger_max << "\n";
        info_file << m.meshes[i].stick_max << "\n";
        info_file << m.meshes[i].touch_width << "\n";
        info_file << m.meshes[i].touch_height << "\n";
        info_file << m.meshes[i].unused1 << "\n";
        info_file << m.meshes[i].unused2 << "\n";
    }
}

void deleteTexture(GLuint &id){
    glDeleteTextures(1, &id);
    id = 0;
}

void loadTexture(GLuint &id, std::string path){
    if(id == 0)
        glGenTextures(1, &id);

    glBindTexture(GL_TEXTURE_2D, id); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    GLenum format = GL_RED; // nrChannels == 1
    if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
		format = GL_RGBA;

    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }else{
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void drawMesh(Mesh m, glm::mat4 motion, GLuint shader){
    glBindVertexArray(m.vao);
		
	shaderUniformInt(shader, "num_textures", (int)m.textures.size());
	for(size_t i=0; i<m.textures.size(); i++){
        std::string name = "textures[";
		name.append(std::to_string(i));
		name.append("]");
	    shaderUniformInt(shader, std::string(name).append(".id").c_str(), (int)i);
        shaderUniformInt(shader, std::string(name).append(".type").c_str(), m.textures[i].type);
        shaderUniformFloat(shader, std::string(name).append(".offsetX").c_str(), m.textures[i].offsetX);
        shaderUniformFloat(shader, std::string(name).append(".offsetY").c_str(), m.textures[i].offsetY);
        shaderUniformFloat(shader, std::string(name).append(".scaleX").c_str(), m.textures[i].scaleX);
        shaderUniformFloat(shader, std::string(name).append(".scaleY").c_str(), m.textures[i].scaleY);
        shaderUniformFloat(shader, std::string(name).append(".rotation").c_str(), m.textures[i].rotation);
        glActiveTexture((GLenum)(GL_TEXTURE0 + i));
	    glBindTexture(GL_TEXTURE_2D, m.textures[i].id);
    }
    
    shaderUniformFloat(shader, "material.ambient",  m.material.ambient);
    shaderUniformFloat(shader, "material.diffuse",  m.material.diffuse);
    shaderUniformFloat(shader, "material.specular", m.material.specular);
    shaderUniformVec3(shader, "material.color", glm::vec3(m.material.color[0], m.material.color[1], m.material.color[2]));
    shaderUniformFloat(shader, "material.shininess", m.material.shininess);
 
    glm::mat4 model = glm::mat4(1.0f);
    //MOTION
    model *= motion;
    //MODEL POSITION
    model = glm::translate(model, glm::vec3(m.position[0], m.position[1], m.position[2]));
    //STICK ROTATION
    model = glm::rotate(model, m.stick_X / -32767 * m.stick_max, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, m.stick_Y /  32767 * m.stick_max, glm::vec3(1.0f, 0.0f, 0.0f));
    //POPUP
    if(m.popup){
        model = glm::translate(model, glm::vec3(m.popup_offset[0], m.popup_offset[1], m.popup_offset[2]));
		model = glm::rotate(model, m.popup_rotation[0], glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, m.popup_rotation[1], glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, m.popup_rotation[2], glm::vec3(0.0f, 0.0f, 1.0f));
    }else{
        //TRAVEL 
        model = glm::translate(model, glm::vec3(m.travel[0] * m.press, m.travel[1] * m.press, m.travel[2] * m.press));
        //TRIGGER PULL
        model = glm::rotate(model, m.pull / -32767 * m.trigger_max, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    //TOUCH POINTS
    if(m.touch_state){
        model = glm::translate(model, glm::vec3((m.touch_X * m.touch_width) - m.touch_width * 0.5, 0, (m.touch_Y * m.touch_height) - m.touch_height * 0.5));
    }
    
	shaderUniformVec3(shader, "highlight_color", glm::vec3(m.material.highlight[0], m.material.highlight[1], m.material.highlight[2]));
    shaderUniformFloat(shader, "highlight_value", m.highlight_value);
   
    shaderUniformMat4(shader, "model", model);
	glm::mat3 normal = glm::mat3(model);
	shaderUniformMat3(shader, "normal_model", glm::transpose(glm::inverse(normal)));
    if(m.visible){
	    glDrawElements(GL_TRIANGLES, m.elements, GL_UNSIGNED_INT, 0);
    }	
}

void drawModel(Model m, GLuint shader) {
    for (Mesh mesh : m.meshes) {
        m.meshes[(int)mesh_idx::left_shoulder].popup = m.popup_bumpers;
        m.meshes[(int)mesh_idx::right_shoulder].popup = m.popup_bumpers;
        m.meshes[(int)mesh_idx::left_trigger].popup = m.popup_triggers;
        m.meshes[(int)mesh_idx::right_trigger].popup = m.popup_triggers;
        m.meshes[(int)mesh_idx::paddle1].popup = m.popup_paddles;
        m.meshes[(int)mesh_idx::paddle2].popup = m.popup_paddles;
        m.meshes[(int)mesh_idx::paddle3].popup = m.popup_paddles;
        m.meshes[(int)mesh_idx::paddle4].popup = m.popup_paddles;
        drawMesh(mesh, m.motion_matrix, shader);
    }
}
