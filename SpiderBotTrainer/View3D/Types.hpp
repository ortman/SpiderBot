#ifndef _VIEW3D_TYPES_HPP_
#define _VIEW3D_TYPES_HPP_

#include <Core/Core.h>

typedef struct {
	vec3 p;
	vec3 n;
} View3D_Point_t;

namespace glm {
	inline unsigned GetHashValue(const vec3& v) {
		unsigned hash = 0;
		Upp::CombineHash(hash, v.x, v.y, v.z);
		return hash;
	}
}

#endif