#ifndef _STATE_EDIT_HPP_
#define _STATE_EDIT_HPP_

#include "Command.hpp"

class StateEdit : public Ctrl {
private:
	EditFloat edit;
	RobotState* state = NULL;
	int maxSegmentsX = 0;
	int step = 7;
	int editHeight = 18;
	int editWidth = 0;
	Vector<Vector<SegmentState*>> segPointers;
	SegmentState* editSegment = NULL;

public:
	StateEdit() {
		edit.WhenAction = [=] {
			if (editSegment) {
				editSegment->SetAngle(~edit);
				WhenAction();
			}
		};
	}
	
	void SetState(RobotState* s) {
		SetFocus();
		RemoveChild(&edit);
		editSegment = NULL;
		state = s;
		segPointers.Clear();
		maxSegmentsX = 0;
		if (state) {
			for (SegmentState& s : state->segments) {
				InitSegment(segPointers.Create(), s, 0);
			}
			for (Vector<SegmentState*> &segs : segPointers) {
				int cnt = segs.GetCount();
				for (int i = cnt; i < maxSegmentsX; ++i) segs.Add(NULL);
			}
		}
		Refresh();
	}
	
	void Paint(Draw& w) override {
		Size sz = GetSize();
		w.DrawRect(sz, SColorFace());
		if (state == NULL || maxSegmentsX == 0) return;
		int x = 0, y = 0;
		int cnt = state->segments.GetCount();
		editWidth = UPP::max(40, sz.cx / maxSegmentsX - step);
		for (const SegmentState& s : state->segments) {
			w.DrawLine(x, y + 9, x + step, y + 9, 1, SColorHighlight());
			DrawSegment(w, s, x + step, y);
			y += editHeight + step;
		}
		if (cnt > 1) {
			w.DrawLine(x, 9, x, y + editHeight / 2 - (editHeight + step), 1, SColorHighlight());
		}
	}
	
	void LeftDown(Point p, dword keyflags) override {
		if (state == NULL) return;
		int x = p.x / (editWidth + step);
		int y = p.y / (editHeight + step);
		if (y < segPointers.GetCount() && x < segPointers[y].GetCount() && segPointers[y][x]) {
			Add(edit.LeftPos(x * (editWidth + step) + step, editWidth).TopPos(y * (editHeight + step), editHeight));
			editSegment = segPointers[y][x];
			edit.SetData(editSegment->GetAngle());
			edit.SetFocus();
		} else {
			SetFocus();
			RemoveChild(&edit);
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
			if (i) y += editHeight + step;
			if (i) {
				w.DrawLine(x + editWidth + step / 2, y + 9, xEnd + step, y + 9, 1, SColorHighlight());
				w.DrawLine(x + editWidth + step / 2, y + 9, x + editWidth + step / 2, y - step - editHeight / 2, 1, SColorHighlight());
			} else {
				w.DrawLine(xEnd, y + 9, xEnd + step, y + 9, 1, SColorHighlight());
			}
			DrawSegment(w, ss, xEnd + step, y);
		}
	}
	
	void InitSegment(Vector<SegmentState*> &segs, SegmentState& s, int maxX) {
		segs.Add(&s);
		maxSegmentsX = UPP::max(maxSegmentsX, ++maxX);
		int cnt = s.segments.GetCount();
		for (int i = 0; i < cnt; i++) {
			InitSegment(i ? segPointers.Create(maxX, (SegmentState*)NULL) : segs, s.segments[i], maxX);
		}
	}
};

#endif