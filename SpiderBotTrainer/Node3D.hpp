#ifndef _NODE_3D_HPP_
#define _NODE_3D_HPP_

#include <GLCtrl/GLCtrl.h>

using namespace Upp;

struct Point3f {
  float x, y, z;
  Point3f() = default;
  Point3f(float x, float y, float z) : x(x), y(y), z(z) {}
  
  Point3f operator+(const Point3f& p) const { return {x + p.x, y + p.y, z + p.z}; }
  Point3f operator-(const Point3f& p) const { return {x - p.x, y - p.y, z - p.z}; }
  Point3f operator*(const Point3f& p) const { return {x * p.x, y * p.y, z * p.z}; }
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
};
	
struct Triangle {
  Point3f normal;
  Point3f vertices[3];
};

struct Node3D {
private:
	Vector<Node3D*> nodes;
	int id;
	
  Point3f scale;
  Point3f rotate;
  Point3f translate;
  Color color = LtGray;
  bool isSelected = false;

protected:
  Point3f min;
  Point3f max;
	Vector<Triangle> triangles;
  static int nextId;
	
public:
	Node3D() {
		id = nextId++;
	  min = {0.0f, 0.0f, 0.0f};
	  max = {0.0f, 0.0f, 0.0f};
	  scale = {1.0f, 1.0f, 1.0f};
	  rotate = {0.0f, 0.0f, 0.0f};
	  translate = {0.0f, 0.0f, 0.0f};
	}
	
	Node3D& Add(Node3D *node) {
		nodes.Add(node);
	  return *this;
	}
	
	virtual Node3D& LoadSTL(const String& filepath) {
		triangles.Clear();
		FileIn in(filepath);
		if (!in) {
			//error = "Ошибка открытия файла: " + filepath;
			//Refresh();
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
			min = {FLT_MAX, FLT_MAX, FLT_MAX};
			max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
			for (const auto& tri : triangles) {
				for (const auto& pt : tri.vertices) {
					min.min(pt);
					max.max(pt);
				}
			}
		}
		return *this;
	}
	
	virtual void GLPaint(bool isSelectMode) {
		// Преобразования модели
		glScalef(scale.x, scale.y, scale.z);
		glTranslatef(translate.x, translate.y, translate.z);
		glRotatef(rotate.x, 1, 0, 0);
		glRotatef(rotate.y, 0, 1, 0);
		glRotatef(rotate.z, 0, 0, 1);
		
    for (Node3D* node : nodes) {
      glPushMatrix();
      node->GLPaint(isSelectMode);
      glPopMatrix();
    }
		
		if (triangles.GetCount() > 0) {
			if (isSelectMode) {
				glLoadName(id);
			} else {
				if (isSelected) {
					glColor3ub(255, 0, 0);
				} else {
					glColor3ub(color.GetR(), color.GetG(), color.GetB());
				}
			}
			// Отрисовка модели с нормалями
			glBegin(GL_TRIANGLES);
			for (const auto& tri : triangles) {
				glNormal3f(tri.normal.x, tri.normal.y, tri.normal.z);
				for (int i = 0; i < 3; i++) {
					glVertex3f(tri.vertices[i].x, tri.vertices[i].y, tri.vertices[i].z);
				}
			}
			glEnd();
		}
	}
	
	Node3D& Scale(const Point3f& s) {
		scale = s;
	  return *this;
	}
	
	Node3D& Rotate(const Point3f& p) {
		rotate = p;
	  return *this;
	}
	
	Node3D& Translate(const Point3f& t) {
		translate = t;
	  return *this;
	}
	
	Node3D& SetColor(const Color& c) {
		color = c;
	  return *this;
	}
	
	Point3f GetMin() {
		Point3f res = (min * scale - translate);
		for (Node3D* node : nodes) res.min(node->GetMin());
		return res;
	}
	
	Point3f GetMax() {
		Point3f res = (max * scale + translate);
		for (Node3D* node : nodes) res.min(node->GetMax());
		return res;
	}
	
	Node3D& Selected(bool isSel = true) {
		isSelected = isSel;
		for (Node3D* node : nodes) {
      node->Selected(isSel);
    }
	  return *this;
	}
	
	bool IsSelected() {
		return isSelected;
	}
	
	int GetId() {
		return id;
	}
	
	Node3D* GetNode(int id) {
		if (id == this->id) return this;
		Node3D* node = NULL;
		if (id < 0) return node;
		for (Node3D* n : nodes) {
	    if (n->GetId() == id) {
				return n;
	    } else if ((node = n->GetNode(id)) != NULL) {
	      return node;
	    }
	  }
		return node;
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
			for (int v = 0; v < 3; v++) {
				in.Get(&tri.vertices[v].x, sizeof(float));
				in.Get(&tri.vertices[v].y, sizeof(float));
				in.Get(&tri.vertices[v].z, sizeof(float));
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
		
	  min = {-c2x, -c2y, -c2z};
	  max = {c2x, c2y, c2z};
	}
private:
	virtual Node3D& LoadSTL(const String& filepath) { return *this; };
};

#endif