#ifndef _VIEW_3D_HPP_
#define _VIEW_3D_HPP_

#include "Node3D.hpp"

class View3D : public GLCtrl {
private:
	bool multiselect = false;
	Point mouseMiddleStart, mouseRightStart;
	float distance = 2.0f;
	float azimuth = (float)(-M_PI_4);
	float elevation = (float)(M_PI / 6.0);
	mat4 pv = mat4(1.f);

	Vector<Node3D*> nodes;
	vec3 cameraPos, cameraCenter, pivotPoint;
	Bboxf bbox;
	GLuint gridVao = 0;
	GLuint gridVbo = 0;

	void UpdateCameraPosition() {
		vec3 cameraVector = {
			distance * cos(elevation) * sin(azimuth),
			distance * cos(elevation) * cos(azimuth),
			distance * sin(elevation)
		};
		cameraPos = pivotPoint + cameraVector;
		cameraCenter = pivotPoint;
		
		//vec3 eulerAngles(10.f, 30., 0.);
		//quat q(radians(eulerAngles));
		//vec3 lp = pivotPoint + q * cameraVector;
		//light_position[0] = lp.x;
		//light_position[1] = lp.y;
		//light_position[2] = lp.z;
		
		Size sz = GetSize();
		if (sz.cx > 0 && sz.cy > 0) {
			float aspect = (float)sz.cx / (float)sz.cy;
			mat4 projection = glm::perspective((float)M_PI_4, aspect, 0.1f, 2000.0f);
			mat4 view = lookAt(
		    cameraPos,
		    cameraCenter,
		    vec3(0.0f, 0.0f, 1.0f)
			);
			pv = projection * view;
		}
	}

public:
	Event<int, Node3D*, bool> WhenSelected; // id, node, multiselect
	Event<Point, int, dword> WhenWeel;
	
	View3D() {
		UpdateCameraPosition();
	}
	
	~View3D() {
		ExecuteGL([&] {
			Shader::VaoDeinit(gridVao, gridVbo);
		});
	}

	View3D& Add(Node3D* node) {
		nodes.Add(node);
		return *this;
	}

	bool Remove(Node3D* node) {
//		if (node == NULL) return false;
//		for (int i = 0; i < nodes.GetCount(); ++i) {
//			if (node == nodes[i]) {
//				nodes.Remove(i);
//				return true;
//			} else {
//				if (nodes[i]->Remove(node)) {
//					return true;
//				}
//			}
//		}
		return false;
	}

	View3D& Clear() {
		nodes.Clear();
	  return *this;
	}

	View3D& ViewAll() {
		RecalcBbox();

		pivotPoint = (bbox.min + bbox.max) / 2.0f;
		distance = glm::length(bbox.GetSize()) * 1.5f;

		UpdateCameraPosition();
		Refresh();
		return *this;
	}
	
	View3D& RecalcBbox() {
		Bboxf bbox{{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
		for (Node3D* node : nodes) {
			bbox += node->GetBbox();
		}
		this->bbox = bbox;
		return *this;
	}

	int GetNodeId(const Point &p) {
		int selectedId = -1;
		ExecuteGL([&] {
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			for (Node3D* node : nodes) {
				node->GLPaint(pv, cameraPos, mat4(1.f), true);
			}
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			unsigned char res[4];
			glReadPixels(p.x, viewport[3] - p.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
			if (res[3] > 0) { // Проверка alpha, если объект там есть
				selectedId = res[0] + (res[1] << 8) + (res[2] << 16);
			}
		});
		return selectedId;
	}

	template <class T>
	T* GetNode(int id) const {
		if (id < 0) return NULL;
		T* res = NULL;
		for (Node3D* node : nodes) {
			if (node->GetId() == id) {
				return dynamic_cast<T*>(node);
			}
			if ((res = node->GetNode<T>(id)) != NULL) {
				return res;
			}
		}
		return NULL;
	}

	void SelectNode(Node3D* node, bool recursive = false, bool multiSelect = false) {
		if (!multiSelect || !multiselect) {
			for (Node3D* n : nodes) {
				n->Selected(false, true);
			}
		}
		if (node != NULL) node->Selected(true, recursive);
		Refresh();
	}

	void SelectNode(int id, bool recursive = false, bool multiSelect = false) {
		Node3D* selectedNode = GetNode<Node3D>(id);
		SelectNode(selectedNode, recursive, multiSelect);
	}
	
	virtual void Layout() override {
		UpdateCameraPosition();
	}
	
	View3D& MultiSelect(bool b = true) { multiselect = b; return *this; }

private:
	virtual void MouseMove(Point p, dword keyflags) override {
		if (keyflags & K_MOUSEMIDDLE) {
			float dx = (float)(p.x - mouseMiddleStart.x) * 0.01f;
			float dy = (float)(p.y - mouseMiddleStart.y) * 0.01f;

			azimuth += dx;
			elevation += dy;

			const float maxElevation = (float)M_PI_2 - 0.01f;
			elevation = UPP::clamp(elevation, -maxElevation, maxElevation);
			
			UpdateCameraPosition();
			mouseMiddleStart = p;
			Refresh();
		} else if (keyflags & K_MOUSERIGHT) {
			float dx = (float)(p.x - mouseRightStart.x) * -0.001f;
			float dy = (float)(p.y - mouseRightStart.y) * 0.001f;

			vec3 dir = glm::normalize(cameraCenter - cameraPos);
			vec3 right = glm::normalize(glm::cross(dir, vec3(0.0f, 0.0f, 1.0f)));
			vec3 up = glm::normalize(glm::cross(right, dir));

			pivotPoint = pivotPoint + right * dx * distance + up * dy * distance;
			UpdateCameraPosition();
			mouseRightStart = p;
			Refresh();
		}
	}

	virtual void MiddleDown(Point p, dword keyflags) override {
		mouseMiddleStart = p;
	}

	virtual void LeftDown(Point p, dword keyflags) override {
		int id = GetNodeId(p);
		Node3D* selectedNode = GetNode<Node3D>(id);
		bool ms = multiselect && keyflags & K_CTRL;
		SelectNode(selectedNode, false, ms);
		WhenSelected(id, selectedNode, ms);
	}

	virtual void RightDown(Point p, dword keyflags) override {
		mouseRightStart = p;
	}

	//virtual void RightUp(Point p, dword keyflags) override {
	//}
	
	virtual void MouseWheel(Point p, int zdelta, dword keyflags) override {
		if (keyflags) {
			WhenWeel(p, zdelta, keyflags);
		} else {
			distance *= (zdelta > 0) ? 0.9f : 1.1f;
			distance = UPP::clamp(distance, 0.1f, 1000.0f);
			UpdateCameraPosition();
			Refresh();
		}
	}

	virtual void GLPaint() override {
		vec3 bgColor((float)SColorFace().GetR() / 255.0f, (float)SColorFace().GetG() / 255.0f, (float)SColorFace().GetB() / 255.0f);
		glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		for (Node3D* node : nodes) {
			node->GLPaint(pv, cameraPos, mat4(1.f), false);
		}
		Node3D::shaderFlat.Use();
		Node3D::shaderFlat.SetPV(pv);
		Node3D::shaderFlat.SetColor(vec4(1.f));
		Node3D::shaderFlat.SetViewPos(cameraPos);
		Node3D::shaderFlat.SetModel(scale(mat4(1.f), bbox.GetSize()));
		if (!gridVao) GridInit();
		Shader::DrawLines(gridVao, gridVbo, 44);
		//for (Node3D* node : nodes) {
		//	node->DrawBbox(Node3D::shaderFlat);
		//}
	}
	
	void GridInit() {
		Vector<vec3> grid(44);
		float s;
		for (int i = 0; i < 11; ++i) {
			s = i * 0.1f - 0.5f;
			grid[i * 2]      = vec3(s, -0.5f, -0.5f);
			grid[i * 2 + 1]  = vec3(s,  0.5f, -0.5f);
			grid[i * 2 + 22] = vec3(-0.5f, s, -0.5f);
			grid[i * 2 + 23] = vec3(0.5f,  s, -0.5f);
		}
		Shader::VaoLinesInit(grid, gridVao, gridVbo);
	}
};

#endif