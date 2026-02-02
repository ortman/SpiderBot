#include <CtrlLib/CtrlLib.h>
#include "View3D/View3D.hpp"
#include "Servo3D.hpp"
#include "RobotEditor.hpp"
#include "Command.hpp"
#include "Algorithm.hpp"

#include <CtrlLib/CtrlLib.h>

#define IMAGECLASS SpiderBotImg
#define IMAGEFILE <SpiderBotTrainer/SpiderBot.iml>
#include <Draw/iml.h>

using namespace Upp;

class MainWindow : public WithMainlayout<TopWindow> {
private:
	MenuBar menu;
	RobotEditor robotEditor;
	Node3D body;
	Command cmd;
	Algorithm algLinear;
	Array<RobotState> playStates;

public:
	MainWindow() {
		//SetDarkThemeEnabled(false);
		CtrlLayout(*this, t_("SpiderBot Trainer"));
		Zoomable().Sizeable();

		AddFrame(menu);
		menu.Set([=](Bar& bar) { MainMenu(bar); });

		clStates.NoRoundSize();
		clStates.WhenAction = [&]() {
			int idx = clStates.GetCursor();
			if (cmd.SetCursor(idx)) {
				stateEdit.SetState(&cmd[idx]);
				cmd[idx].ApplyTo(body);
				viewer.Refresh();
			}
			bUp.Enable(idx > 0);
			bDown.Enable(cmd.GetStepCount() - idx > 1);
		};

		stateEdit.WhenAction = [=] {
			int i = cmd.GetCursor();
			if (i >= 0) {
				cmd[i].ApplyTo(body);
				viewer.Refresh();
			}
		};

		//GLCtrl::SetDoubleBuffering();
		GLCtrl::SetMSAA(); // Anti-aliasing on

		viewer.MultiSelect();
		viewer.WhenSelected = [=](int id, Node3D* node, bool multiselect) {
			int i = cmd.GetCursor();
			if (i >= 0) {
				RobotState& state = cmd[i];
				if (!multiselect) state.ClearSelection();
				SegmentState* seg = state.GetSegment(body, id);
				if (seg) seg->Select(!seg->IsSel());
			}
		};
		viewer.WhenWeel = [=](Point p, int zdelta, dword keyflags) {
			if (keyflags & (K_CTRL | K_SHIFT)) {
				int i = cmd.GetCursor();
				if (i >= 0) {
					RobotState& state = cmd[i];
					float d = (zdelta > 0) ? 1.f : -1.f;
					if (keyflags & K_SHIFT) d *= 10.f;
					for (SegmentState& seg : state.segments) {
						ChangeAngleOfSelectedSegment(seg, d);
					}
					stateEdit.SetState(&state);
					state.ApplyTo(body);
					viewer.Refresh();
				}
			}
		};

		bUnits << [=] {};
		bAdd.SetImage(CtrlImg::Add());

		bAdd << [=] {
			int idx = cmd.GetCursor();
			if (idx < 0) {
				RobotState state(&body);
				cmd.AddStep(state);
				clStates.Add(state.GetName());
			} else {
				RobotState state(cmd[idx]);
				cmd.InsertStep(idx, state);
				clStates.Insert(idx + 1, state.GetName());
			}
			clStates.SetCursor(idx + 1);
		};
 
		bRemove.SetImage(CtrlImg::Remove());
		bRemove << [=] {
			int idx = cmd.GetCursor();
			if (idx < 0) return;
			stateEdit.SetState(NULL);
			cmd.RemoveStep(idx);
			clStates.Remove(idx);
		};
		bUp.SetImage(CtrlImg::up_arrow());

		bUp << [=] {
			int stPos = clStates.GetCursor();
			if (stPos < 1) return;
			int prevPos = stPos - 1;
			cmd.Replace(stPos, prevPos);
			clStates.Set(stPos, cmd[stPos].GetName());
			clStates.Set(prevPos, cmd[prevPos].GetName());
			clStates.SetCursor(prevPos);
		};

		bDown.SetImage(CtrlImg::down_arrow());

		bDown << [=] {
			int stPos = clStates.GetCursor();
			int stCount = clStates.GetCount();
			int nextPos = stPos + 1;
			if (nextPos >= stCount) return;
			cmd.Replace(stPos, nextPos);
			clStates.Set(stPos, cmd[stPos].GetName());
			clStates.Set(nextPos, cmd[nextPos].GetName());
			clStates.SetCursor(nextPos);
		};

		bPlay.SetImage(SpiderBotImg::Play());
		bPlay.WhenPush = [=] {
			if (ExistsTimeCallback(0)) {
				KillTimeCallback(0);
				bPlay.SetImage(SpiderBotImg::Play());
			} else {
				playStates.Clear();
				int stateCount = cmd.GetStepCount();
				if (stateCount < 2) return;
				algLinear.SetCount(10).SetTime(5000);
				for (int i = 1; i < stateCount; ++i) {
					algLinear.SetStates(cmd[i - 1], cmd[i]);
					algLinear.Calculate();
					playStates.Append(algLinear.states);
				}
				
				SetTimeCallback(-1000 / 25, [=] {
					static int pos = 0;
					if (pos >= playStates.GetCount()) pos = 0;
					playStates[pos].ApplyTo(body);
					viewer.Refresh();
					++pos;
				}, 0);
				bPlay.SetImage(SpiderBotImg::Pause());
			}
		};
		LoadRobot();
	}

	~MainWindow() {
	}

private:
	float testDelta = 1.f;
	float testDelta2 = 1.f;
	
	void LoadRobot() {
		viewer.Clear();
		clStates.Clear();
		body.RemoveAll();

		if (!LoadFromJsonFile(body, "robot.json")) return;
		viewer.Add(&body).ViewAll();
		
		RobotState defaultState(&body);
		cmd.AddStep(defaultState);
		
		clStates.Add(defaultState.GetName());
		clStates.SetCursor(0);
	}
	
	void MainMenu(Bar& bar) {
		bar.Sub(t_("File"), [=](Bar& bar) {
			bar.Add(t_("Exit"), [=] {	Exit(); });
		});
		bar.Add(t_("Robot editor"), [=] { if (!robotEditor.IsOpen()) robotEditor.Open(); });
		bar.Add(t_("Load robot"), [=] { LoadRobot(); });
	}
	
	void ChangeAngleOfSelectedSegment(SegmentState& seg, float d) {
		if (seg.IsSel()) seg.SetAngle(seg.GetAngle() + d);
		for (SegmentState& s : seg.segments) {
			ChangeAngleOfSelectedSegment(s, d);
		}
	}
};

GUI_APP_MAIN {
	StdLogSetup(LOG_FILE);
	MainWindow().Run();
}