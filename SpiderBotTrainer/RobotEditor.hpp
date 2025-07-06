#ifndef _ROBOT_EDITOR_HPP_
#define _ROBOT_EDITOR_HPP_

using namespace Upp;

#define LAYOUTFILE <SpiderBotTrainer/Layouts.lay>
#include <CtrlCore/lay.h>

class RobotEditor : public WithRobotEditorLayout<TopWindow> {
private:
  Servo3D body;
	Servo3D* currentServo;
	FileSel fs;
	
public:
	RobotEditor() {
		CtrlLayout(*this, t_("Robot Editor"));
    Zoomable().Sizeable();
    
    fs.ActiveDir(GetExeFolder());
    
    viewer.Add(&body);
    tMotors.SetRoot(CtrlImg::File(), (int64_t)&body, t_("Body"));
    
    bAddMotor.WhenPush = [=] {
      int parentId = tMotors.GetCursor();
      if (parentId >= 0) {
        Servo3D* parentServ = dynamic_cast<Servo3D*>((Node3D*)(int64_t)tMotors[parentId]);
        if (parentServ) {
	        int id = tMotors.Add(parentId, CtrlImg::File(), (int64_t)parentServ->Create<Servo3D>(), t_("Servo"));
	        tMotors.SetCursor(id);
        }
      }
    };
    
    bRemoveMotor.WhenPush = [=] {
      int id = tMotors.GetCursor();
      if (id > 0) {
        Node3D* node = (Node3D*)(int64_t)tMotors[id];
        tMotors.Remove(id);
        viewer.Remove(node);
        viewer.Refresh();
      }
    };
    
    bCopyMotor.WhenPush = [=] {
      int id = tMotors.GetCursor();
      if (id > 0) {
        int parentId = tMotors.GetParent(id);
        Node3D* node = (Node3D*)(int64_t)tMotors[id];
        Node3D* parent = (Node3D*)(int64_t)tMotors[parentId];
        Node3D* duplicate = node->Duplicate();
        parent->Add(duplicate, true);
        AddNodeToTree(parentId, duplicate);
      }
    };
    
    tMotors.WhenSel = [=] {
      int id = tMotors.GetCursor();
      if (id == 0) {
        SetServo(&body, true);
      } else if (id > 0) {
        SetServo(dynamic_cast<Servo3D*>((Node3D*)(int64_t)tMotors[id]));
      } else {
        SetServo(NULL);
      }
    };
    
    viewer.WhenSelected = [=](int id, Node3D* node) {
      if (node != NULL) {
        Servo3D* serv = dynamic_cast<Servo3D*>(node);
        while (serv == NULL && node != NULL) {
          node = node->GetParent();
          serv = dynamic_cast<Servo3D*>(node);
        }
        if (serv != NULL) {
          int i = tMotors.Find((int64_t)serv);
          if (i >= 0) tMotors.SetCursor(i);
        }
      }
    };
    
    bChangeModelPath.WhenPush = [=] {
      if (currentServo != NULL) {
        if (fs.ExecuteOpen(t_("Select 3d model"))) {
					currentServo->GetModel().LoadSTL(fs);
					eModelPath <<= (String)fs;
					viewer.ViewAll();
        }
      }
    };
    
		eMinAngle.WhenAction = [=] {
			if (currentServo) {
				currentServo->SetMinAngle(~eMinAngle);
				viewer.Refresh();
			}
		};
		
		eMaxAngle.WhenAction = [=] {
			if (currentServo) {
				currentServo->SetMaxAngle(~eMaxAngle);
				viewer.Refresh();
			}
		};
    
		ePosX.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetTranslate();
				p.x = (float)~ePosX;
				currentServo->SetTranslate(p);
				viewer.Refresh();
			}
		};
		
		ePosY.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetTranslate();
				p.y = (float)~ePosY;
				currentServo->SetTranslate(p);
				viewer.Refresh();
			}
		};
		
		ePosZ.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetTranslate();
				p.z = (float)~ePosZ;
				currentServo->SetTranslate(p);
				viewer.Refresh();
			}
		};
		
		eRotX.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetRotate();
				p.x = (float)~eRotX;
				currentServo->SetRotate(p);
				viewer.Refresh();
			}
		};
		
		eRotY.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetRotate();
				p.y = (float)~eRotY;
				currentServo->SetRotate(p);
				viewer.Refresh();
			}
		};
		
		eRotZ.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetRotate();
				p.z = (float)~eRotZ;
				currentServo->SetRotate(p);
				viewer.Refresh();
			}
		};
		
		eModelPosX.WhenAction = [=] {
			if (currentServo) {
				Node3D& model = currentServo->GetModel();
				Point3f p = model.GetTranslate();
				p.x = (float)~eModelPosX;
				model.SetTranslate(p);
				viewer.Refresh();
			}
		};
		
		eModelPosY.WhenAction = [=] {
			if (currentServo) {
				Node3D& model = currentServo->GetModel();
				Point3f p = model.GetTranslate();
				p.y = (float)~eModelPosY;
				model.SetTranslate(p);
				viewer.Refresh();
			}
		};
		
		eModelPosZ.WhenAction = [=] {
			if (currentServo) {
				Node3D& model = currentServo->GetModel();
				Point3f p = model.GetTranslate();
				p.z = (float)~eModelPosZ;
				model.SetTranslate(p);
				viewer.Refresh();
			}
		};
		
		eModelRotX.WhenAction = [=] {
			if (currentServo) {
				Node3D& model = currentServo->GetModel();
				Point3f p = model.GetRotate();
				p.x = (float)~eModelRotX;
				model.SetRotate(p);
				viewer.Refresh();
			}
		};
		
		eModelRotY.WhenAction = [=] {
			if (currentServo) {
				Node3D& model = currentServo->GetModel();
				Point3f p = model.GetRotate();
				p.y = (float)~eModelRotY;
				model.SetRotate(p);
				viewer.Refresh();
			}
		};
		
		eModelRotZ.WhenAction = [=] {
			if (currentServo) {
				Node3D& model = currentServo->GetModel();
				Point3f p = model.GetRotate();
				p.z = (float)~eModelRotZ;
				model.SetRotate(p);
				viewer.Refresh();
			}
		};
		
		cColor.WhenAction = [=] {
			if (currentServo) {
				currentServo->GetModel().SetColor(~cColor);
				viewer.RecalcBbox();
				viewer.Refresh();
			}
		};
    
    SetServo(NULL);
	}
	
	~RobotEditor() {
		currentServo = NULL;
		tMotors.Clear();
		viewer.Clear();
	}
	
private:
	void SetServo(Servo3D* serv, bool isBody = false) {
		currentServo = serv;
		viewer.SelectNode(serv);
		if (isBody || currentServo == NULL) {
			DisableCtrls({
				&lbMotorParams,
				&eMinAngle, &eMaxAngle,
				&ePosX, &ePosY, &ePosZ,
				&eRotX, &eRotY, &eRotZ
			});
			eMinAngle.Clear();
			eMaxAngle.Clear();
			ePosX.Clear();
			ePosY.Clear();
			ePosZ.Clear();
			eRotX.Clear();
			eRotY.Clear();
			eRotZ.Clear();
		} else {
			EnableCtrls({
				&lbMotorParams,
				&eMinAngle, &eMaxAngle,
				&ePosX, &ePosY, &ePosZ,
				&eRotX, &eRotY, &eRotZ
			});
			eMaxAngle <<= currentServo->GetMaxAngle();
			eMinAngle <<= currentServo->GetMinAngle();
			ePosX <<= currentServo->GetTranslate().x;
			ePosY <<= currentServo->GetTranslate().y;
			ePosZ <<= currentServo->GetTranslate().z;
			eRotX <<= currentServo->GetRotate().x;
			eRotY <<= currentServo->GetRotate().y;
			eRotZ <<= currentServo->GetRotate().z;
		}
		if (currentServo == NULL) {
			DisableCtrls({
				&lbModel, &eModelPath, &bChangeModelPath, &cColor,
				&eModelPosX, &eModelPosY, &eModelPosZ,
				&eModelRotX, &eModelRotY, &eModelRotZ
			});
			eModelPath.Clear();
			eModelPosX.Clear();
			eModelPosY.Clear();
			eModelPosZ.Clear();
			eModelRotX.Clear();
			eModelRotY.Clear();
			eModelRotZ.Clear();
		} else {
			EnableCtrls({
				&lbModel, &eModelPath, &bChangeModelPath, &cColor,
				&eModelPosX, &eModelPosY, &eModelPosZ,
				&eModelRotX, &eModelRotY, &eModelRotZ
			});
			Node3D& model = currentServo->GetModel();
			eModelPath <<= currentServo->GetModelPath();
			cColor     <<= model.GetColor();
			eModelPosX <<= model.GetTranslate().x;
			eModelPosY <<= model.GetTranslate().y;
			eModelPosZ <<= model.GetTranslate().z;
			eModelRotX <<= model.GetRotate().x;
			eModelRotY <<= model.GetRotate().y;
			eModelRotZ <<= model.GetRotate().z;
		}
	}
	
	void AddNodeToTree(int parentId, Node3D* node) {
		int id = tMotors.Add(parentId, CtrlImg::File(), (int64_t)node, t_("Servo"));
		for (Node3D* n : node->GetChildren()) {
			if (dynamic_cast<Servo3D*>(n)) {
				AddNodeToTree(id, n);
			}
		}
	}
};

#endif