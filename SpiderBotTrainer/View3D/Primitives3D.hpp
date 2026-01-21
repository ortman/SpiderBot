#ifndef _PRIMITIVES_3D_HPP_
#define _PRIMITIVES_3D_HPP_

#include "Node3D.hpp"

struct Primitive : public Node3D {
protected:
	void AddTriangle(vec3 p1, vec3 p2, vec3 p3, vec3 n1, vec3 n2, vec3 n3) {
		points.Add({p1, n1});
		points.Add({p2, n2});
		points.Add({p3, n3});
	}
	
  void AddFlatTriangle(vec3 p1, vec3 p2, vec3 p3, vec3 n) {
      AddTriangle(p1, p2, p3, n, n, n);
  }
};

struct Rect3D : public Primitive {
private:
	void AddFace(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec3 n) {
		AddFlatTriangle(p1, p2, p3, n);
		AddFlatTriangle(p1, p3, p4, n);
	}
public:
	Rect3D() = default;
	Rect3D(float cx, float cy, float cz) : Primitive() {
		float x = cx / 2.0f;
		float y = cy / 2.0f;
		float z = cz / 2.0f;

    AddFace({-x, -y,  z}, { x, -y,  z}, { x,  y,  z}, {-x,  y,  z}, {0, 0, 1});
    AddFace({ x, -y, -z}, {-x, -y, -z}, {-x,  y, -z}, { x,  y, -z}, {0, 0, -1});
    AddFace({-x,  y,  z}, { x,  y,  z}, { x,  y, -z}, {-x,  y, -z}, {0, 1, 0});
    AddFace({-x, -y, -z}, { x, -y, -z}, { x, -y,  z}, {-x, -y,  z}, {0, -1, 0});
    AddFace({ x, -y,  z}, { x, -y, -z}, { x,  y, -z}, { x,  y,  z}, {1, 0, 0});
    AddFace({-x, -y, -z}, {-x, -y,  z}, {-x,  y,  z}, {-x,  y, -z}, {-1, 0, 0});

		bbox = {{-x, -y, -z}, {x, y, z}};
	}
	
  virtual void Jsonize(JsonIO& json) override {
		Node3D::Jsonize(json);
	}
};

INITBLOCK {
	Node3D::Register<Rect3D>();
}

struct Cylinder3D : public Primitive {
private:
	// Вспомогательный метод для боковой грани (разные нормали для сглаживания)
	void AddSideFace(vec3 p1, vec3 p2, vec3 p3, vec3 p4, vec3 n1, vec3 n2) {
		AddTriangle(p1, p2, p3, n1, n2, n1);
		AddTriangle(p2, p4, p3, n2, n2, n1);
	}
public:
	Cylinder3D() = default;
    Cylinder3D(vec3 start, vec3 end, float radius, float stepLength) : Primitive() {
        vec3 dir = end - start;
        float height = length(dir);
        if (height < 1e-6f) return;
        
        vec3 axis = normalize(dir);

        // Вычисляем количество сегментов исходя из длины отрезка
        int segments = UPP::max(3, (int)(M_2PI * radius / stepLength));

        // Построение базиса (векторы 'right' и 'up', перпендикулярные оси цилиндра)
        vec3 temp = (abs(axis.y) > 0.99f) ? vec3(1, 0, 0) : vec3(0, 1, 0);
        vec3 right = normalize(cross(temp, axis));
        vec3 up = cross(axis, right);

        for (int i = 0; i < segments; i++) {
            float a1 = (float)(M_2PI * i / segments);
            float a2 = (float)(M_2PI * (i + 1) / segments);

            // Направления радиусов
            vec3 r1 = right * cos(a1) + up * sin(a1);
            vec3 r2 = right * cos(a2) + up * sin(a2);

            // Точки на нижнем и верхнем круге
            vec3 b1 = start + r1 * radius; // bottom 1
            vec3 b2 = start + r2 * radius; // bottom 2
            vec3 t1 = end + r1 * radius;   // top 1
            vec3 t2 = end + r2 * radius;   // top 2

            // 1. Боковая поверхность (используем радиус-векторы как нормали для гладкости)
            AddSideFace(b1, b2, t1, t2, r1, r2);

            // 2. Нижняя крышка
            AddFlatTriangle(start, b2, b1, -axis);

            // 3. Верхняя крышка
            AddFlatTriangle(end, t1, t2, axis);
        }

        // Условный bbox (упрощенно)
        bbox = { min(start - radius, end - radius), max(start + radius, end + radius) };
    }

    virtual void Jsonize(JsonIO& json) override {
			Node3D::Jsonize(json);
		}
};

INITBLOCK {
	Node3D::Register<Cylinder3D>();
}

#endif