#ifndef _VIEW_3D_HPP_
#define _VIEW_3D_HPP_

#include "Node3D.hpp"

class View3D : public GLCtrl {
private:
	Point mouseLeftStart, mouseRightStart, mouseLeftClickPos;
	float distance = 2.0f;
	float azimuth = (float)(-M_PI_4);
	float elevation = (float)(M_PI / 6.0);
	mat4 pv = mat4(1.f);

	Vector<Node3D*> nodes;
	vec3 cameraPos, cameraCenter, pivotPoint;
	Bboxf bbox;
	vec3 grid[40];

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
	Event<int, Node3D*> WhenSelected; // id, node
	Event<Point, int, dword> WhenWeel;
	
	View3D() {
		UpdateCameraPosition();
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
		calcGrid();
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

	void SelectNode(Node3D* node, bool recursive = false) {
		for (Node3D* n : nodes) {
			n->Selected(false, true);
		}
		if (node != NULL) node->Selected(true, recursive);
		Refresh();
	}

	void SelectNode(int id, bool recursive = false) {
		Node3D* selectedNode = GetNode<Node3D>(id);
		SelectNode(selectedNode, recursive);
		WhenSelected(id, selectedNode);
	}

private:
	virtual void MouseMove(Point p, dword keyflags) {
		if (keyflags & K_MOUSELEFT) {
			float dx = (float)(p.x - mouseLeftStart.x) * 0.01f;
			float dy = (float)(p.y - mouseLeftStart.y) * 0.01f;

			azimuth += dx;
			elevation += dy;

			const float maxElevation = (float)M_PI_2 - 0.01f;
			elevation = UPP::clamp(elevation, -maxElevation, maxElevation);
			
			UpdateCameraPosition();
			mouseLeftStart = p;
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

	virtual void LeftDown(Point p, dword keyflags) {
		mouseLeftClickPos = mouseLeftStart = p;
	}

	virtual void LeftUp(Point p, dword keyflags) {
		if (p == mouseLeftClickPos) {
			SelectNode(GetNodeId(p));
		}
	}

	virtual void RightDown(Point p, dword keyflags) {
		mouseRightStart = p;
	}

	//virtual void RightUp(Point p, dword keyflags) {
	//}
	
	virtual void MouseWheel(Point p, int zdelta, dword keyflags) {
		if (keyflags) {
			WhenWeel(p, zdelta, keyflags);
		} else {
			distance *= (zdelta > 0) ? 0.9f : 1.1f;
			distance = UPP::clamp(distance, 0.1f, 1000.0f);
			UpdateCameraPosition();
			Refresh();
		}
	}

	virtual void GLPaint() {
		vec3 bgColor((float)SColorFace().GetR() / 255.0f, (float)SColorFace().GetG() / 255.0f, (float)SColorFace().GetB() / 255.0f);
		glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Включаем буфер глубины и освещение
		glEnable(GL_DEPTH_TEST);

		for (Node3D* node : nodes) {
			node->GLPaint(pv, cameraPos, mat4(1.f), false);
			//glLineWidth(1.0f);
			//glColor3f(1.0f, 1.0f, 1.0f); // White
			//node->DrawBbox();
		}
		Node3D::shaderFlat.Use();
		Node3D::shaderFlat.SetModel(mat4(1.f));
		Node3D::shaderFlat.SetPV(pv);
		Node3D::shaderFlat.SetColor(vec4(1.f));
		Node3D::shaderFlat.SetViewPos(cameraPos);
		glLineWidth(1.0f);
		vec3* p;
		glBegin(GL_LINES);
		for (int i = 0; i < 20; ++i) {
			p = &grid[i * 2];
			glVertex3f(p->x, p->y, p->z);
			p = &grid[i * 2 + 1];
			glVertex3f(p->x, p->y, p->z);
		}
		glEnd();
	}

	void calcGrid() {
		vec3 size = bbox.GetSize();
		float stepX = size.x / 9.0f;
		float stepY = size.y / 9.0f;
		float x, y, z = bbox.min.z;
		for (int i = 0; i < 10; ++i) {
			x = bbox.min.x + stepX * i;
			grid[i * 2]      = {x, bbox.min.y, z};
			grid[i * 2 + 1]  = {x, bbox.max.y,  z};
			y = bbox.min.y + stepY * i;
			grid[i * 2 + 20] = {bbox.min.x, y, z};
			grid[i * 2 + 21] = {bbox.max.x, y, z};
		}
	}
};

#endif