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
	mat4 setvoTransform;
	
	static GLuint sectorVao;
	static GLuint sectorVbo;
	
	void UpdateServoTransform() {
		setvoTransform = glm::scale(mat4(1.0f), servoScale);
		setvoTransform = glm::translate(setvoTransform, servoTranslate);
		setvoTransform = glm::rotate(setvoTransform, glm::radians(servoRotate.x), vec3(1, 0, 0));
		setvoTransform = glm::rotate(setvoTransform, glm::radians(servoRotate.y), vec3(0, 1, 0));
		setvoTransform = glm::rotate(setvoTransform, glm::radians(servoRotate.z + angle), vec3(0, 0, 1));
	}
	
public:
	Servo3D() = default;
	
	virtual ~Servo3D() {
		Shader::VaoDeinit(sectorVao, sectorVbo);
	}
	
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
		UpdateServoTransform();
		return *this;
	}
	float GetAngle() { return angle; }
	float GetMinAngle() { return minAngle; }
	Servo3D& SetMinAngle(float angle) { minAngle = angle; return *this; }
	float GetMaxAngle() { return maxAngle; }
	Servo3D& SetMaxAngle(float angle) { maxAngle = angle; return *this; }

	virtual void GLPaint(const mat4& pv, const vec3& cameraPos, mat4 t, bool isSelectMode) override {
		t = t * setvoTransform;
		Node3D::GLPaint(pv, cameraPos, t, isSelectMode);
		if (!isSelectMode && isSelected) {
			shaderFlat.Use();
			t = glm::rotate(t, glm::radians(90.f - angle), vec3(0, 0, 1));
			shaderFlat.SetModel(t);
			shaderFlat.SetPV(pv);
			shaderFlat.SetColor({0.3f, 1.0f, 0.3f, 0.5f});
			shaderFlat.SetViewPos(cameraPos);
			DrawSector(maxAngle - minAngle);
		}
	}
	
	vec3 GetScale() const override { return servoScale; }
	Node3D& SetScale(const vec3& s) override {
		servoScale = s;
		UpdateServoTransform();
		return *this;
	}
	vec3 GetRotate() const override { return servoRotate; }
	Node3D& SetRotate(const vec3& p) override {
		servoRotate = p;
		UpdateServoTransform();
		return *this;
	}
	const vec3& GetTranslate() const & override { return servoTranslate; }
	Node3D& SetTranslate(const vec3& t) override {
		servoTranslate = t;
		UpdateServoTransform();
		return *this;
	}
	
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
		if (json.IsLoading()) UpdateServoTransform();
	}

	virtual const Bboxf GetBbox() const override {
		Bboxf res = Node3D::GetBbox();
		return res.Rotate(servoRotate).Translate(servoTranslate) * servoScale;
	}

private:
	void CreateSector(float radius = 40.f) {
		Vector<vec3> points; // XYZ, normal XYZ
		double angle = 0.0f;
		vec3 center(0.0f);
		vec3 normal(0.f, 0.f, 1.f);
		while (angle < M_2PI) {
			points.Add(center); points.Add(normal);
			points.Add(vec3(radius * cos(angle), radius * sin(angle), 0.0f)); points.Add(normal);
			angle += M_PI / 180.;
			points.Add(vec3(radius * cos(angle), radius * sin(angle), 0.0f)); points.Add(normal);
		}
		Shader::VaoTrianglesInit(points, sectorVao, sectorVbo);
	}

	void DrawSector(float angle) {
		if (angle < 1.0 || angle > 360.) return;
		if (!sectorVao) CreateSector();
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		Shader::DrawObject(sectorVao, sectorVbo, (int)angle * 3);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	}
};

GLuint Servo3D::sectorVao = 0;
GLuint Servo3D::sectorVbo = 0;

INITBLOCK {
	Node3D::Register<Servo3D>();
}

#endif