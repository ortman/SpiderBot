#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <CtrlLib/CtrlLib.h>
#include "Servo3D.hpp"

class RobotState {
private:
	String name = "Step";
public:
	struct Segment {
		float angle;
		Array<Segment> segments;

		void Jsonize(JsonIO& json) {
			json("angle", angle)("segments", segments);
		}
    Segment(const Node3D *node) {
      const Servo3D* serv = dynamic_cast<const Servo3D*>(node);
      angle = serv ? serv->GetAngle() : 0.f;
			for (const Node3D& n : node->GetChildren()) {
				segments.Add(Segment(&n));
			}
    }
    Segment(){}
    Segment(const Segment& s) {
      angle = s.angle;
			segments <<= s.segments;
		}
		Vector<float> GetAngles() const {
			Vector<float> angles;
			angles.Add(angle);
			for (const Segment& s : segments) {
				angles.Append(s.GetAngles());
			}
			return angles;
		}
		bool Set(int i, float val) {
			if (i == 0) {
				angle = val;
				return true;
			}
			if (i < 0 || segments.GetCount() != 1) return false;
			return segments[0].Set(i - 1, val);
		}
	};
	
	Array<Segment> segments;
	
	RobotState() {}
	RobotState(const Node3D *node) {
		if (node) {
			for (const Node3D& n : node->GetChildren()) {
				segments.Add(Segment(&n));
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
	int GetMaxSegments() const {
		return 3; // TODO!
	}
	//Vector<Segment>& GetSegments() { return segments; }
	RobotState& SetName(const String& n) { name = n; return *this; }
	String GetName() const { return name; }
	
	void Set(int y, int x, float val) {
		if (x < 0 || y < 0 || y >= segments.GetCount()) return;
		segments[y].Set(x, val);
	}
};

struct Command {
private:
	int cursor = -1;
	uint8_t code;
	Array<RobotState> steps;
	
public:
	void Jsonize(JsonIO& json) {
		json("code", code)("steps", steps);
	}
	
	bool SetCursor(int i) {
		if (i >= 0 && i < steps.GetCount()) {
			cursor = i;
			return true;
		}
		return false;
	}
	
	int GetCursor() { return cursor; }
	
	void AddStep(const RobotState& step) { steps.Add(step); }
	void InsertStep(int i, const RobotState& step) { steps.Insert(i, step); }
	void RemoveStep(int i) {
		steps.Remove(i);
		if (i == cursor) cursor = -1;
	}
	int GetStepCount() { return steps.GetCount(); }
	void Replace(int i1, int i2) {
		if (i1 >= 0 && i2 >= 0 && i1 < steps.GetCount() && i2 < steps.GetCount() ) {
			RobotState s = steps[i1];
			steps[i1] = steps[i2];
			steps[i2] = s;
		}
	}
	RobotState& operator[](int i) { return steps[i]; }
	
};

#endif