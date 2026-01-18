#ifndef _SHADER_SELECT_HPP_
#define _SHADER_SELECT_HPP_

#include "Shader.hpp"

class ShaderSelect : public Shader {
private:
	virtual void Load() override {
		// Vertex Shader
		const char* vs_src = R"(
			#version 330 core
			layout(location = 0) in vec3 aPos;
			layout(location = 1) in vec3 aNormal;
			
			uniform mat4 u_projection_view;
			uniform mat4 u_model;
			
			void main() {
				vec3 pos = aPos + aNormal * 0.5; // outline width
				gl_Position = u_projection_view * u_model * vec4(pos, 1.0);
			}
		)";

		// Fragment Shader
		const char* fs_src = R"(
			#version 330 core
			in vec3 vFragPos;
			in vec3 vNormal;
			
			out vec4 FragColor;
			
			uniform vec4 u_color;
			uniform vec3 u_viewPos;
			
			void main() {
				vec3 brightColor = u_color.rgb * 5.0;
				FragColor = vec4(brightColor, 1.0);
			}
		)";

    glAttachShader(id, compile(GL_VERTEX_SHADER, vs_src));
    glAttachShader(id, compile(GL_FRAGMENT_SHADER, fs_src));
	}
};

#endif