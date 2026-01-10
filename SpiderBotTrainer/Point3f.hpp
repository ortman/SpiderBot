#ifndef _POINT3F_HPP_
#define _POINT3F_HPP_

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
	
	void Jsonize(JsonIO& json) {
		json("x", x)("y", y)("z", z);
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

#endif