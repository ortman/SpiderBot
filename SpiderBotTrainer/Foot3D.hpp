#ifndef _FOOT_HPP_
#define _FOOT_HPP_

#include "Node3D.hpp"

struct Foot3D : public Node3D {
private:
	Node3D segment1, segment2, segment3;
	
public:
	Foot3D() : Node3D() {
    segment1.LoadSTL("model/Segment1.stl").SetColor(LtBlue).Translate({96.5f, 60.0f, -1.8f});
    segment2.LoadSTL("model/Segment2.stl").SetColor(LtCyan).Translate({96.5f, 60.0f, -1.8f});
    segment3.LoadSTL("model/Segment3.stl").SetColor(LtMagenta).Translate({96.5f, 60.0f, -1.8f});
    Add(&segment1).Add(&segment2).Add(&segment3);
	}
};

#endif