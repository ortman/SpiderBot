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
	
	int GetMaxSegments() const {
		return 3; // TODO!
	}
	
	/*
	bool Set(Vector<int> idx, float val) {
		Array<Segment>* s = &segments;
		int cnt = idx.GetCount();
		for (int i = 0; i < cnt; ++i) {
			if (i < 0 || i >= s->GetCount()) return false;
			if (i + 1 == cnt) {
				(*s)[i].angle = val;
				return true;
			} else {
				s = &(*s)[i].segments;
			}
		}
		return false;
	}
	*/
	
	void ClearSelection() {
		//TODO
	}
	
	Node3D* GetNode(int col, int row) {
		return NULL;
	}
	
	SegmentState* GetSegment(int id) {
		return NULL;
	}
};

#endif