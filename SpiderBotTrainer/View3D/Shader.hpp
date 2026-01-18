#ifndef _SHADER_HPP_
#define _SHADER_HPP_

class Shader {
private:
	virtual void Load() = 0;
	
protected:
	GLuint id = 0;
	GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
	}
	
public:
	GLuint GetId() {
		if (id == 0) {
			id = glCreateProgram();
			LOG("Create shader " << typeid(*this).name() << ": " << id);
			Load();
			glLinkProgram(id);
	    char buffer[512];
			glGetProgramInfoLog(id, 512, NULL, buffer);
			if (strlen(buffer)) LOG("Shader Error: " << buffer);
		}
		return id;
	}
};

#endif