#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <CtrlLib/CtrlLib.h>
#include "Servo3D.hpp"
#include "RobotState.hpp"

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