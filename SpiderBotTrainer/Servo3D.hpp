#ifndef _SERVO3D_HPP_
#define _SERVO3D_HPP_

#include "Node3D.hpp"

class Servo3D : public Node3D {
private:
	Node3D stl;
	float length = 0.0f;
	String stlPath;
	
public:
	Servo3D() {
		Add(&stl);
	}
	
	Servo3D(const String& stlPath, const Point3f& translationVector, const Point3f& rotationVector, const Color& color) : Node3D() {
		this->stlPath = stlPath;
		Add(&stl.LoadSTL(stlPath).Translate(translationVector).Rotate(rotationVector).SetColor(color));
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
	
	Servo3D& SetLenght(float len) {
		//length = len;
		return *this;
	}
	
	float GetLength() {
		return length;
	}
	
	String GetModelPath() {
		return stlPath;
	}
	
	Servo3D& SetModelPath(const String& path) {
		stlPath = path;
		stl.LoadSTL(stlPath);
	  return *this;
	}
	
	Servo3D& SetModelRotate(const Point3f& p) {
		stl.Rotate(p);
	  return *this;
	}
	
	const Point3f& GetModelRotate() const & {
		return stl.GetRotate();
	}
	
	Servo3D& SetModelTranslate(const Point3f& t) {
		stl.Translate(t);
	  return *this;
	}
	
	const Point3f& GetModelTranslate() const & {
		return stl.GetTranslate();
	}
	
	Servo3D& SetModelColor(const Color& c) {
		stl.SetColor(c);
	  return *this;
	}
	
	const Color& GetModelColor() const & {
		return stl.GetColor();
	}
	
	virtual void GLPaint(bool isSelectMode) override {
		Node3D::GLPaint(isSelectMode);
		if (!isSelectMode && isSelected) {
			//glDisable(GL_LIGHTING); // Отключаем освещение для осей
			glLineWidth(2.0f);
			glBegin(GL_LINES);
        // Ось X (красный)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(100.0f, 0.0f, 0.0f); // Длина оси = 100

        // Ось Y (зеленый)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 100.0f, 0.0f);

        // Ось Z (синий)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 100.0f);
			glEnd();
		}
	}
};

#endif