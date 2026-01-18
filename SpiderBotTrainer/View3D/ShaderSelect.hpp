#ifndef _SHADER_SELECT_HPP_
#define _SHADER_SELECT_HPP_

#include "Shader.hpp"

class ShaderSelect : public Shader {
private:
	virtual void Load() override {
		// Vertex Shader
		const char* vs_select_src = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    
    uniform mat4 u_projection_view;
    uniform mat4 u_model;
    
    out vec3 vFragPos;
    out vec3 vNormal;
    
    void main() {
        vFragPos = vec3(u_model * vec4(aPos, 1.0));
        // Используем mat3 для нормалей, чтобы не учитывать смещение
        vNormal = mat3(transpose(inverse(u_model))) * aNormal;
        gl_Position = u_projection_view * vec4(vFragPos, 1.0);
    }
)";

// Fragment Shader со свечением краев (Fresnel Effect)
const char* fs_select_src = R"(
    #version 330 core
    in vec3 vFragPos;
    in vec3 vNormal;
    
    out vec4 FragColor;
    
    uniform vec4 u_color;
    uniform vec3 u_viewPos;
    
    void main() {
        // Базовое освещение (упрощенное)
        vec3 norm = normalize(vNormal);
        vec3 viewDir = normalize(u_viewPos - vFragPos);
        
        // --- ЭФФЕКТ СВЕЧЕНИЯ КРАЕВ ---
        // dot(norm, viewDir) = 1.0 (смотрим в упор), 0.0 (смотрим на край)
        float fresnel = 1.0 - max(dot(norm, viewDir), 0.0);
        
        // Возводим в степень для управления "толщиной" ободка (3.0 - тонкий, 1.0 - мягкий)
        fresnel = pow(fresnel, 2.0);
        
        // Цвет свечения (например, ярко-белый или золотистый)
        vec3 rimColor = vec3(1.0, 1.0, 1.0);
        vec3 glow = rimColor * fresnel * 0.5; // 1.5 - интенсивность
        
        // Стандартная модель (Ambient + Diffuse)
        vec3 lightDir = viewDir; // свет от камеры
        float diff = max(dot(norm, lightDir), 0.0) * 0.6;
        vec3 ambient = vec3(0.3) * u_color.rgb;
        vec3 diffuse = diff * u_color.rgb;
        
        // Финальный цвет: основной + свечение
        vec3 result = ambient + diffuse + glow;
        
        // Можно также добавить пульсацию, если передать u_time
        FragColor = vec4(result, u_color.a);
    }
)";

    glAttachShader(id, compile(GL_VERTEX_SHADER, vs_select_src));
    glAttachShader(id, compile(GL_FRAGMENT_SHADER, fs_select_src));
	}
};

#endif