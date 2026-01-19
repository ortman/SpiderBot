#ifndef _SHADER_FLAT_HPP_
#define _SHADER_FLAT_HPP_

#include "Shader.hpp"

class ShaderFlat : public Shader {
private:
	virtual void Load() override {
		// Vertex Shader
		const char* vs_src = R"(
			#version 330 core
			layout(location = 0) in vec3 pos;
			
			uniform mat4 u_projection_view;
			uniform mat4 u_model;
						
			void main() {
				gl_Position = u_projection_view * u_model * vec4(pos, 1.0);
			}
		)";
		// Fragment Shader
		const char* fs_src = R"(
			#version 330 core
			
			out vec4 FragColor;
			
			uniform vec4 u_color;
						
			void main() {
				FragColor = u_color;
			}
		)";

		glAttachShader(program_id, compile(GL_VERTEX_SHADER, vs_src));
		glAttachShader(program_id, compile(GL_FRAGMENT_SHADER, fs_src));
	}
};

#endif