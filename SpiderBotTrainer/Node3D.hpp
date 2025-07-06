#ifndef _NODE_3D_HPP_
#define _NODE_3D_HPP_

#include <GLCtrl/GLCtrl.h>

using namespace Upp;

struct Point3f {
	float x = 0.f, y = 0.f, z = 0.f;
	Point3f() = default;
	Point3f(float x, float y, float z) : x(x), y(y), z(z) {}
	
	Point3f operator+(const Point3f& p) const { return {x + p.x, y + p.y, z + p.z}; }
	Point3f& operator+=(const Point3f& p) {x += p.x; y += p.y; z += p.z; return *this;}
	Point3f operator-(const Point3f& p) const { return {x - p.x, y - p.y, z - p.z}; }
	Point3f operator*(const Point3f& p) const { return {x * p.x, y * p.y, z * p.z}; }
	Point3f& operator*=(const Point3f& p) {x *= p.x; y *= p.y; z *= p.z; return *this;}
	Point3f operator*(float f) const         { return {x * f, y * f, z * f}; }
	Point3f operator/(float f) const         { return {x / f, y / f, z / f}; }
	bool operator==(const Point3f& p) { return x == p.x && y == p.y && z == p.z; }

	float Length() const {
		return sqrtf(x*x + y*y + z*z);
	}

	Point3f Normalize() const {
		float len = Length();
		return len > 0 ? *this / len : *this;
	}

	void min(const Point3f& p) {
		if (p.x < x) x = p.x;
		if (p.y < y) y = p.y;
		if (p.z < z) z = p.z;
	}

	void max(const Point3f& p) {
		if (p.x > x) x = p.x;
		if (p.y > y) y = p.y;
		if (p.z > z) z = p.z;
	}

	Point3f Cross(const Point3f& p) {
		return Point3f{
			y * p.z - z * p.y,
			z * p.x - x * p.z,
			x * p.y - y * p.x
		};
	}
	
	static void CalcRotateMatrix(const Point3f& angle_deg) {
		if (calcRxyzAngleDeg == angle_deg) return;
		calcRxyzAngleDeg = angle_deg;
		float ax = (float)(angle_deg.x * M_PI / 180.0);
		float ay = (float)(angle_deg.y * M_PI / 180.0);
		float az = (float)(angle_deg.z * M_PI / 180.0);

		// Вычисление синусов и косинусов
		float cx = std::cos(ax), sx = std::sin(ax);
		float cy = std::cos(ay), sy = std::sin(ay);
		float cz = std::cos(az), sz = std::sin(az);

		float Rx[3][3] = {
			{1,  0,   0},
			{0, cx, -sx},
			{0, sx,  cx}
		};

		float Ry[3][3] = {
			{ cy, 0, sy},
			{  0, 1,  0},
			{-sy, 0, cy}
		};

		float Rz[3][3] = {
			{cz, -sz, 0},
			{sz,  cz, 0},
			{ 0,   0, 1}
		};

		// Вычисление результирующей матрицы: Rz * Ry * Rx
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				calcRxyz[i][j] = 0.f;
				for (int k = 0; k < 3; ++k) {
					float RyRx = 0.0f;
					for (int l = 0; l < 3; ++l) {
						RyRx += Ry[k][l] * Rx[l][j];
					}
					calcRxyz[i][j] += Rz[i][k] * RyRx;
				}
			}
		}
	}

	Point3f Rotate(const Point3f& angle_deg) {
		CalcRotateMatrix(angle_deg);
		return Point3f{
			calcRxyz[0][0] * x + calcRxyz[0][1] * y + calcRxyz[0][2] * z,
			calcRxyz[1][0] * x + calcRxyz[1][1] * y + calcRxyz[1][2] * z,
			calcRxyz[2][0] * x + calcRxyz[2][1] * y + calcRxyz[2][2] * z
		};
	}

private:
	static Point3f calcRxyzAngleDeg;
	static float calcRxyz[3][3];
};

Point3f Point3f::calcRxyzAngleDeg = {NAN, NAN, NAN};
float Point3f::calcRxyz[3][3];

struct Triangle {
	Point3f normal;
	Point3f vertices[3];
};

struct Bboxf {
	Point3f min;
	Point3f max;

	Bboxf& operator+=(const Bboxf& b) {
		min.min(b.min);
		min.min(b.max);
		max.max(b.min);
		max.max(b.max);
		return *this;
	}

	Bboxf& operator+=(const Point3f& p) {
		min.min(p);
		max.max(p);
		return *this;
	}

	Bboxf operator*(float f) const {
		return {min * f, max * f};
	}

	Bboxf operator*(const Point3f& p) const {
		return {min * p, max * p};
	}

	Bboxf Translate(const Point3f& p) {
		return Bboxf{min + p, max + p};
	}

	Bboxf Rotate(const Point3f& angleDeg) {
		if (angleDeg.Length() > 0.01) {
			Point3f rMin = min.Rotate(angleDeg);
			Point3f rMax = max.Rotate(angleDeg);
			rMin.min(rMax); rMax.max(rMin);
			Point3f p = Point3f{min.x, min.y, max.z}.Rotate(angleDeg);
			rMin.min(p); rMax.max(p);
			p = Point3f{min.x, max.y, min.z}.Rotate(angleDeg);
			rMin.min(p); rMax.max(p);
			p = Point3f{min.x, max.y, max.z}.Rotate(angleDeg);
			rMin.min(p); rMax.max(p);
			p = Point3f{max.x, min.y, min.z}.Rotate(angleDeg);
			rMin.min(p); rMax.max(p);
			p = Point3f{max.x, min.y, max.z}.Rotate(angleDeg);
			rMin.min(p); rMax.max(p);
			p = Point3f{max.x, max.y, min.z}.Rotate(angleDeg);
			rMin.min(p); rMax.max(p);
			
			return Bboxf{rMin, rMax};
		}
		return *this;
	}

	Point3f GetSize() {
		return Point3f {
			std::abs(min.x - max.x),
			std::abs(min.y - max.y),
			std::abs(min.z - max.z)
		};
	}

	bool isEmpty() const {
		return (min + max).Length() < 0.001;
	}
};

class Node3D {
private:
	Vector<Node3D*> createdNodes;

protected:
	static int nextId;
	int id;
	Point3f scale = {1.0f, 1.0f, 1.0f};
	Point3f rotate;
	Point3f translate;
	Color color = LtGray;
	bool isSelected = false;
	Bboxf bbox;
	Vector<Triangle> triangles;

	Vector<Node3D*> nodes;
	Node3D* parent = NULL;

public:
	Node3D() {
		id = nextId++;
	}

	virtual Node3D* Duplicate(Node3D* src = NULL, bool uniqIDs = true) {
		if (src == NULL) src = new Node3D();
		if (!uniqIDs) src->id = id;
		src->bbox = bbox;
		src->scale = scale;
		src->rotate = rotate;
		src->translate = translate;
		src->color = color;
		src->isSelected = isSelected;
		src->triangles = clone(triangles);
		Node3D* tmp;
		for (Node3D* n : nodes) {
			tmp = n->Duplicate(NULL, uniqIDs);
			src->createdNodes.Add(tmp);
			src->nodes.Add(tmp);
		}
		return src;
	}

	virtual ~Node3D() {
		for (Node3D* node : createdNodes) {
			delete node;
		}
		createdNodes.Clear();
		nodes.Clear();
	}

	Node3D& Add(Node3D* node, bool autoFree = false) {
		if (node != NULL) {
			nodes.Add(node);
			node->parent = this;
			if (autoFree) createdNodes.Add(node);
		}
		return *this;
	}

	bool Remove(Node3D* node) {
		if (node == NULL) return false;
		for (int i = 0; i < nodes.GetCount(); ++i) {
			if (node == nodes[i]) {
				nodes.Remove(i);
				for (int j = 0; j < createdNodes.GetCount(); ++j) {
					if (node == createdNodes[j]) {
						delete createdNodes[j];
						createdNodes.Remove(j);
						break;
					}
				}
				return true;
			} else {
				if (nodes[i]->Remove(node)) {
					return true;
				}
			}
		}
		return false;
	}

	virtual Node3D& LoadSTL(const String& filepath) {
		triangles.Clear();
		FileIn in(filepath);
		if (!in) {
			LOG("Ошибка открытия файла: " + filepath);
			return *this;
		}

		// Проверка формата
		String header = in.Get(80);
		bool is_ascii = header.StartsWith("solid") && header.Find("endsolid") < 0;
		in.Seek(0);

		if (is_ascii) {
			LoadAsciiSTL(in);
		} else {
			LoadBinarySTL(in);
		}

		if (!triangles.IsEmpty()) {
			bbox = {{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
			for (const Triangle& t : triangles) {
				for (const Point3f& p : t.vertices) {
					bbox += p;
				}
			}
		}
		return *this;
	}

	virtual void GLPaint(bool isSelectMode) {
		glPushMatrix();
		// Преобразования модели
		glScalef(scale.x, scale.y, scale.z);
		glTranslatef(translate.x, translate.y, translate.z);
		glRotatef(rotate.x, 1, 0, 0);
		glRotatef(rotate.y, 0, 1, 0);
		glRotatef(rotate.z, 0, 0, 1);
		
		for (Node3D* node : nodes) {
			node->GLPaint(isSelectMode);
		}
		
		if (triangles.GetCount() > 0) {
			if (isSelectMode) {
				glLoadName(id);
			} else {
				glEnable(GL_LIGHTING);
				if (isSelected) {
					glColor3ub(255, 0, 0);
				} else {
					glColor3ub(color.GetR(), color.GetG(), color.GetB());
				}
			}
			// Отрисовка модели с нормалями
			glBegin(GL_TRIANGLES);
			for (const Triangle& tri : triangles) {
				glNormal3f(tri.normal.x, tri.normal.y, tri.normal.z);
				for (const Point3f& p : tri.vertices) {
					glVertex3f(p.x, p.y, p.z);
				}
			}
			glEnd();
		}
		glPopMatrix();
	}

	Node3D& SetScale(const Point3f& s) {
		scale = s;
		return *this;
	}
	
	Node3D& SetScale(const float s) {
		scale = {s, s, s};
		return *this;
	}

	Node3D& SetRotate(const Point3f& p) {
		rotate = p;
		return *this;
	}

	const Point3f& GetRotate() const & {
		return rotate;
	}

	Node3D& SetTranslate(const Point3f& t) {
		translate = t;
		return *this;
	}

	const Point3f& GetTranslate() const & {
		return translate;
	}

	Node3D& SetColor(const Color& c) {
		color = c;
		return *this;
	}

	const Color& GetColor() const & {
		return color;
	}

	const Bboxf GetBbox() const {
		Bboxf res{{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
		for (Node3D* node : nodes) {
			res += node->GetBbox();
		}
		if (!bbox.isEmpty() || nodes.GetCount() == 0) res += bbox;
		return res.Rotate(rotate).Translate(translate) * scale;
	}

	Node3D& Selected(bool isSel = true, bool recursive = false) {
		isSelected = isSel;
		if (recursive) for (Node3D* node : nodes) node->Selected(isSel, recursive);
	  return *this;
	}

	bool IsSelected() const {
		return isSelected;
	}

	int GetId() const {
		return id;
	}

	Node3D* GetParent() {
		return parent;
	}

	const Vector<Node3D*>& GetChildren() {
		return nodes;
	}

	template <class T>
	T* GetNode(int id) {
		if (id == this->id) return dynamic_cast<T*>(this);
		if (id < 0) return NULL;
		T* res = NULL;
		for (Node3D* node : nodes) {
			if (node->GetId() == id) {
				return dynamic_cast<T*>(node);
			} else if ((res = node->GetNode<T>(id)) != NULL) {
				return res;
			}
		}
		return NULL;
	}
	
	void DrawBbox() {
		for (Node3D* node : nodes) {
			node->DrawBbox();
		}
		const Bboxf bbx = GetBbox();
		glBegin(GL_LINE_STRIP);
			glVertex3f(bbx.min.x, bbx.min.y, bbx.min.z);
			glVertex3f(bbx.min.x, bbx.max.y, bbx.min.z);
			glVertex3f(bbx.max.x, bbx.max.y, bbx.min.z);
			glVertex3f(bbx.max.x, bbx.min.y, bbx.min.z);
			glVertex3f(bbx.min.x, bbx.min.y, bbx.min.z);
			
			glVertex3f(bbx.min.x, bbx.min.y, bbx.max.z);
			glVertex3f(bbx.min.x, bbx.max.y, bbx.max.z);
			glVertex3f(bbx.max.x, bbx.max.y, bbx.max.z);
			glVertex3f(bbx.max.x, bbx.min.y, bbx.max.z);
			glVertex3f(bbx.min.x, bbx.min.y, bbx.max.z);
		glEnd();
		glBegin(GL_LINES);
			glVertex3f(bbx.max.x, bbx.min.y, bbx.min.z);
			glVertex3f(bbx.max.x, bbx.min.y, bbx.max.z);
			
			glVertex3f(bbx.min.x, bbx.max.y, bbx.min.z);
			glVertex3f(bbx.min.x, bbx.max.y, bbx.max.z);
			
			glVertex3f(bbx.max.x, bbx.max.y, bbx.min.z);
			glVertex3f(bbx.max.x, bbx.max.y, bbx.max.z);
		glEnd();
	}
	
private:
	void LoadAsciiSTL(FileIn& in) {
		String line;
		Triangle current;
		int vertex_index = 0;

		while (!in.IsEof()) {
			line = in.GetLine();
			if (line.StartsWith("facet normal")) {
				Vector<String> parts = Split(line, ' ', false);
				if (parts.GetCount() >= 5) {
					current.normal.x = ScanFloat(parts[2]);
					current.normal.y = ScanFloat(parts[3]);
					current.normal.z = ScanFloat(parts[4]);
				}
			} else if (line.StartsWith("vertex")) {
				Vector<String> parts = Split(line, ' ', false);
				if (parts.GetCount() >= 4 && vertex_index < 3) {
					current.vertices[vertex_index].x = ScanFloat(parts[1]);
					current.vertices[vertex_index].y = ScanFloat(parts[2]);
					current.vertices[vertex_index].z = ScanFloat(parts[3]);
					vertex_index++;
				}
			} else if (line.StartsWith("endfacet")) {
				if (vertex_index == 3) triangles.Add(current);
				vertex_index = 0;
			}
		}
	}

	void LoadBinarySTL(FileIn& in) {
		// Пропускаем 80-байтовый заголовок
		in.SeekCur(80);
		
		// Читаем количество треугольников
		uint32 triCount;
		in.Get(&triCount, sizeof(triCount));
		for (uint32 i = 0; i < triCount; i++) {
			Triangle tri;
			// Читаем нормаль
			in.Get(&tri.normal.x, sizeof(float));
			in.Get(&tri.normal.y, sizeof(float));
			in.Get(&tri.normal.z, sizeof(float));
			
			// Читаем вершины
			for (Point3f& p : tri.vertices) {
				in.Get(&p.x, sizeof(float));
				in.Get(&p.y, sizeof(float));
				in.Get(&p.z, sizeof(float));
			}
			triangles.Add(tri);
			
			// Пропускаем атрибуты
			in.SeekCur(2);
		}
	}
	
};

int Node3D::nextId = 1;

struct Square3D : public Node3D {
public:
	Square3D(float cx, float cy, float cz) : Node3D() {
		float c2x = cx / 2.0f;
		float c2y = cy / 2.0f;
		float c2z = cz / 2.0f;

		triangles.Add({{0.0f, 0.0f, -c2z}, {{-c2x, -c2y, -c2z}, {c2x, -c2y, -c2z}, {c2x, c2y, -c2z}}});
		triangles.Add({{0.0f, 0.0f, -c2z}, {{-c2x, -c2y, -c2z}, {-c2x,  c2y, -c2z}, {c2x, c2y, -c2z}}});

		triangles.Add({{-c2x, 0.0f, 0.0f}, {{-c2x, -c2y, -c2z}, {-c2x,  -c2y,  c2z}, {-c2x, c2y, -c2z}}});
		triangles.Add({{-c2x, 0.0f, 0.0f}, {{-c2x, c2y, c2z}, {-c2x,  -c2y,  c2z}, {-c2x, c2y, -c2z}}});

		triangles.Add({{0.0f, 0.0f, c2z}, {{-c2x, -c2y, c2z}, {c2x, -c2y, c2z}, {c2x, c2y, c2z}}});
		triangles.Add({{0.0f, 0.0f, c2z}, {{-c2x, -c2y, c2z}, {-c2x,  c2y, c2z}, {c2x, c2y, c2z}}});

		triangles.Add({{c2x, 0.0f, 0.0f}, {{c2x, -c2y, -c2z}, {c2x,  -c2y,  c2z}, {c2x, c2y, -c2z}}});
		triangles.Add({{c2x, 0.0f, 0.0f}, {{c2x, c2y, c2z}, {c2x,  -c2y,  c2z}, {c2x, c2y, -c2z}}});

		triangles.Add({{0.0f, c2y, 0.0f}, {{c2x, c2y, c2z}, {-c2x,  c2y,  c2z}, {c2x, c2y, -c2z}}});
		triangles.Add({{0.0f, c2y, 0.0f}, {{-c2x, c2y, c2z}, {-c2x,  c2y,  -c2z}, {c2x, c2y, -c2z}}});

		triangles.Add({{0.0f, -c2y, 0.0f}, {{c2x, -c2y, c2z}, {-c2x,  -c2y,  c2z}, {c2x, -c2y, -c2z}}});
		triangles.Add({{0.0f, -c2y, 0.0f}, {{-c2x, -c2y, c2z}, {-c2x,  -c2y,  -c2z}, {c2x, -c2y, -c2z}}});

		bbox = {{-c2x, -c2y, -c2z}, {c2x, c2y, c2z}};
	}
private:
	virtual Node3D& LoadSTL(const String& filepath) { return *this; };
};

#endif