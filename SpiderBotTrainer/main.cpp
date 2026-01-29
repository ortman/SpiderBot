#include <CtrlLib/CtrlLib.h>
#include "View3D/View3D.hpp"
#include "Servo3D.hpp"
#include "RobotEditor.hpp"
#include "Command.hpp"

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
			if (i) {
				RobotState& state = cmd[i];
				if (!multiselect) state.ClearSelection();
				SegmentState* seg = state.GetSegment(id);
				if (seg) seg->Select(!seg->IsSel());
			}
		};
		viewer.WhenWeel = [=](Point p, int zdelta, dword keyflags) {
			if (keyflags & (K_CTRL | K_SHIFT)) {
				int i = cmd.GetCursor();
				if (i) {
					RobotState& state = cmd[i];
					float d = (zdelta > 0) ? 1.f : -1.f;
					//TODO
					for (SegmentState& seg : state.segments) {
						if (seg.IsSel()) seg.SetAngle(seg.GetAngle() + d);
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
				SetTimeCallback(-1000 / 25, [=] {
					const Array<Node3D>& foots = body.GetChildren();
					if (foots.GetCount() != 6) return;
					Servo3D* foot3 = (Servo3D*)&foots[3];
					Servo3D* segment2 = (Servo3D*)&foot3->GetChildren()[0];
					if (segment2 == NULL || foot3 == NULL) return;
					
					float angle1 = foot3->GetAngle() + testDelta2;
					if (angle1 > foot3->GetMaxAngle()) testDelta2 = -1.f;
					if (angle1 < foot3->GetMinAngle()) testDelta2 = 1.f;
					foot3->SetAngle(angle1);
					
					float angle2 = segment2->GetAngle() + testDelta;
					if (angle2 > segment2->GetMaxAngle()) testDelta = -1.f;
					if (angle2 < segment2->GetMinAngle()) testDelta = 1.f;
					segment2->SetAngle(angle2);
					
					viewer.Refresh();
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
};

GUI_APP_MAIN {
	StdLogSetup(LOG_FILE);
	MainWindow().Run();
}