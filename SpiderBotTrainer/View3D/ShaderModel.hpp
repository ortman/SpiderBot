#ifndef _SHADER_MODEL_HPP_
#define _SHADER_MODEL_HPP_

#include "Shader.hpp"

class ShaderModel : public Shader {
private:
	virtual void Load() {
		const char* vs_src = R"(
			#version 330 core
			layout(location = 0) in vec3 aPos;
			layout(location = 1) in vec3 aNormal;
			
			uniform mat4 u_projection_view;
			uniform mat4 u_model;
			
			out vec3 vFragPos;
			out vec3 vNormal;
			
			void main() {
		    vFragPos = vec3(u_model * vec4(aPos, 1.0));
		    vNormal = mat3(transpose(inverse(u_model))) * aNormal;
		    gl_Position = u_projection_view * vec4(vFragPos, 1.0);
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
			
			// lights
			vec3 lightPos      = u_viewPos;
	    vec3 lightAmbient  = vec3(0.3, 0.3, 0.3);
	    vec3 lightDiffuse  = vec3(0.6, 0.6, 0.6);
	    vec3 lightSpecular = vec3(0.5, 0.5, 0.5);
	
	    // materials
	    vec3 matAmbient    = u_color.rgb * 0.5;
	    vec3 matDiffuse    = u_color.rgb;
	    vec3 matSpecular   = vec3(0.8, 0.8, 0.8);
	    float matShininess = 50.0;
			
			void main() {
		    vec3 norm = normalize(vNormal);
		    vec3 lightDir = normalize(lightPos - vFragPos);
		    vec3 viewDir = normalize(u_viewPos - vFragPos);
		
		    vec3 ambient = lightAmbient * matAmbient;
		
		    float diff = max(dot(norm, lightDir), 0.0);
		    vec3 diffuse = lightDiffuse * (diff * matDiffuse);
		
		    vec3 reflectDir = reflect(-lightDir, norm);
		    float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
		    vec3 specular = lightSpecular * (spec * matSpecular);
		
		    vec3 result = ambient + diffuse + specular;
		    
		    FragColor = vec4(result, u_color.a);
			}
		)";
		
		GLuint vs = compile(GL_VERTEX_SHADER, vs_src);
    glAttachShader(id, vs);
    
    GLuint fs = compile(GL_FRAGMENT_SHADER, fs_src);
    glAttachShader(id, fs);
	}
};

#endif