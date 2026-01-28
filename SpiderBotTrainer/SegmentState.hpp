#ifndef _SEGMENT_STATE_HPP_
#define _SEGMENT_STATE_HPP_

class SegmentState {
private:
	float angle;

public:
	bool isselect;
	Array<SegmentState> segments;

	void Jsonize(JsonIO& json) {
		json("angle", angle)("segments", segments);
	}
  SegmentState(const Node3D *node) {
    const Servo3D* serv = dynamic_cast<const Servo3D*>(node);
    angle = serv ? serv->GetAngle() : 0.f;
		for (const Node3D& n : node->GetChildren()) {
			segments.Add(SegmentState(&n));
		}
  }
  SegmentState(){}
  SegmentState(const SegmentState& s) {
    angle = s.angle;
		segments <<= s.segments;
	}
	Vector<float> GetAngles() const {
		Vector<float> angles;
		angles.Add(angle);
		for (const SegmentState& s : segments) {
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
	
	bool ApplyTo(Node3D& node) {
		Servo3D* serv = dynamic_cast<Servo3D*>(&node);
		if (!serv) return false;
		serv->SetAngle(angle);
		Array<Node3D>& nodes = node.GetChildren();
		int cnt = segments.GetCount();
		if (cnt != nodes.GetCount()) return false;
		for (int i = 0; i < cnt; ++i) {
			if (!segments[i].ApplyTo(nodes[i])) return false;
		}
		return true;
	}
	
	void ClearSelection() {
		isselect = false;
		for (SegmentState& s : segments) s.ClearSelection();
	}
	
	bool IsSel() { return isselect; }
	void Select(bool s) { isselect = s; }
	float GetAngle() const { return angle; }
	void SetAngle(float a) { angle = a; }
};

#endif