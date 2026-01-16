#ifndef _BBOXF_HPP_

#include <plugin/glm/glm.hpp>
#include <plugin/glm/gtc/quaternion.hpp>

struct Bboxf {
	vec3 min, max;

	Bboxf& operator+=(const Bboxf& b) {
		min = glm::min(min, b.min);
		//glm::min(min, b.max);
		//glm::max(max, b.min);
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

	Bboxf Translate(const vec3& p) {
		return Bboxf{min + p, max + p};
	}

	Bboxf Rotate(const vec3& angleDeg) {
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

	vec3 GetSize() {
		return vec3(
			std::abs(min.x - max.x),
			std::abs(min.y - max.y),
			std::abs(min.z - max.z)
		);
	}

	bool isEmpty() const {
		return glm::length(min + max) < 0.001;
	}
	
private:
	vec3 Rotate(const vec3& v, const vec3& angle) {
		glm::quat q(glm::radians(angle));
		return q * v;
	}
};

#endif