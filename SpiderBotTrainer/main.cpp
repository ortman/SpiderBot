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
	
	Square3D sq = Square3D(300, 300, 2);
	//StatusBar status;

public:
	
	void LoadRobot() {
		viewer.Clear();
		clFoots.Clear();
		
		body.SetModelPath("models/Body.stl").SetModelTranslate({0.0f, 0.0f, 0.0f}).SetModelRotate({0.0f, 0.0f, 0.0f}).SetModelColor(LtGreen);

		Servo3D* foot1 = new Servo3D("models/Segment1.stl", {95.f, -2.0f, -61.f}, {-90.0f, 0.0f, 0.0f}, LtBlue);
		foot1->SetAngle(90.0f).Translate({36.5f,   0.0f, 0.0f});
		Servo3D* segment2 = new Servo3D("models/Segment2.stl", {95.f, -2.0f, -61.f}, {-90.0f, 0.0f, 0.0f}, LtCyan);
		segment2->SetAngle(90.0f);
		Servo3D* segment3 = new Servo3D("models/Segment3.stl", {95.f, -2.0f, -61.f}, {-90.0f, 0.0f, 0.0f}, LtMagenta);
		segment3->SetAngle(90.0f);
		foot1->Add(segment2, true);
		segment2->Add(segment3, true);
		body.Add(foot1, true);
		
		Servo3D* foot2 = (Servo3D*)foot1->Duplicate();
		foot2->Translate({-36.5f,  0.0f, 0.0f}).Rotate({0.0f, 0.0f, 180.0f});
		body.Add(foot2, true);
		
		Servo3D* foot3 = (Servo3D*)foot1->Duplicate();
		foot3->Translate({-25.f,  67.0f, 0.0f}).Rotate({0.0f, 0.0f, 150.0f});
		body.Add(foot3, true);
		
		Servo3D* foot4 = (Servo3D*)foot1->Duplicate();
		foot4->Translate({25.f,   67.0f, 0.0f}).Rotate({0.0f, 0.0f, 30.0f});
		body.Add(foot4, true);
		
		Servo3D* foot5 = (Servo3D*)foot1->Duplicate();
		foot5->Translate({-25.f, -67.0f, 0.0f}).Rotate({0.0f, 0.0f, 210.0f});
		body.Add(foot5, true);
		
		Servo3D* foot6 = (Servo3D*)foot1->Duplicate();
		foot6->Translate({25.f,  -67.0f, 0.0f}).Rotate({0.0f, 0.0f, -30.0f});
		body.Add(foot6, true);

    viewer.Add(&body).ViewAll();
    
    clFoots.Add((int64)&foot1, Value("Foot1"));
    clFoots.Add((int64)&foot2, Value("Foot2"));
    clFoots.Add((int64)&foot3, Value("Foot3"));
    clFoots.Add((int64)&foot4, Value("Foot4"));
    clFoots.Add((int64)&foot5, Value("Foot5"));
    clFoots.Add((int64)&foot6, Value("Foot6"));
	}
	
	MainWindow() {
		//SetDarkThemeEnabled(false);
		CtrlLayout(*this, t_("SpiderBot Trainer"));
    Zoomable().Sizeable();
    
		AddFrame(menu);
		menu.Set([=](Bar& bar) { MainMenu(bar); });
    
    clSteps.Add("Step1");
    
//    clFoots.WhenAction = [=] {
//      body.Selected(false);
//      int i = clFoots.GetCursor();
//      if (i >= 0) {
//				Servo3D* foot = (Servo3D*)(int64)clFoots[i];
//				if (foot) foot->Selected(true);
//				viewer.Refresh();
//      }
//    };

    //AddFrame(status);
    //status = t_("Ready");

    viewer.Add(&sq.SetColor(LtGray).Translate({0, 0, -75})).ViewAll();
    
    GLCtrl::SetDoubleBuffering();
    GLCtrl::SetMSAA();
    
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
};

GUI_APP_MAIN {
	StdLogSetup(LOG_FILE);
  MainWindow win;
  win.Run();
}