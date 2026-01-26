#ifndef _STEP_EDIT_HPP_
#define _STEP_EDIT_HPP_

#include "Command.hpp"

class StepEdit : public Ctrl {
private:
	EditFloat e;
	RobotState* state = NULL;
	int maxSegmentsX = 1;
	int step = 7;
	int editHeight = 18;
	int editWidth = 0;

public:
	void SetStep(RobotState* s) {
		state = s;
		if (step) maxSegmentsX = state->GetMaxSegments();
		Refresh();
	}
	
	void Paint(Draw& w) override {
		if (step == NULL) return;
		int x = 0, y = 0;
		Size sz = GetSize();
		int cnt = state->segments.GetCount();
		if (cnt > 1) {
			w.DrawLine(x, y + 9, x, y + cnt * (editHeight + step) + (editHeight / 2 - (editHeight + step)), 1, SColorHighlight());
		}
		editWidth = UPP::max(40, sz.cx / maxSegmentsX - step);
		for (const SegmentState& s : state->segments) {
			w.DrawLine(x, y + 9, x + step, y + 9, 1, SColorHighlight());
			DrawSegment(w, s, x + step, y);
			y += editHeight + step;
		}
	}
	
	void LeftDown(Point p, dword keyflags) override {
		if (state) {
			int x = p.x / (editWidth + step);
			int y = p.y / (editHeight + step);
			Node3D* node = state->GetNode(x, y);
			if (Servo3D* serv = dynamic_cast<Servo3D*>(node)) {
				Add(e.LeftPos(x * (editWidth + step) + step, editWidth).TopPos(y * (editHeight + step), editHeight));
				e.SetData(serv->GetAngle());
				e.SetFocus();
			} else {
				SetFocus();
				RemoveChild(&e);
			}
		}
	}

private:
	void DrawSegment(Draw& w, const SegmentState& s, int x, int& y) {
		int xEnd = x + editWidth;
		int yEnd = y + editHeight;
		
		EditFieldFrame().FramePaint(w, Rect(x, y, xEnd, yEnd));
		
		w.DrawText(x + 3, y + 2, DblStr(s.GetAngle()), StdFont(), SColorText());
		
		int cnt = s.segments.GetCount();
		for (int i = 0; i < cnt; i++) {
			const SegmentState& ss = s.segments[i];
			if (i) {
				w.DrawLine(x + editWidth / 2, y + 9, xEnd + step, y + 9, 1, SColorHighlight());
				w.DrawLine(x + editWidth / 2, y + 9, x + editWidth / 2, y - step, 1, SColorHighlight());
			} else {
				w.DrawLine(xEnd, y + 9, xEnd + step, y + 9, 1, SColorHighlight());
			}
			DrawSegment(w, ss, xEnd + step, y);
			if (i) y += editHeight + step;
		}
	}
};

#endif