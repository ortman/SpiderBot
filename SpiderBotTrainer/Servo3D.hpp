#ifndef _SERVO_3D_HPP_
#define _SERVO_3D_HPP_

#include "Node3D.hpp"

class Servo3D : public Node3D {
private:
	float minAngle = 0.0f;
	float maxAngle = 180.0f;
	float angle = 0.0f;
	
	Point3f servoScale = {1.0f, 1.0f, 1.0f};
	Point3f servoRotate;
	Point3f servoTranslate;
	
public:
	Servo3D() = default;
	
	virtual Servo3D* Copy() const override {
		Servo3D* node = new Servo3D();
		node->bbox = bbox;
		node->scale = scale;
		node->rotate = rotate;
		node->translate = translate;
		node->isSelected = isSelected;
		node->color = color;
		node->stlPath = stlPath;
		node->points = clone(points);
		for (const Node3D& n : nodes) {
			node->nodes.Add(n.Copy());
		}
		node->minAngle = minAngle;
		node->maxAngle = maxAngle;
		node->angle = angle;
		node->servoScale = servoScale;
		node->servoRotate = servoRotate;
		node->servoTranslate = servoTranslate;
		return node;
	}

	Servo3D(const String& stlPath, const Point3f& translationVector, const Point3f& rotationVector, const Color& color) : Node3D() {
		LoadSTL(stlPath).SetTranslate(translationVector).SetRotate(rotationVector).SetColor(color);
	}

	Servo3D& SetAngle(float a) {
		angle = clamp(a, minAngle, maxAngle);
		return *this;
	}

	float GetMinAngle() { return minAngle; }
	Servo3D& SetMinAngle(float angle) { minAngle = angle; return *this; }
	float GetMaxAngle() { return maxAngle; }
	Servo3D& SetMaxAngle(float angle) { maxAngle = angle; return *this; }

	virtual void GLPaint(bool isSelectMode) override {
		glPushMatrix();
		// Преобразования модели
		glScalef(servoScale.x, servoScale.y, servoScale.z);
		glTranslatef(servoTranslate.x, servoTranslate.y, servoTranslate.z);
		glRotatef(servoRotate.x, 1, 0, 0);
		glRotatef(servoRotate.y, 0, 1, 0);
		glRotatef(servoRotate.z, 0, 0, 1);
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
				glVertex3f(0.0f, 0.0f, -100.0f);
				glVertex3f(0.0f, 0.0f, 100.0f);
			glEnd();

			glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
			DrawSector(maxAngle, 50.0f);
		}
		glPopMatrix();
	}
	
	Node3D& SetScale(const Point3f& s) override { servoScale = s; return *this; }
	Node3D& SetScale(const float s) override { servoScale = {s, s, s}; return *this; }
	Node3D& SetRotate(const Point3f& p) override { servoRotate = p; return *this; }
	Point3f GetRotate() const override { return servoRotate; }
	Node3D& SetTranslate(const Point3f& t) override { servoTranslate = t; return *this; }
	const Point3f& GetTranslate() const & override { return servoTranslate; }
	
	Servo3D& SetModelScale(const Point3f& s) { scale = s; return *this; }
	Servo3D& SetModelScale(const float s) { scale = {s, s, s}; return *this; }
	virtual Servo3D& SetModelRotate(const Point3f& p) { rotate = p; return *this; }
	virtual Point3f GetModelRotate() const { return rotate; }
	Servo3D& SetModelTranslate(const Point3f& t) { translate = t; return *this; }
	const Point3f& GetModelTranslate() const & { return translate; }
	
	virtual void Jsonize(JsonIO& json) override {
		json("Angle", angle);
		json("servoScale", servoScale)("servoRotate", servoRotate)("servoTranslate", servoTranslate);
		Node3D::Jsonize(json);
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

INITBLOCK {
	Node3D::Register<Servo3D>();
}

#endif