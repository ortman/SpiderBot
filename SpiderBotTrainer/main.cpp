#include <CtrlLib/CtrlLib.h>
#include "View3D.hpp"

using namespace Upp;

#define LAYOUTFILE <SpiderBotTrainer/Layouts.lay>
#include <CtrlCore/lay.h>
 
class MainWindow : public WithMainlayout<TopWindow> {
private:
	Node3D body;
	Foot3D foot1, foot2, foot3, foot4, foot5, foot6;
	
	//StatusBar status;
	//Square3D sq = Square3D(100, 100, 100);

public:
	MainWindow() {
		//SetDarkThemeEnabled(false);
		CtrlLayout(*this, t_("SpiderBot Trainer"));
    Zoomable().Sizeable();
    
    clSteps.Add("Step1");
    
    clFoots.Add((int64)&foot1, Value("Foot1"));
    clFoots.Add((int64)&foot2, Value("Foot2"));
    clFoots.Add((int64)&foot3, Value("Foot3"));
    clFoots.Add((int64)&foot4, Value("Foot4"));
    clFoots.Add((int64)&foot5, Value("Foot5"));
    clFoots.Add((int64)&foot6, Value("Foot6"));
    clFoots.WhenAction = [=] {
      body.Selected(false);
      int i = clFoots.GetCursor();
      if (i >= 0) {
				Foot3D* foot = (Foot3D*)(int64)clFoots[i];
				if (foot) foot->Selected(true);
				viewer.Refresh();
      }
    };
    
    //AddFrame(status);
    //status = t_("Ready");

    foot1.Translate({36.5f,   0.0f, 0.0f}).Rotate({-90.0f,   0.0f, 0.0f});
    foot2.Translate({-36.5f,  0.0f, 0.0f}).Rotate({-90.0f, 180.0f, 0.0f});
    foot3.Translate({-25.f,  67.0f, 0.0f}).Rotate({-90.0f, 200.0f, 0.0f});
    foot4.Translate({25.f,   67.0f, 0.0f}).Rotate({-90.0f, -20.0f, 0.0f});
    foot5.Translate({-25.f, -67.0f, 0.0f}).Rotate({-90.0f, 160.0f, 0.0f});
    foot6.Translate({25.f,  -67.0f, 0.0f}).Rotate({-90.0f,  20.0f, 0.0f});
    body.LoadSTL("model/Body.stl").SetColor(LtGreen);
    body.Add(&foot1).Add(&foot2).Add(&foot3).Add(&foot4).Add(&foot5).Add(&foot6);

    viewer.Add(&body).ViewAll();
    //viewer.Add(&sq.SetColor(LtMagenta)).ViewAll();
    
    GLCtrl::SetDoubleBuffering();
    GLCtrl::SetMSAA();
  }
};

GUI_APP_MAIN {
  MainWindow win;
  win.Run();
}