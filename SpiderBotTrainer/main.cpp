#include <CtrlLib/CtrlLib.h>
#include "View3D.hpp"
#include "Servo3D.hpp"
#include "RobotEditor.hpp"

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

		clSteps.Add("Step1");

		tFoots.WhenSel = [=] {
			int idx = tFoots.GetCursor();
			if (idx >= 0) viewer.SelectNode(tFoots[idx], true);
		};

		GLCtrl::SetDoubleBuffering();
		GLCtrl::SetMSAA(); // Anti-aliasing on

		viewer.WhenSelected = [=](int id, Node3D* node) {
			int i = tFoots.Find(id);
			if (i >= 0) {
				tFoots.SetCursor(i);
			} else {
				tFoots.KillCursor();
			}
		};

		bUnits.WhenPush = [=] {
		};
	}

	~MainWindow() {
	}

private:
	void LoadRobot() {
		viewer.Clear();
		tFoots.Clear();

		body.RemoveAll();

		body.LoadSTL("models/Body.stl").SetColor(Yellow);

		Servo3D& segment1 = body.Create<Servo3D>();
		segment1.LoadSTL("models/Segment1.stl")
		        .SetTranslate({96.4f, -1.8f, -81.f})
		        .SetRotate({-90.0f, 0.0f, 0.0f})
		        .SetColor(LtBlue);
		//segment1.SetAngle(90.0f).SetTranslate({36.f, 0.0f, 20.5f});
		
		Servo3D& segment2 = segment1.Create<Servo3D>();
		segment2.LoadSTL("models/Segment2.stl")
		        .SetTranslate({38.f, -93.0f, 1.5f})
		        .SetRotate({180.0f, 0.0f, 13.5f})
		        .SetColor(LtCyan);
		//segment2.SetAngle(90.0f).SetTranslate({37.6f, 0.f, 0.4f}).SetRotate({90.f, 0.f, 21.5f});
		
		Servo3D& segment3 = segment2.Create<Servo3D>();
		segment3.LoadSTL("models/Segment3.stl")
        .SetTranslate({93.f, 7.5f, 2.f})
        .SetRotate({180.0f, 0.0f, -85.5f})
        .SetColor(LtMagenta);
		//segment3.SetAngle(90.0f).SetTranslate({45.f, 0.f, 0.f}).SetRotate({0.f, 0.f, -100.5f});

		Servo3D& foot2 = body.Create<Servo3D>(segment1);
		foot2.SetNextId(true).SetTranslate({-25.f,  67.0f, 20.5f}).SetRotate({0.0f, 0.0f, 150.0f});
		
		Servo3D& foot3 = body.Create<Servo3D>(segment1);
		foot3.SetNextId(true).SetTranslate({-25.f,  67.0f, 20.5f}).SetRotate({0.0f, 0.0f, 150.0f});
		
		Servo3D& foot4 = body.Create<Servo3D>(segment1);
		foot4.SetNextId(true).SetTranslate({25.f,   67.0f, 20.5f}).SetRotate({0.0f, 0.0f, 30.0f});
		
		Servo3D& foot5 = body.Create<Servo3D>(segment1);
		foot5.SetNextId(true).SetTranslate({-25.f, -67.0f, 20.5f}).SetRotate({0.0f, 0.0f, 210.0f});
		
		Servo3D& foot6 = body.Create<Servo3D>(segment1);
		foot6.SetNextId(true).SetTranslate({25.f,  -67.0f, 20.5f}).SetRotate({0.0f, 0.0f, -30.0f});

		viewer.Add(&body).ViewAll();
		AddNodeToTree(body);
	}
	
	void SaveRobot() {
		StoreAsJsonFile(body, "robot.json", true);
	}
	
	void MainMenu(Bar& bar) {
		bar.Sub(t_("File"), [=](Bar& bar) {
			bar.Add(t_("Exit"), [=] {
				Exit();
			});
		});
		bar.Add(t_("Robot editor"), [=] {
			if (!robotEditor.IsOpen()) robotEditor.Open();
		});
		bar.Add(t_("Load robot"), [=] {
			LoadRobot();
		});
		bar.Add(t_("Save robot"), [=] {
			SaveRobot();
		});
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