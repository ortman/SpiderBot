#ifndef _SEGMENT_STATE_HPP_
#define _SEGMENT_STATE_HPP_

class SegmentState {
private:
	float angle;
	float minAngle;
	float maxAngle;

public:
	bool isselect = false;
	Array<SegmentState> segments;

	void Jsonize(JsonIO& json) {
		json("angle", angle)("segments", segments);
	}
	SegmentState(const Node3D *node) {
		const Servo3D* serv = dynamic_cast<const Servo3D*>(node);
		if (serv) {
			angle = serv->GetAngle();
			minAngle = serv->GetMinAngle();
			maxAngle = serv->GetMaxAngle();
		} else {
			angle = minAngle = maxAngle = 0.f;
		}
		for (const Node3D& n : node->GetChildren()) {
			segments.Add(SegmentState(&n));
		}
	}
  SegmentState(){}
  SegmentState(const SegmentState& s) {
    angle = s.angle;
    minAngle = s.minAngle;
    maxAngle = s.maxAngle;
		segments <<= s.segments;
	}
	
	bool ApplyTo(Node3D& node) {
		Servo3D* serv = dynamic_cast<Servo3D*>(&node);
		if (!serv) return false;
		serv->SetAngle(angle);
		serv->Selected(isselect);
		Array<Node3D>& nodes = node.GetChildren();
		int cnt = segments.GetCount();
		if (cnt != nodes.GetCount()) return false;
		for (int i = 0; i < cnt; ++i) {
			if (!segments[i].ApplyTo(nodes[i])) return false;
		}
		return true;
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
	
	void ClearSelection() {
		isselect = false;
		for (SegmentState& s : segments) s.ClearSelection();
	}
	
	bool IsSel() { return isselect; }
	void Select(bool s) { isselect = s; }
	float GetAngle() const { return angle; }
	void SetAngle(float a) { angle = UPP::clamp(a, minAngle, maxAngle); }
	float GetMinAngle() const { return minAngle; }
	float GetMaxAngle() const { return maxAngle; }
};

#endif