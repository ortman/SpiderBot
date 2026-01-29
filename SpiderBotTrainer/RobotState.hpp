#ifndef _ROBOT_STATE_HPP_
#define _ROBOT_STATE_HPP_

#include "SegmentState.hpp"

class RobotState {
private:
	String name = "Step";
	
public:
	Array<SegmentState> segments;
	
	RobotState() {}
	RobotState(const Node3D *node) {
		if (node) {
			for (const Node3D& n : node->GetChildren()) {
				segments.Add(SegmentState(&n));
			}
		}
	}
	RobotState(const RobotState& s) { *this = s; }
	RobotState& operator=(const RobotState& s) {
		name = s.name;
		segments <<= s.segments;
		return *this;
	}
	void Jsonize(JsonIO& json) {
		json("segments", segments);
	}
	RobotState& SetName(const String& n) { name = n; return *this; }
	String GetName() const { return name; }
	
	void ClearSelection() {
		//TODO
	}
	
	SegmentState* GetSegment(int id) {
		//TODO
		return NULL;
	}
	
	void ApplyTo(Node3D& node) {
	}
};

#endif