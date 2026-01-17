#ifndef _SERVO_3D_HPP_
#define _SERVO_3D_HPP_

#include "View3D/Node3D.hpp"

class Servo3D : public Node3D {
private:
	float minAngle = 0.0f;
	float maxAngle = 180.0f;
	float angle = 90.0f;
	
	vec3 servoScale = {1.0f, 1.0f, 1.0f};
	vec3 servoRotate;
	vec3 servoTranslate;
	
public:
	Servo3D() = default;
	
	virtual Servo3D* Copy(Node3D* node = NULL) const override {
		Servo3D* serv = node ? dynamic_cast<Servo3D*>(node) : new Servo3D();
		if (serv) {
			Node3D::Copy(serv);
			serv->minAngle = minAngle;
			serv->maxAngle = maxAngle;
			serv->angle = angle;
			serv->servoScale = servoScale;
			serv->servoRotate = servoRotate;
			serv->servoTranslate = servoTranslate;
			return serv;
		} else {
			return NULL;
		}
	}

	Servo3D& SetAngle(float a) {
		angle = UPP::clamp(a, minAngle, maxAngle);
		return *this;
	}
	float GetAngle() { return angle; }
	float GetMinAngle() { return minAngle; }
	Servo3D& SetMinAngle(float angle) { minAngle = angle; return *this; }
	float GetMaxAngle() { return maxAngle; }
	Servo3D& SetMaxAngle(float angle) { maxAngle = angle; return *this; }

	virtual void GLPaint(const mat4& pv, const vec3& cameraPos, mat4 t, bool isSelectMode) override {
		glPushMatrix();
		mat4 transform = glm::scale(mat4(1.0f), servoScale);
		transform = glm::translate(transform, servoTranslate);
		transform = glm::rotate(transform, glm::radians(servoRotate.x), vec3(1, 0, 0));
		transform = glm::rotate(transform, glm::radians(servoRotate.y), vec3(0, 1, 0));
		transform = glm::rotate(transform, glm::radians(servoRotate.z + angle), vec3(0, 0, 1));
		t = t * transform;
		
		Node3D::GLPaint(pv, cameraPos, t, isSelectMode);
		if (!isSelectMode && isSelected) {
			glDisable(GL_LIGHTING);
			glColor4f(0.f, 0.5f, 0.f, 0.5f);
			glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				DrawSector(90.f - angle, maxAngle - minAngle, 40.0f);
			glDisable(GL_BLEND);
			glEnable(GL_LIGHTING);
		}
		glPopMatrix();
	}
	
	Node3D& SetScale(const vec3& s) override { servoScale = s; return *this; }
	Node3D& SetScale(const float s) override { servoScale = {s, s, s}; return *this; }
	Node3D& SetRotate(const vec3& p) override { servoRotate = p; return *this; }
	vec3 GetRotate() const override { return servoRotate; }
	Node3D& SetTranslate(const vec3& t) override { servoTranslate = t; return *this; }
	const vec3& GetTranslate() const & override { return servoTranslate; }
	
	Servo3D& SetModelScale(const vec3& s) { Node3D::SetScale(s); return *this; }
	Servo3D& SetModelScale(const float s) { Node3D::SetScale(s); return *this; }
	virtual Servo3D& SetModelRotate(const vec3& p) { Node3D::SetRotate(p); return *this; }
	virtual vec3 GetModelRotate() const { return Node3D::GetRotate(); }
	Servo3D& SetModelTranslate(const vec3& t) { Node3D::SetTranslate(t); return *this; }
	const vec3& GetModelTranslate() const & { return Node3D::GetTranslate(); }
	
	virtual void Jsonize(JsonIO& json) override {
		json("Angle", angle)("minAngle", minAngle)("maxAngle", maxAngle);
		json("servoScale", servoScale)("servoRotate", servoRotate)("servoTranslate", servoTranslate);
		Node3D::Jsonize(json);
	}

	virtual const Bboxf GetBbox() const override {
		Bboxf res = Node3D::GetBbox();
		return res.Rotate(servoRotate).Translate(servoTranslate) * servoScale;
	}

private:
	void DrawSector(float startAngle, float angle, float radius) {
		glBegin(GL_TRIANGLE_FAN);
			glVertex3f(0.0f, 0.0f, 0.0f);
			int segments = 50;
			startAngle *= (float)(M_PI / 180.0);
			float a, angleStep = (float)(angle * M_PI / 180.0 / segments);
			for (int i = 0; i < segments; ++i) {
				a = startAngle + angleStep * i;
				glVertex3f(radius * cos(a), radius * sin(a), 0.0f);
			}
		glEnd();
	}
};

INITBLOCK {
	Node3D::Register<Servo3D>();
}

#endif