#ifndef _BBOXF_HPP_

#include <plugin/glm/glm.hpp>
#include <plugin/glm/gtc/quaternion.hpp>

struct Bboxf {
	vec3 min, max;
	
	static GLuint vao;
	static GLuint vbo;

	Bboxf& operator+=(const Bboxf& b) {
		min = glm::min(min, b.min);
		max = glm::max(max, b.max);
		return *this;
	}

	Bboxf& operator+=(const vec3& p) {
		min = glm::min(min, p);
		max = glm::max(max, p);
		return *this;
	}

	Bboxf operator*(float f) const {
		return {min * f, max * f};
	}

	Bboxf operator*(const vec3& p) const {
		return {min * p, max * p};
	}

	Bboxf Transform(const mat4& m) const {
		return Bboxf{m * vec4(min, 1.0f), m * vec4(max, 1.0f)};
	}

	Bboxf Translate(const vec3& p) const {
		return Bboxf{min + p, max + p};
	}

	Bboxf Rotate(const vec3& angleDeg) const {
		if (glm::length(angleDeg) > 0.01) {
			vec3 rMin = Rotate(min, angleDeg);
			vec3 rMax = Rotate(max, angleDeg);
			rMin = glm::min(rMin, rMax); rMax = glm::max(rMin, rMax);
			vec3 p = vec3(min.x, min.y, max.z);
			p = Rotate(p, angleDeg);
			rMin = glm::min(rMin, p); rMax = glm::max(p, rMax);
			p = vec3(min.x, max.y, min.z);
			p =Rotate(p, angleDeg);
			rMin = glm::min(rMin, p); rMax = glm::max(p, rMax);
			p = vec3(min.x, max.y, max.z);
			p = Rotate(p, angleDeg);
			rMin = glm::min(rMin, p); rMax = glm::max(p, rMax);
			p = vec3(max.x, min.y, min.z);
			p = Rotate(p, angleDeg);
			rMin = glm::min(rMin, p); rMax = glm::max(p, rMax);
			p = vec3(max.x, min.y, max.z);
			p = Rotate(p, angleDeg);
			rMin = glm::min(rMin, p); rMax = glm::max(p, rMax);
			p = vec3(max.x, max.y, min.z);
			p = Rotate(p, angleDeg);
			rMin = glm::min(rMin, p); rMax = glm::max(p, rMax);
			
			return Bboxf{rMin, rMax};
		}
		return *this;
	}

	vec3 GetSize() const {
		return vec3(
			std::abs(min.x - max.x),
			std::abs(min.y - max.y),
			std::abs(min.z - max.z)
		);
	}

	bool isEmpty() const {
		return glm::length(min + max) < 0.001;
	}
	
	void GLPaint() const {
		if (!vao) {
			Vector<vec3> points(24);
			points[0]  = vec3(-0.5f, -0.5f, -0.5f); points[1]  = vec3(-0.5f,  0.5f, -0.5f);
			points[2]  = vec3(-0.5f,  0.5f, -0.5f); points[3]  = vec3( 0.5f,  0.5f, -0.5f);
			points[4]  = vec3( 0.5f,  0.5f, -0.5f); points[5]  = vec3( 0.5f, -0.5f, -0.5f);
			points[6]  = vec3( 0.5f, -0.5f, -0.5f); points[7]  = vec3(-0.5f, -0.5f, -0.5f);

			points[8]  = vec3(-0.5f, -0.5f,  0.5f);  points[9] = vec3(-0.5f,  0.5f,  0.5f);
			points[10] = vec3(-0.5f,  0.5f,  0.5f); points[11] = vec3( 0.5f,  0.5f,  0.5f);
			points[12] = vec3( 0.5f,  0.5f,  0.5f); points[13] = vec3( 0.5f, -0.5f,  0.5f);
			points[14] = vec3( 0.5f, -0.5f,  0.5f); points[15] = vec3(-0.5f, -0.5f,  0.5f);

			points[16] = vec3(-0.5f, -0.5f, -0.5f); points[17] = vec3(-0.5f, -0.5f,  0.5f);
			points[18] = vec3(-0.5f,  0.5f, -0.5f); points[19] = vec3(-0.5f,  0.5f,  0.5f);
			points[20] = vec3( 0.5f, -0.5f, -0.5f); points[21] = vec3( 0.5f, -0.5f,  0.5f);
			points[22] = vec3( 0.5f,  0.5f, -0.5f); points[23] = vec3( 0.5f,  0.5f,  0.5f);
			
			Shader::VaoLinesInit(points, vao, vbo);
		}
		Shader::DrawLines(vao, vbo, 24);
	}
	
private:
	vec3 Rotate(const vec3& v, const vec3& angle) const {
		glm::quat q(glm::radians(angle));
		return q * v;
	}
};

GLuint Bboxf::vao = 0;
GLuint Bboxf::vbo = 0;

#endif