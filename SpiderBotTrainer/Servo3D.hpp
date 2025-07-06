#ifndef _SERVO3D_HPP_
#define _SERVO3D_HPP_

#include "Node3D.hpp"

class Servo3D : public Node3D {
private:
	Node3D stl;
	float minAngle = 0.0f;
	float maxAngle = 180.0f;
	String stlPath;
	
public:
	Servo3D() {
		Add(&stl);
	}
	
	Servo3D(const String& stlPath, const Point3f& translationVector, const Point3f& rotationVector, const Color& color) : Node3D() {
		this->stlPath = stlPath;
		Add(&stl.LoadSTL(stlPath).SetTranslate(translationVector).SetRotate(rotationVector).SetColor(color));
	}
	
	virtual Node3D* Duplicate(Node3D* src = NULL, bool uniqIDs = true) override {
		if (src == NULL) src = new Servo3D();
		Node3D* dublicate = Node3D::Duplicate(src, uniqIDs);
		Servo3D* serv = dynamic_cast<Servo3D*>(src);
		if (serv) {
			if (serv->nodes.GetCount() > 1) serv->nodes.Remove(1); // remove Old STL.
			serv->stlPath = stlPath;
			stl.Duplicate(&serv->stl);
		}
		return dublicate;
	}
	
	Servo3D& SetAngle(float angle) {
		return *this;
	}
	
	float GetMinAngle() {
		return minAngle;
	}
	
	void SetMinAngle(float angle) {
		minAngle = angle;
	}
	
	float GetMaxAngle() {
		return maxAngle;
	}
	
	void SetMaxAngle(float angle) {
		maxAngle = angle;
	}
	
	String GetModelPath() {
		return stlPath;
	}
	
	Node3D& GetModel() {
		return stl;
	}
	
	virtual void GLPaint(bool isSelectMode) override {
		Node3D::GLPaint(isSelectMode);
		if (!isSelectMode && isSelected) {
			glDisable(GL_LIGHTING); // Отключаем освещение для осей
			glLineWidth(2.0f);
			glBegin(GL_LINES);
        // Ось X (красный)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(100.0f, 0.0f, 0.0f); // Длина оси = 100

        // Ось Z (синий)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 100.0f);
			glEnd();
      
      //glColor3f(0.0f, 1.0f, 0.0f);
			//DrawSector(maxAngle, 100.0f);
		}
	}

private:
	void DrawSector(float angle, float radius) {
		glBegin(GL_TRIANGLE_FAN);
			glVertex3f(0.0f, 0.0f, 0.0f);
			int segmants = 101;
			float a, angleStep = (float)(angle * M_PI / 180.0 / segmants);
			for (int i = 0; i < segmants; ++i) {
				a = angleStep * i;
				glVertex3f(radius * cos(a), radius * sin(a), 0.0f);
			}
		glEnd();
	}
};

#endif