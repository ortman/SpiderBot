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

public:
	MainWindow() {
		//SetDarkThemeEnabled(false);
		CtrlLayout(*this, t_("SpiderBot Trainer"));
		Zoomable().Sizeable();

		AddFrame(menu);
		menu.Set([=](Bar& bar) { MainMenu(bar); });
		
		clSteps.WhenAction = [&]() {
			int idx = clSteps.GetCursor();
			if (idx >= 0) {
				Value v = clSteps.Get(idx);
				if (v.Is<CommandStep>()) {
					const CommandStep& step = v.To<CommandStep>();
					// ...
				}
			}
		};

		tFoots.WhenSel = [=] {
			int idx = tFoots.GetCursor();
			if (idx >= 0) viewer.SelectNode(tFoots[idx]);
		};

		//GLCtrl::SetDoubleBuffering();
		GLCtrl::SetMSAA(); // Anti-aliasing on

		viewer.WhenSelected = [=](int id, Node3D* node) {
			int i = tFoots.Find(id);
			if (i >= 0) {
				tFoots.SetCursor(i);
			} else {
				tFoots.KillCursor();
			}
		};
		viewer.WhenWeel = [=](Point p, int zdelta, dword keyflags) {
			int i = tFoots.GetCursor();
			if (keyflags & (K_CTRL | K_SHIFT) && i >= 0) {
				Servo3D* serv = viewer.GetNode<Servo3D>(tFoots[i]);
				if (serv) {
					float delta = zdelta > 0 ? 1.f : -1.f;
					if (keyflags & K_SHIFT) delta *= 10.f;
					serv->SetAngle(serv->GetAngle() + delta);
					viewer.Refresh();
				}
			}
		};

		bUnits << [=] {};
		bAdd << [=] {};
		bRemove << [=] {};
		bUp << [=] {};
		bDown << [=] {};
		
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
		tFoots.Clear();
		body.RemoveAll();

		LoadFromJsonFile(body, "robot.json");
		viewer.Add(&body).ViewAll();
		AddNodeToTree(body);
		
		CommandStep defaultStep;
		clSteps.Add(RawToValue(defaultStep), (Value)defaultStep.ToString());
	}
	
	void MainMenu(Bar& bar) {
		bar.Sub(t_("File"), [=](Bar& bar) {
			bar.Add(t_("Exit"), [=] {	Exit(); });
		});
		bar.Add(t_("Robot editor"), [=] { if (!robotEditor.IsOpen()) robotEditor.Open(); });
		bar.Add(t_("Load robot"), [=] { LoadRobot(); });
	}

	void AddNodeToTree(const Node3D& node, int parentId = -1) {
		int id = 0;
		if (parentId < 0) {
			tFoots.SetRoot(CtrlImg::File(), node.GetId(), t_("Body") + IntStr(node.GetId()));
		} else {
			id = tFoots.Add(parentId, CtrlImg::File(), node.GetId(), t_("Servo") + IntStr(node.GetId()));
		}
		for (const Node3D& n : node.GetChildren()) {
			AddNodeToTree(n, id);
		}
	}
};

GUI_APP_MAIN {
	StdLogSetup(LOG_FILE);
	MainWindow().Run();
}