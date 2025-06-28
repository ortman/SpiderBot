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
          if (i >= 0) {
            tMotors.SetCursor(i);
          }
        }
      }
    };
    
    bChangeModelPath.WhenPush = [=] {
      if (currentServo != NULL) {
        if (fs.ExecuteOpen(t_("Select 3d model"))) {
					currentServo->SetModelPath(fs);
					eModelPath <<= (String)fs;
					viewer.ViewAll();
        }
      }
    };
    
		ePosX.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetTranslate();
				p.x = (float)~ePosX;
				currentServo->Translate(p);
				viewer.Refresh();
			}
		};
		
		ePosY.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetTranslate();
				p.y = (float)~ePosY;
				currentServo->Translate(p);
				viewer.Refresh();
			}
		};
		
		ePosZ.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetTranslate();
				p.z = (float)~ePosZ;
				currentServo->Translate(p);
				viewer.Refresh();
			}
		};
		
		eRotX.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetRotate();
				p.x = (float)~eRotX;
				currentServo->Rotate(p);
				viewer.Refresh();
			}
		};
		
		eRotY.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetRotate();
				p.y = (float)~eRotY;
				currentServo->Rotate(p);
				viewer.Refresh();
			}
		};
		
		eRotZ.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetRotate();
				p.z = (float)~eRotZ;
				currentServo->Rotate(p);
				viewer.Refresh();
			}
		};
		
		eModelPosX.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetModelTranslate();
				p.x = (float)~eModelPosX;
				currentServo->SetModelTranslate(p);
				viewer.Refresh();
			}
		};
		
		eModelPosY.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetModelTranslate();
				p.y = (float)~eModelPosY;
				currentServo->SetModelTranslate(p);
				viewer.Refresh();
			}
		};
		
		eModelPosZ.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetModelTranslate();
				p.z = (float)~eModelPosZ;
				currentServo->SetModelTranslate(p);
				viewer.Refresh();
			}
		};
		
		eModelRotX.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetModelRotate();
				p.x = (float)~eModelRotX;
				currentServo->SetModelRotate(p);
				viewer.Refresh();
			}
		};
		
		eModelRotY.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetModelRotate();
				p.y = (float)~eModelRotY;
				currentServo->SetModelRotate(p);
				viewer.Refresh();
			}
		};
		
		eModelRotZ.WhenAction = [=] {
			if (currentServo) {
				Point3f p = currentServo->GetModelRotate();
				p.z = (float)~eModelRotZ;
				currentServo->SetModelRotate(p);
				viewer.Refresh();
			}
		};
		
		cColor.WhenAction = [=] {
			if (currentServo) {
				currentServo->SetModelColor(~cColor);
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
		if (isBody || currentServo == NULL) {
			DisableCtrls({
				&lbMotorParams,
				&eLength,
				&ePosX, &ePosY, &ePosZ,
				&eRotX, &eRotY, &eRotZ
			});
			eLength.Clear();
			ePosX.Clear();
			ePosY.Clear();
			ePosZ.Clear();
			eRotX.Clear();
			eRotY.Clear();
			eRotZ.Clear();
		} else {
			EnableCtrls({
				&lbMotorParams,
				&eLength,
				&ePosX, &ePosY, &ePosZ,
				&eRotX, &eRotY, &eRotZ
			});
			eLength <<= currentServo->GetLength();
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
			eModelPath <<= currentServo->GetModelPath();
			cColor     <<= currentServo->GetModelColor();
			eModelPosX <<= currentServo->GetModelTranslate().x;
			eModelPosY <<= currentServo->GetModelTranslate().y;
			eModelPosZ <<= currentServo->GetModelTranslate().z;
			eModelRotX <<= currentServo->GetModelRotate().x;
			eModelRotY <<= currentServo->GetModelRotate().y;
			eModelRotZ <<= currentServo->GetModelRotate().z;
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