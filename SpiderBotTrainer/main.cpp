#include <CtrlLib/CtrlLib.h>
#include "View3D.hpp"
#include "Servo3D.hpp"
#include "RobotEditor.hpp"

using namespace Upp;

class MainWindow : public WithMainlayout<TopWindow> {
private:
	MenuBar menu;
	RobotEditor robotEditor;
	Servo3D body;

public:
	
	void LoadRobot() {
		//Square3D* sq = new Square3D(10.f, 20.f, 5.f);
		//sq->SetColor(LtBlue);
		//sq->SetRotate({45,0,0});
		//sq->SetTranslate({10,5,2.5});
		//viewer.Add(sq).ViewAll();
		viewer.Clear();
		tFoots.Clear();
		
		for (Node3D* node : body.GetChildren()) body.Remove(node);
		Node3D& bodyStr = body.GetModel();
		body.Add(&bodyStr);
		
		bodyStr.LoadSTL("models/Body.stl").SetTranslate({0.0f, 0.0f, 0.0f}).SetRotate({0.0f, 0.0f, 0.0f}).SetColor(Yellow);

		Servo3D* foot1 = new Servo3D("models/Segment1.stl", {95.f, -2.0f, -61.f}, {-90.0f, 0.0f, 0.0f}, LtBlue);
		foot1->SetAngle(90.0f).SetTranslate({36.5f,   0.0f, 0.0f});
		Servo3D* segment2 = new Servo3D("models/Segment2.stl", {95.f, -2.0f, -61.f}, {-90.0f, 0.0f, 0.0f}, LtCyan);
		segment2->SetAngle(90.0f);
		Servo3D* segment3 = new Servo3D("models/Segment3.stl", {95.f, -2.0f, -61.f}, {-90.0f, 0.0f, 0.0f}, LtMagenta);
		segment3->SetAngle(90.0f);
		foot1->Add(segment2, true);
		segment2->Add(segment3, true);
		
		Servo3D* foot2 = (Servo3D*)foot1->Duplicate();
		foot1->SetTranslate({-36.5f,  0.0f, 0.0f}).SetRotate({0.0f, 0.0f, 180.0f});
		
		Servo3D* foot3 = (Servo3D*)foot1->Duplicate();
		foot3->SetTranslate({-25.f,  67.0f, 0.0f}).SetRotate({0.0f, 0.0f, 150.0f});
		
		Servo3D* foot4 = (Servo3D*)foot1->Duplicate();
		foot4->SetTranslate({25.f,   67.0f, 0.0f}).SetRotate({0.0f, 0.0f, 30.0f});
		
		Servo3D* foot5 = (Servo3D*)foot1->Duplicate();
		foot5->SetTranslate({-25.f, -67.0f, 0.0f}).SetRotate({0.0f, 0.0f, 210.0f});
		
		Servo3D* foot6 = (Servo3D*)foot1->Duplicate();
		foot6->SetTranslate({25.f,  -67.0f, 0.0f}).SetRotate({0.0f, 0.0f, -30.0f});
    
		body.Add(foot1, true);
		body.Add(foot2, true);
		body.Add(foot3, true);
		body.Add(foot4, true);
		body.Add(foot5, true);
		body.Add(foot6, true);
    viewer.Add(&body).ViewAll();
    
    tFoots.SetRoot(CtrlImg::File(), (int64_t)&body, t_("Body"));
    AddNodeToTree(0, foot1);
    AddNodeToTree(0, foot2);
    AddNodeToTree(0, foot3);
    AddNodeToTree(0, foot4);
    AddNodeToTree(0, foot5);
    AddNodeToTree(0, foot6);
	}
	
	MainWindow() {
		//SetDarkThemeEnabled(false);
		CtrlLayout(*this, t_("SpiderBot Trainer"));
    Zoomable().Sizeable();
    
		AddFrame(menu);
		menu.Set([=](Bar& bar) { MainMenu(bar); });
    
    clSteps.Add("Step1");
    
    tFoots.WhenSel = [=] {
      int id = tFoots.GetCursor();
      if (id != 0) body.Selected(false);
      Servo3D* serv = dynamic_cast<Servo3D*>((Node3D*)(int64_t)tFoots[id]);
      if (serv != NULL && id >= 0) viewer.SelectNode(serv);
      viewer.Refresh();
    };

    
    GLCtrl::SetDoubleBuffering();
    GLCtrl::SetMSAA();
    
    viewer.WhenSelected = [=](int id, Node3D* node) {
      if (node != NULL) {
        Servo3D* serv = dynamic_cast<Servo3D*>(node);
        while (serv == NULL && node != NULL) {
          node = node->GetParent();
          serv = dynamic_cast<Servo3D*>(node);
        }
        if (serv != NULL && serv != &body) {
          int i = tFoots.Find((int64_t)serv);
          if (i >= 0) tFoots.SetCursor(i);
        }
      }
    };

    bUnits.WhenPush = [=] {
    };
  }
  
	~MainWindow() {
	}
	
private:
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
	}
	
	void AddNodeToTree(int parentId, Node3D* node) {
		int id = tFoots.Add(parentId, CtrlImg::File(), (int64_t)node, t_("Servo"));
		for (Node3D* n : node->GetChildren()) {
			if (dynamic_cast<Servo3D*>(n)) {
				AddNodeToTree(id, n);
			}
		}
	}
};

GUI_APP_MAIN {
	StdLogSetup(LOG_FILE);
  MainWindow win;
  win.Run();
}