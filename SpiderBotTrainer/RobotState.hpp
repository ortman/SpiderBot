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
		for (SegmentState& s : segments) s.ClearSelection();
	}
	
	SegmentState* GetSegment(Node3D& parent, int id) {
		SegmentState* res = NULL;
		Array<Node3D>& children = parent.GetChildren();
		int cnt = children.GetCount();
		if (segments.GetCount() != cnt) return NULL;
		for (int i = 0; i < cnt; ++i) {
			if (children[i].GetId() == id) return &segments[i];
			res = segments[i].GetSegment(children[i], id);
			if (res) return res;
		}
		return NULL;
	}
	
	bool ApplyTo(Node3D& node) {
		Array<Node3D>& nodes = node.GetChildren();
		int cnt = segments.GetCount();
		if (cnt != nodes.GetCount()) return false;
		for (int i = 0; i < cnt; ++i) {
			if (!segments[i].ApplyTo(nodes[i])) return false;
		}
		return true;
	}
};

#endif