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

		// X -> Y -> Z
		// calcRxyz[0][0] = cy * cz;
		// calcRxyz[0][1] = sx * sy * cz - cx * sz;
		// calcRxyz[0][2] = cx * sy * cz + sx * sz;
		// calcRxyz[1][0] = cy * sz;
		// calcRxyz[1][1] = sx * sy * sz + cx * cz;
		// calcRxyz[1][2] = cx * sy * sz - sx * cz;
		// calcRxyz[2][0] = -sy;
		// calcRxyz[2][1] = sx * cy;
		// calcRxyz[2][2] = cx * cy;
		
		// Z -> Y -> X
		calcRxyz[0][0] = cy * cz;
		calcRxyz[0][1] = -cy*sz;
		calcRxyz[0][2] = sy;
		calcRxyz[1][0] = cx*sz + sx*sy*cz;
		calcRxyz[1][1] = cx*cz - sx*sy*sz;
		calcRxyz[1][2] = -sx*cy;
		calcRxyz[2][0] = sx*sz - cx*sy*cz;
		calcRxyz[2][1] = sx*cz + cx*sy*sz;
		calcRxyz[2][2] = cx*cy;
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
	GLuint vao = 0;
	GLuint vbo = 0;

protected:
	static int nextId;
	int id;
	Point3f scale = {1.0f, 1.0f, 1.0f};
	Point3f rotate;
	Point3f translate;
	Color color = LtGray;
	bool isSelected = false;
	Bboxf bbox;
	Vector<Point3f> points; // XYZ, normal XYZ

	Vector<Node3D*> nodes;
	Node3D* parent = NULL;
	String stlPath;

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
		src->points = clone(points);
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
		GLDeinit();
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
		points.Clear();
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

		if (!points.IsEmpty()) {
			bbox = {{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
			int pointsCount = points.GetCount();
			for (int i = 0; i < pointsCount; ++i) {
				bbox += points[i++];
			}
		}
		stlPath = filepath;
		return *this;
	}
	
	const String& GetSTLPath() {
		return stlPath;
	}
	
	void Jsonize(JsonIO& json) {
		String c = ColorToHtml(color);
		json("STL", stlPath)("Color", c);
		if (nodes.GetCount()) {
			//json("nodes", nodes);
		}
	}

	virtual void GLPaint(bool isSelectMode) {
		if (!vao) GLInit();
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
		
		if (points.GetCount() > 0) {
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
			
			if (vao && vbo) {
				glBindVertexArray(vao);
				
				glEnableClientState(GL_VERTEX_ARRAY);
				glEnableClientState(GL_NORMAL_ARRAY);
				
				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				glVertexPointer(3, GL_FLOAT, sizeof(Point3f) * 2, (void*)0);
				glNormalPointer(GL_FLOAT, sizeof(Point3f) * 2, (void*)(3 * sizeof(float)));
				
				glDrawArrays(GL_TRIANGLES, 0, points.GetCount() / 2);
				
				glDisableClientState(GL_NORMAL_ARRAY);
				glDisableClientState(GL_VERTEX_ARRAY);
				
				glBindVertexArray(0);
			}
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

	virtual Node3D& SetRotate(const Point3f& p) {
		rotate = p;
		return *this;
	}

	virtual Point3f GetRotate() const {
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
	virtual void GLInit() {
		int pointsCount = points.GetCount();
		if (pointsCount == 0) return;
		
		if (!vao) glGenVertexArrays(1, &vao);
    if (!vbo) glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    glBufferData(GL_ARRAY_BUFFER, pointsCount * sizeof(Point3f), points.begin(), GL_STATIC_DRAW);

    // Атрибут 0: Координаты (3 float)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point3f) * 2, (void*)0);

    // Атрибут 1: Нормали (3 float). Смещение 3 float (т.к. сначала идет x,y,z)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point3f) * 2, (void*)(sizeof(Point3f)));

    glBindVertexArray(0);
	}
	
	virtual void GLDeinit() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
	
	void LoadAsciiSTL(FileIn& in) {
		String line;
		Point3f normal, p;
		while (!in.IsEof()) {
			line = TrimBoth(in.GetLine());
			Vector<String> tokens = Split(line, ' ', true);
			if (tokens.IsEmpty()) continue;
			if (tokens[0] == "facet" && tokens.GetCount() >= 5) {
				// facet normal ni nj nk
				normal.x = (float)ScanDouble(tokens[2]);
				normal.y = (float)ScanDouble(tokens[3]);
				normal.z = (float)ScanDouble(tokens[4]);
			}	else if (tokens[0] == "vertex" && tokens.GetCount() >= 4) {
				// vertex x y z
				p.x = (float)ScanDouble(tokens[1]);
				p.y = (float)ScanDouble(tokens[2]);
				p.z = (float)ScanDouble(tokens[3]);

				points.Add(p);
				points.Add(normal);
			}
		}
	}

	void LoadBinarySTL(FileIn& in) {
		// Пропускаем 80-байтовый заголовок
		in.SeekCur(80);
		Point3f normal, p;
		// Читаем количество треугольников
		uint32 triCount;
		in.Get(&triCount, sizeof(triCount));
		for (uint32 i = 0; i < triCount; i++) {
			// Читаем нормаль
			in.Get(&normal.x, sizeof(float));
			in.Get(&normal.y, sizeof(float));
			in.Get(&normal.z, sizeof(float));
			
			// Читаем вершины
			for (int i = 0; i < 3; ++i) {
				in.Get(&p.x, sizeof(float));
				in.Get(&p.y, sizeof(float));
				in.Get(&p.z, sizeof(float));
				points.Add(p);
				points.Add(normal);
			}
			
			// Пропускаем атрибуты
			in.SeekCur(2);
		}
	}
};

int Node3D::nextId = 1;

struct Square3D : public Node3D {
public:
	Square3D(float cx, float cy, float cz) : Node3D() {
		float x = cx / 2.0f;
		float y = cy / 2.0f;
		float z = cz / 2.0f;
		
    auto AddTriangle = [&](Point3f p1, Point3f p2, Point3f p3, Point3f n) {
        points.Add(p1); points.Add(n);
        points.Add(p2); points.Add(n);
        points.Add(p3); points.Add(n);
    };

    auto AddFace = [&](Point3f p1, Point3f p2, Point3f p3, Point3f p4, Point3f n) {
        AddTriangle(p1, p2, p3, n);
        AddTriangle(p1, p3, p4, n);
    };

    AddFace({-x, -y,  z}, { x, -y,  z}, { x,  y,  z}, {-x,  y,  z}, {0, 0, 1});
    AddFace({ x, -y, -z}, {-x, -y, -z}, {-x,  y, -z}, { x,  y, -z}, {0, 0, -1});
    AddFace({-x,  y,  z}, { x,  y,  z}, { x,  y, -z}, {-x,  y, -z}, {0, 1, 0});
    AddFace({-x, -y, -z}, { x, -y, -z}, { x, -y,  z}, {-x, -y,  z}, {0, -1, 0});
    AddFace({ x, -y,  z}, { x, -y, -z}, { x,  y, -z}, { x,  y,  z}, {1, 0, 0});
    AddFace({-x, -y, -z}, {-x, -y,  z}, {-x,  y,  z}, {-x,  y, -z}, {-1, 0, 0});

		bbox = {{-x, -y, -z}, {x, y, z}};
	}
private:
	virtual Node3D& LoadSTL(const String& filepath) { return *this; };
};

#endif