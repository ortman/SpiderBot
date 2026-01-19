#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include <plugin/glm/glm.hpp>
using namespace glm;

class Shader {
private:
	virtual void Load() = 0;
	
protected:
	GLuint program_id = 0;
	GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
	}
	
public:
	GLuint GetProgramId() {
		if (program_id == 0) {
			program_id = glCreateProgram();
			LOG("Create shader " << typeid(*this).name() << ": " << program_id);
			Load();
			glLinkProgram(program_id);
	    char buffer[512];
			glGetProgramInfoLog(program_id, 512, NULL, buffer);
			if (strlen(buffer)) LOG("Shader Error: " << buffer);
		}
		return program_id;
	}
	
	void Use() {
		glUseProgram(GetProgramId());
	}
	
	void Set(const char* var, int i) {
		GLint u_v = glGetUniformLocation(program_id, var);
		glUniform1i(u_v, i);
	}
	
	void Set(const char* var, const mat4& m) {
		GLint u_v = glGetUniformLocation(program_id, var);
		glUniformMatrix4fv(u_v, 1, GL_FALSE, &m[0][0]);
	}
	
	void Set(const char* var, const vec3& m) {
		GLint u_v = glGetUniformLocation(program_id, var);
		glUniform3f(u_v, m.x, m.y, m.z);
	}
	
	void Set(const char* var, const vec4& m) {
		GLint u_v = glGetUniformLocation(program_id, var);
		glUniform4f(u_v, m[0], m[1], m[2], m[3]);
	}
	
	void SetPV(const mat4& m) { Set("u_projection_view", m);	}
	void SetViewPos(const vec3& p) { Set("u_viewPos", p); }
	void SetModel(const mat4& m) { Set("u_model", m);	}
	void SetColor(const vec4& color) { Set("u_color", color); }
	void SetColor(const Upp::Color& color) {
		SetColor(vec4(
			(float)color.GetR() / 255.f,
			(float)color.GetG() / 255.f,
			(float)color.GetB() / 255.f,
			1.0f
		));
	}

	static void VaoTrianglesInit(UPP::Vector<vec3>& points, GLuint& vao, GLuint& vbo) {
		int pointsCount = points.GetCount();
		if (pointsCount == 0) return;
		
		if (!vao) glGenVertexArrays(1, &vao);
		if (!vbo) glGenBuffers(1, &vbo);
		
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		
		glBufferData(GL_ARRAY_BUFFER, pointsCount * sizeof(vec3), points.begin(), GL_STATIC_DRAW);
		
		GLsizei stride = 2 * sizeof(vec3);
		// Attribute 0: Vertex (3 float)
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		
		// Attribute 1: Normal (3 float)
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(vec3)));
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	
	static void VaoLinesInit(UPP::Vector<vec3>& points, GLuint& vao, GLuint& vbo) {
		int pointsCount = points.GetCount();
		if (pointsCount == 0) return;
		
		if (!vao) glGenVertexArrays(1, &vao);
		if (!vbo) glGenBuffers(1, &vbo);
		
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		
		glBufferData(GL_ARRAY_BUFFER, pointsCount * sizeof(vec3), points.begin(), GL_STATIC_DRAW);
		
		// Attribute 0: Vertex (3 float)
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	static void VaoDeinit(GLuint& vao, GLuint& vbo) {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
		vao = 0;
		vbo = 0;
	}
	
	static void DrawObject(GLuint vao, GLuint vbo, GLsizei pointsCount) {
		if (vao && vbo && pointsCount) {
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, pointsCount);
			glBindVertexArray(0);
		}
	}
	
	static void DrawLines(GLuint vao, GLuint vbo, GLsizei pointsCount) {
		if (vao && vbo && pointsCount) {
			glBindVertexArray(vao);
			glDrawArrays(GL_LINES, 0, pointsCount);
			glBindVertexArray(0);
		}
	}
};

#endif