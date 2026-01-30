#ifndef _ALGORITHM_HPP_
#define _ALGORITHM_HPP_

#include "RobotState.hpp"

class Algorithm {
private:
	RobotState begin;
	RobotState end;
	int timeMS = 1000;
	int count = 10;

public:
	Array<RobotState> states;
	
	Algorithm& SetStates(RobotState& b, RobotState& e) { begin = b; end = e; return *this; }
	Algorithm& SetTime(int ms) { timeMS = ms; return *this; }
	Algorithm& SetCount(int c) { count = c; return *this; }
	void Calculate() {
		if (count != states.GetCount()) states.SetCount(count, begin);
		int segCount = begin.segments.GetCount();
		for (int i = 0; i < count; ++i) {
			RobotState& s = states[i];
			s.SetName("CalcState " + IntStr(i));
			for (int segI = 0; segI < segCount; ++segI) {
				CalcMid(i, begin.segments[segI], end.segments[segI], s.segments[segI]);
			}
		}
	}

private:
	void CalcMid(int n, const SegmentState& b, const SegmentState& e, SegmentState& s) {
		s.SetAngle(b.GetAngle() + (e.GetAngle() - b.GetAngle()) * n / (count - 1));
		int count = b.segments.GetCount();
		for (int i = 0; i < count; ++i) {
			CalcMid(n, b.segments[i], e.segments[i], s.segments[i]);
		}
	}
};

#endif
