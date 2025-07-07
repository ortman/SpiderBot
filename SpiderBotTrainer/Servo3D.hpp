#ifndef _SERVO3D_HPP_
#define _SERVO3D_HPP_

#include "Node3D.hpp"

class Servo3D : public Node3D {
private:
	Node3D stl;
	float minAngle = 0.0f;
	float maxAngle = 180.0f;
	float angle = 0.0f;
	String stlPath;
	Point3f servoRotation;
	
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

	Servo3D& SetAngle(float a) {
		angle = a;
		if (angle > maxAngle) angle = maxAngle;
		if (angle < minAngle) angle = minAngle;
		Point3f rot = servoRotation;
		rot.z += angle;
		Node3D::SetRotate(rot);
		return *this;
	}

	float GetMinAngle() {
		return minAngle;
	}

	Servo3D& SetMinAngle(float angle) {
		minAngle = angle;
		return *this;
	}

	float GetMaxAngle() {
		return maxAngle;
	}

	Servo3D& SetMaxAngle(float angle) {
		maxAngle = angle;
		return *this;
	}

	String GetModelPath() {
		return stlPath;
	}

	Node3D& GetModel() {
		return stl;
	}
	
	virtual Node3D& SetRotate(const Point3f& p) override {
		Point3f rot = servoRotation = p;
		rot.z += angle;
		return Node3D::SetRotate(rot);
	}

	virtual Point3f GetRotate() const override {
		return servoRotation;
	}

	virtual void GLPaint(bool isSelectMode) override {
		Node3D::GLPaint(isSelectMode);
		if (!isSelectMode && isSelected) {
			glPushMatrix();
			// Преобразования модели
			glScalef(scale.x, scale.y, scale.z);
			glTranslatef(translate.x, translate.y, translate.z);
			glRotatef(rotate.x, 1, 0, 0);
			glRotatef(rotate.y, 0, 1, 0);
			glRotatef(rotate.z, 0, 0, 1);

			glDisable(GL_LIGHTING); // Отключаем освещение для осей
			glLineWidth(2.0f);
			glBegin(GL_LINES);
				// Ось X (красный)
				glColor3f(1.0f, 0.0f, 0.0f);
				glVertex3f(0.0f, 0.0f, 0.0f);
				glVertex3f(100.0f, 0.0f, 0.0f); // Длина оси = 100

				// Ось Z (синий)
				glColor3f(0.0f, 0.0f, 1.0f);
				glVertex3f(0.0f, 0.0f, -100.0f);
				glVertex3f(0.0f, 0.0f, 100.0f);
			glEnd();

			glColor3f(0.0f, 1.0f, 0.0f);
			DrawSector(maxAngle, 50.0f);
			glPopMatrix();
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