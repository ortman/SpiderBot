#ifndef _COMMAND_HPP_
#define _COMMAND_HPP_

#include <CtrlLib/CtrlLib.h>

class CommandStep : public Moveable<CommandStep> {
private:
	String name = "Step";
public:
	struct Segment : public Moveable<Segment> {
		float angle;
		Vector<Segment> segments;

		void Jsonize(JsonIO& json) {
			json("angle", angle)("segments", segments);
		}
		Segment() : angle(0) {}
    Segment(const Segment& s) { angle = s.angle; segments <<= s.segments; }
		void operator=(const Segment& s) { angle = s.angle; segments <<= s.segments; }
	};
	
	Vector<Segment> segments;
	
	CommandStep() {};
	CommandStep(String name) { this->name = name; }
	CommandStep(const CommandStep& s) {
		name = s.name;
		segments <<= s.segments;
	}
    
	String ToString() const { return name; }
	void Jsonize(JsonIO& json) {
		json("segments", segments);
	}
};

struct Command {
	uint8_t code;
	Vector<CommandStep> steps;
	
	void Jsonize(JsonIO& json) {
		json("code", code)("steps", steps);
	}
};

#endif