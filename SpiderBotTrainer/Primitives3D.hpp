#ifndef _PRIMITIVES_3D_HPP_
#define _PRIMITIVES_3D_HPP_

#include "Node3D.hpp"

struct Square3D : public Node3D {
private:
	void AddTriangle(vec3 p1, vec3 p2, vec3 p3, vec3 n) {
		points.Add(p1); points.Add(n);
		points.Add(p2); points.Add(n);
		points.Add(p3); points.Add(n);
	}
	
	void AddFace(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec3 n) {
		AddTriangle(p1, p2, p3, n);
		AddTriangle(p1, p3, p4, n);
	}
	
public:
	Square3D(float cx, float cy, float cz) : Node3D() {
		float x = cx / 2.0f;
		float y = cy / 2.0f;
		float z = cz / 2.0f;

    AddFace({-x, -y,  z}, { x, -y,  z}, { x,  y,  z}, {-x,  y,  z}, {0, 0, 1});
    AddFace({ x, -y, -z}, {-x, -y, -z}, {-x,  y, -z}, { x,  y, -z}, {0, 0, -1});
    AddFace({-x,  y,  z}, { x,  y,  z}, { x,  y, -z}, {-x,  y, -z}, {0, 1, 0});
    AddFace({-x, -y, -z}, { x, -y, -z}, { x, -y,  z}, {-x, -y,  z}, {0, -1, 0});
    AddFace({ x, -y,  z}, { x, -y, -z}, { x,  y, -z}, { x,  y,  z}, {1, 0, 0});
    AddFace({-x, -y, -z}, {-x, -y,  z}, {-x,  y,  z}, {-x,  y, -z}, {-1, 0, 0});

		bbox = {{-x, -y, -z}, {x, y, z}};
	}
private:
	virtual Node3D& LoadSTL(const String& filepath) { return *this; };
};

#endif