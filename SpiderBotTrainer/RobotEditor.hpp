#ifndef _ROBOT_EDITOR_HPP_
#define _ROBOT_EDITOR_HPP_

using namespace Upp;

#define LAYOUTFILE <SpiderBotTrainer/Layouts.lay>
#include <CtrlCore/lay.h>

class RobotEditor : public WithRobotEditorLayout<TopWindow> {
private:
	Node3D body;
	Node3D* currentNode;
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
				Node3D* parent = (Node3D*)(int64_t)tMotors[parentId];
				if (parent) {
					Servo3D& serv	= parent->Create<Servo3D>();
					int id = tMotors.Add(parentId, CtrlImg::File(), (int64_t)&serv, t_("Servo"));
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
//				Servo3D& serv = parent->Create<Servo3D>(node);
//				int id = tMotors.Add(parentId, CtrlImg::File(), (int64_t)&serv, t_("Servo"));
//				tMotors.SetCursor(id);
			}
		};

		tMotors.WhenSel = [=] {
			int id = tMotors.GetCursor();
			if (id == 0) {
				SetNode(&body);
			} else if (id > 0) {
				SetServo(dynamic_cast<Servo3D*>((Node3D*)(int64_t)tMotors[id]));
			} else {
				SetNoNode();
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
		
		bSave.WhenPush = [=] {
			StoreAsJsonFile(body, "RobotEditor.json", true);
		};
		
		bLoad.WhenPush = [=] {
			LoadFromJsonFile(body, "RobotEditor.json");
			viewer.Refresh();
		};

		bChangeModelPath.WhenPush = [=] {
			if (currentNode != NULL) {
				if (fs.ExecuteOpen(t_("Select a 3D model"))) {
					currentNode->LoadSTL(fs);
					eModelPath <<= (String)fs;
					viewer.ViewAll();
				}
			}
		};

		eMinAngle.WhenAction = [=] {
			if (currentNode) {
				//currentServo->SetMinAngle(~eMinAngle);
				viewer.Refresh();
			}
		};

		eMaxAngle.WhenAction = [=] {
			if (currentNode) {
				//currentServo->SetMaxAngle(~eMaxAngle);
				viewer.Refresh();
			}
		};

		ePosX.WhenAction = [=] {
			if (currentNode) {
				Point3f p = currentNode->GetTranslate();
				p.x = (float)~ePosX;
				currentNode->SetTranslate(p);
				viewer.Refresh();
			}
		};

		ePosY.WhenAction = [=] {
			if (currentNode) {
				Point3f p = currentNode->GetTranslate();
				p.y = (float)~ePosY;
				currentNode->SetTranslate(p);
				viewer.Refresh();
			}
		};

		ePosZ.WhenAction = [=] {
			if (currentNode) {
				Point3f p = currentNode->GetTranslate();
				p.z = (float)~ePosZ;
				currentNode->SetTranslate(p);
				viewer.Refresh();
			}
		};

		eRotX.WhenAction = [=] {
			if (currentNode) {
				Point3f p = currentNode->GetRotate();
				p.x = (float)~eRotX;
				currentNode->SetRotate(p);
				viewer.Refresh();
			}
		};

		eRotY.WhenAction = [=] {
			if (currentNode) {
				Point3f p = currentNode->GetRotate();
				p.y = (float)~eRotY;
				currentNode->SetRotate(p);
				viewer.Refresh();
			}
		};

		eRotZ.WhenAction = [=] {
			if (currentNode) {
				Point3f p = currentNode->GetRotate();
				p.z = (float)~eRotZ;
				currentNode->SetRotate(p);
				viewer.Refresh();
			}
		};

		eModelPosX.WhenAction = [=] {
			if (currentNode) {
				Servo3D* serv = dynamic_cast<Servo3D*>(currentNode);
				if (serv) {
					Point3f p = serv->GetModelTranslate();
					p.x = (float)~eModelPosX;
					serv->SetModelTranslate(p);
					viewer.Refresh();
				}
			}
		};

		eModelPosY.WhenAction = [=] {
			if (currentNode) {
				Servo3D* serv = dynamic_cast<Servo3D*>(currentNode);
				if (serv) {
					Point3f p = serv->GetModelTranslate();
					p.y = (float)~eModelPosY;
					serv->SetModelTranslate(p);
					viewer.Refresh();
				}
			}
		};

		eModelPosZ.WhenAction = [=] {
			if (currentNode) {
				Servo3D* serv = dynamic_cast<Servo3D*>(currentNode);
				if (serv) {
					Point3f p = serv->GetModelTranslate();
					p.z = (float)~eModelPosZ;
					serv->SetModelTranslate(p);
					viewer.Refresh();
				}
			}
		};

		eModelRotX.WhenAction = [=] {
			if (currentNode) {
				Servo3D* serv = dynamic_cast<Servo3D*>(currentNode);
				if (serv) {
					Point3f p = serv->GetModelRotate();
					p.x = (float)~eModelRotX;
					serv->SetModelRotate(p);
					viewer.Refresh();
				}
			}
		};

		eModelRotY.WhenAction = [=] {
			if (currentNode) {
				Servo3D* serv = dynamic_cast<Servo3D*>(currentNode);
				if (serv) {
					Point3f p = serv->GetModelRotate();
					p.y = (float)~eModelRotY;
					serv->SetModelRotate(p);
					viewer.Refresh();
				}
			}
		};

		eModelRotZ.WhenAction = [=] {
			if (currentNode) {
				Servo3D* serv = dynamic_cast<Servo3D*>(currentNode);
				if (serv) {
					Point3f p = serv->GetModelRotate();
					p.z = (float)~eModelRotZ;
					serv->SetModelRotate(p);
					viewer.Refresh();
				}
			}
		};

		cColor.WhenAction = [=] {
			if (currentNode) {
				currentNode->SetColor(~cColor);
				viewer.RecalcBbox();
				viewer.Refresh();
			}
		};

		SetServo(NULL);
	}
	
	~RobotEditor() {
		currentNode = NULL;
		tMotors.Clear();
		viewer.Clear();
	}

private:
	void SetNoNode() {
		currentNode = NULL;
		DisableCtrls({
			&lbMotorParams,
			&eMinAngle, &eMaxAngle,
			&ePosX, &ePosY, &ePosZ,
			&eRotX, &eRotY, &eRotZ,
			&lbModel, &eModelPath, &bChangeModelPath, &cColor,
			&eModelPosX, &eModelPosY, &eModelPosZ,
			&eModelRotX, &eModelRotY, &eModelRotZ
		});
		eMinAngle.Clear();
		eMaxAngle.Clear();
		ePosX.Clear();
		ePosY.Clear();
		ePosZ.Clear();
		eRotX.Clear();
		eRotY.Clear();
		eRotZ.Clear();
		eModelPath.Clear();
		eModelPosX.Clear();
		eModelPosY.Clear();
		eModelPosZ.Clear();
		eModelRotX.Clear();
		eModelRotY.Clear();
		eModelRotZ.Clear();
	}
	
	void SetNode(Node3D* node) {
		if (node == NULL) return;
		currentNode = node;
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
		
		EnableCtrls({
			&lbModel, &eModelPath, &bChangeModelPath, &cColor,
			&eModelPosX, &eModelPosY, &eModelPosZ,
			&eModelRotX, &eModelRotY, &eModelRotZ
		});
		eModelPath <<= node->GetSTLPath();
		cColor     <<= node->GetColor();
		eModelPosX <<= node->GetTranslate().x;
		eModelPosY <<= node->GetTranslate().y;
		eModelPosZ <<= node->GetTranslate().z;
		eModelRotX <<= node->GetRotate().x;
		eModelRotY <<= node->GetRotate().y;
		eModelRotZ <<= node->GetRotate().z;
	}
	
	void SetServo(Servo3D* serv) {
		if (serv == NULL) return;
		currentNode = serv;
		viewer.SelectNode(serv);
		EnableCtrls({
			&lbMotorParams,
			&eMinAngle, &eMaxAngle,
			&ePosX, &ePosY, &ePosZ,
			&eRotX, &eRotY, &eRotZ,
			&lbModel, &eModelPath, &bChangeModelPath, &cColor,
			&eModelPosX, &eModelPosY, &eModelPosZ,
			&eModelRotX, &eModelRotY, &eModelRotZ
		});
		eMaxAngle <<= serv->GetMaxAngle();
		eMinAngle <<= serv->GetMinAngle();
		ePosX <<= serv->GetTranslate().x;
		ePosY <<= serv->GetTranslate().y;
		ePosZ <<= serv->GetTranslate().z;
		eRotX <<= serv->GetRotate().x;
		eRotY <<= serv->GetRotate().y;
		eRotZ <<= serv->GetRotate().z;

		eModelPath <<= serv->GetSTLPath();
		cColor     <<= serv->GetColor();
		eModelPosX <<= serv->GetModelTranslate().x;
		eModelPosY <<= serv->GetModelTranslate().y;
		eModelPosZ <<= serv->GetModelTranslate().z;
		eModelRotX <<= serv->GetModelRotate().x;
		eModelRotY <<= serv->GetModelRotate().y;
		eModelRotZ <<= serv->GetModelRotate().z;
	}

//	void AddNodeToTree(int parentId, Node3D* node) {
//		int id = tMotors.Add(parentId, CtrlImg::File(), (int64_t)node, t_("Servo"));
//		for (const Node3D& n : node->GetChildren()) {
//			if (dynamic_cast<Servo3D*>(&n)) {
//				AddNodeToTree(id, n);
//			}
//		}
//	}
};

#endif