#ifndef _SHADER_GET_NODE_HPP_
#define _SHADER_GET_NODE_HPP_

#include "Shader.hpp"

class ShaderGetNode : public Shader {
private:
	virtual void Load() override {
		// Vertex Shader
		const char* vs_src = R"(
			layout(location = 0) in vec3 pos;
			uniform mat4 u_projection_view;
			uniform mat4 u_model;
			
			void main() {
				gl_Position = u_projection_view * u_model * vec4(pos, 1.0);
			}
		)";
		// Fragment Shader
		const char* fs_src = R"(
			uniform int u_id;
			out vec4 fragColor;
			
			void main() {
				float r = float((u_id & 0x0000FF) >> 0) / 255.0;
				float g = float((u_id & 0x00FF00) >> 8) / 255.0;
				float b = float((u_id & 0xFF0000) >> 16) / 255.0;
				fragColor = vec4(r, g, b, 1.0);
			}
		)";
		glAttachShader(program_id, compile(GL_VERTEX_SHADER, vs_src));
		glAttachShader(program_id, compile(GL_FRAGMENT_SHADER, fs_src));
	}
public:
	void SetId(int id) { Set("u_id", id); }
};

#endif