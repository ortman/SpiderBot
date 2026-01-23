#ifndef _STEP_EDIT_HPP_
#define _STEP_EDIT_HPP_

#include "Command.hpp"

class StepEdit : public Ctrl {
private:
	EditFloat e;
	RobotState* step = NULL;
	int maxSegmentsX = 1;

public:
	void SetStep(RobotState* s) {
		step = s;
		if (step) maxSegmentsX = step->GetMaxSegments();
		Refresh();
	}
	
	void Paint(Draw& w) override {
		if (step == NULL) return;
		int x = 0, y = 0;
		Size sz = GetSize();
		int cnt = step->segments.GetCount();
		if (cnt > 1) {
			w.DrawLine(x, y + 9, x, y + cnt * 25 + (9 - 25), 1, SColorHighlight());
		}
		int width = UPP::max(40, sz.cx / maxSegmentsX - 7);
		for (const RobotState::Segment& s : step->segments) {
			w.DrawLine(x, y + 9, x + 7, y + 9, 1, SColorHighlight());
			DrawSegment(w, s, x + 7, y, width);
			y += 25;
		}
	}

private:
	void DrawSegment(Draw& w, const RobotState::Segment& s, int x, int& y, int width) {
		EditFieldFrame().FramePaint(w, Rect(x, y, x + width, y + 18));
		
		w.DrawText(x + 5, y + 2, DblStr(s.angle), StdFont(), SColorText());
		
		int cnt = s.segments.GetCount();
		for (int i = 0; i < cnt; i++) {
			const RobotState::Segment& ss = s.segments[i];
			if (i) {
				w.DrawLine(x + width / 2, y + 9, x + width + 7, y + 9, 1, SColorHighlight());
				w.DrawLine(x + width / 2, y + 9, x + width / 2, y - 7, 1, SColorHighlight());
			} else {
				w.DrawLine(x + width, y + 9, x + width + 7, y + 9, 1, SColorHighlight());
			}
			DrawSegment(w, ss, x + width + 7, y, width);
			if (i) y += 25;
		}
		
	}
};

#endif