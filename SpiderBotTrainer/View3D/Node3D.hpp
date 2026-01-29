#ifndef _NODE_3D_HPP_
#define _NODE_3D_HPP_

#include <GLCtrl/GLCtrl.h>
#include "ShaderModel.hpp"
#include "ShaderSelect.hpp"
#include "ShaderFlat.hpp"
#include "ShaderGetNode.hpp"
#include "Types.hpp"

using namespace Upp;
#include "Bboxf.hpp"

class Node3D {
private:
	vec3 scale = {1.0f, 1.0f, 1.0f};
	vec3 rotate = {0.0f, 0.0f, 0.0f};
	vec3 translate = {0.0f, 0.0f, 0.0f};
	mat4 transform = mat4(1.f);
	
	GLuint vao = 0;
	GLuint vbo = 0;
	static ArrayMap<String, Node3D*> nodeTypes;

protected:
	static int nextId;
	int id;
	
	Color color = LtGray;
	bool isSelected = false;
	Bboxf bbox;
	Vector<View3D_Point_t> points;

	Array<Node3D> nodes;
	Node3D* parent = NULL;
	String stlPath;
	
	void UpdateTranfsormMatrix() {
		transform = glm::scale(mat4(1.0f), scale);
		transform = glm::translate(transform, translate);
		transform = glm::rotate(transform, glm::radians(rotate.x), vec3(1, 0, 0));
		transform = glm::rotate(transform, glm::radians(rotate.y), vec3(0, 1, 0));
		transform = glm::rotate(transform, glm::radians(rotate.z), vec3(0, 0, 1));
	}

public:
	static ShaderModel shaderModel;
	static ShaderSelect shaderSelect;
	static ShaderFlat shaderFlat;
	static ShaderGetNode shaderGetNode;
	
	Node3D() {
		id = nextId++;
	}
	
	template <class T>
	static void Register() {
		T* n = new T();
		String type = typeid(*n).name();
		nodeTypes.FindAdd(type, n);
	}

	virtual Node3D* Copy(Node3D* node = NULL) const {
		if (node == NULL) node = new Node3D();
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
		return node;
	}

	virtual ~Node3D() {
		nodes.Clear();
		Shader::VaoDeinit(vao, vbo);
	}
	
	void Add(Node3D* node) {
		node->parent = this;
		nodes.Add(node);
	}

	bool Remove(Node3D* node) {
		if (node == NULL) return false;
		for (int i = 0; i < nodes.GetCount(); ++i) {
			if (node == &nodes[i]) {
				nodes.Remove(i);
				return true;
			} else {
				if (nodes[i].Remove(node)) return true;
			}
		}
		return false;
	}
	
	void RemoveAll() { nodes.Clear(); }

	Node3D& LoadSTL(const String& filepath) {
		FileIn in(filepath);
		if (!in) {
			LOG("Ошибка открытия файла: " + filepath);
			return *this;
		}
		points.Clear();

		String header = in.Get(80);
		bool is_ascii = header.StartsWith("solid") && header.Find("endsolid") < 0;
		in.Seek(0);

		if (is_ascii) {
			LoadAsciiSTL(in);
		} else {
			LoadBinarySTL(in);
		}
		String appFolder = GetExeFolder();
		if (filepath.StartsWith(appFolder)) {
			stlPath = filepath.Mid(appFolder.GetLength() + 1);
		} else {
			stlPath = filepath;
		}
		return *this;
	}
	
	const String& GetSTLPath() {
		return stlPath;
	}
	
	virtual void Jsonize(JsonIO& json) {
		String type = typeid(*this).name();
		json("type", type);
		json("STL", stlPath);
		json("scale", scale)("rotate", rotate)("translate", translate);
		json("Color", color);
		if (json.IsLoading()) {
			if (!stlPath.IsEmpty()) LoadSTL(stlPath);
			UpdateTranfsormMatrix();

			const Value& va = json.Get("nodes");
			nodes.Clear();
			for(int i = 0; i < va.GetCount(); i++) {
				JsonIO jio(va[i]);
				String t = jio.Get("type");
				Node3D* node = nodeTypes.Get(t, NULL);
				if (node) {
					node = node->Copy();
					node->Jsonize(jio);
					nodes.Add(node);
				} else {
					nodes[i].Jsonize(jio);
				}
			}
		} else {
			if (nodes.GetCount()) {
				json("nodes", nodes);
			}
		}
	}

	virtual void GLPaint(const mat4& pv, const vec3& cameraPos, mat4 t, bool isSelectMode) {
		if (!vao) Shader::VaoTrianglesInit(points, vao, vbo);
		
		t = t * transform;
		
		for (Node3D& node : nodes) {
			node.GLPaint(pv, cameraPos, t, isSelectMode);
		}
		
		if (points.GetCount() > 0) {
			if (isSelectMode) {
				shaderGetNode.Use();
				shaderGetNode.SetModel(t);
				shaderGetNode.SetPV(pv);
				shaderGetNode.SetId(id);
			} else {
				vec4 u_color = vec4((float)color.GetR() / 255.f, (float)color.GetG() / 255.f, (float)color.GetB() / 255.f, 1.0f);
				if (isSelected) u_color *= 1.5f;
				shaderModel.Use();
				shaderModel.SetModel(t);
				shaderModel.SetPV(pv);
				shaderModel.SetColor(u_color);
				shaderModel.SetViewPos(cameraPos);
			}
			Shader::DrawObject(vao, vbo, points.GetCount());
		}
	}

	virtual vec3 GetScale() const { return scale; }
	virtual Node3D& SetScale(const vec3& s) {
		scale = s;
		UpdateTranfsormMatrix();
		return *this;
	}
	Node3D& SetScale(const float s) { return SetScale({s, s, s}); }
	virtual Node3D& SetRotate(const vec3& p) {
		rotate = p;
		UpdateTranfsormMatrix();
		return *this;
	}
	virtual vec3 GetRotate() const { return rotate; }
	virtual Node3D& SetTranslate(const vec3& t) {
		translate = t;
		UpdateTranfsormMatrix();
		return *this;
	}
	virtual const vec3& GetTranslate() const & { return translate; }
	virtual Node3D& SetColor(const Color& c) { color = c; return *this; }
	virtual const Color& GetColor() const & { return color; }
	virtual const Bboxf GetBbox() const {
		Bboxf res{{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
		for (const Node3D& node : nodes) {
			res += node.GetBbox().Transform(transform);
		}
		if (!bbox.isEmpty() || nodes.GetCount() == 0) res += bbox;
		return res;
	}

	Node3D& Selected(bool isSel = true, bool recursive = false) {
		isSelected = isSel;
		if (recursive) for (Node3D& node : nodes) node.Selected(isSel, recursive);
	  return *this;
	}

	bool IsSelected() const { return isSelected; }
	int GetId() const { return id; }
	Node3D& SetId(int id) { this->id = id; return *this; }
	Node3D* GetParent() { return parent; }
	Array<Node3D>& GetChildren() { return nodes; }
	const Array<Node3D>& GetChildren() const { return nodes; }

	template <class T>
	T* GetNode(int id) {
		if (id == this->id) return dynamic_cast<T*>(this);
		if (id < 0) return NULL;
		T* res = NULL;
		for (Node3D& node : nodes) {
			if ((res = node.GetNode<T>(id)) != NULL) break;
		}
		return res;
	}
	
	void DrawBbox(Shader& shader) {
		for (Node3D& node : nodes) {
			node.DrawBbox(shader);
		}
		shader.SetModel(glm::scale(mat4(1.f), GetBbox().GetSize())); //TODO GetBbox() recalculate bbox
		bbox.GLPaint();
	}
	
private:
	
	void LoadAsciiSTL(FileIn& in) {
		String line;
		View3D_Point_t point;
		bbox = {{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
		while (!in.IsEof()) {
			line = TrimBoth(in.GetLine());
			Vector<String> tokens = Split(line, ' ', true);
			if (tokens.IsEmpty()) continue;
			if (tokens[0] == "facet" && tokens.GetCount() >= 5) {
				// facet normal ni nj nk
				point.n.x = (float)ScanDouble(tokens[2]);
				point.n.y = (float)ScanDouble(tokens[3]);
				point.n.z = (float)ScanDouble(tokens[4]);
			}	else if (tokens[0] == "vertex" && tokens.GetCount() >= 4) {
				// vertex x y z
				point.p.x = (float)ScanDouble(tokens[1]);
				point.p.y = (float)ScanDouble(tokens[2]);
				point.p.z = (float)ScanDouble(tokens[3]);

				points.Add(point);
				bbox += point.p;
			}
		}
	}

	void LoadBinarySTL(FileIn& in) {
		bbox = {{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
		// Seek 80 bytes header
		in.SeekCur(80);
		View3D_Point_t point;
		uint32_t triCount;
		in.Get(&triCount, sizeof(triCount));
		for (uint32_t i = 0; i < triCount; i++) {
			in.Get(&point.n, sizeof(vec3));
			for (int i = 0; i < 3; ++i) {
				in.Get(&point.p, sizeof(vec3));
				points.Add(point);
				bbox += point.p;
			}
			
			// Seek atributes
			in.SeekCur(2);
		}
	}
};

int Node3D::nextId = 1;
ArrayMap<String, Node3D*> Node3D::nodeTypes;

ShaderModel   Node3D::shaderModel;
ShaderSelect  Node3D::shaderSelect;
ShaderFlat    Node3D::shaderFlat;
ShaderGetNode Node3D::shaderGetNode;

INITBLOCK {
	Node3D::Register<Node3D>();
}

namespace Upp {
    void Jsonize(JsonIO& io, glm::vec3& v) {
        double x = v.x, y = v.y, z = v.z;
        io("x", x)("y", y)("z", z);
        if (io.IsLoading()) {
            v.x = (float)x;
            v.y = (float)y;
            v.z = (float)z;
        }
    }
}

#endif