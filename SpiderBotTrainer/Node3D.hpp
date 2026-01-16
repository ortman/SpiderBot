#ifndef _NODE_3D_HPP_
#define _NODE_3D_HPP_

#include <GLCtrl/GLCtrl.h>
#include <plugin/glm/glm.hpp>

using namespace Upp;
using namespace glm;
#include "Bboxf.hpp"

class Node3D {
private:
	vec3 scale = {1.0f, 1.0f, 1.0f};
	vec3 rotate;
	vec3 translate;
	mat4 transform = mat4(1.f);
	
	GLuint vao = 0;
	GLuint vbo = 0;
	static ArrayMap<String, Node3D*> nodeTypes;
	static GLuint program;

protected:
	static int nextId;
	int id;
	
	Color color = LtGray;
	bool isSelected = false;
	Bboxf bbox;
	Vector<vec3> points; // XYZ, normal XYZ

	Array<Node3D> nodes;
	Node3D* parent = NULL;
	String stlPath;

public:
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
		GLDeinit();
	}
	
	void Add(Node3D* node) {
		node->parent = this;
		nodes.Add(node);
	}

//	bool Remove(Node3D* node) {
//		if (node == NULL) return false;
//		for (int i = 0; i < nodes.GetCount(); ++i) {
//			if (node == nodes[i]) {
//				nodes.Remove(i);
//				for (int j = 0; j < createdNodes.GetCount(); ++j) {
//					if (node == createdNodes[j]) {
//						delete createdNodes[j];
//						createdNodes.Remove(j);
//						break;
//					}
//				}
//				return true;
//			} else {
//				if (nodes[i]->Remove(node)) {
//					return true;
//				}
//			}
//		}
//		return false;
//	}
	
	void RemoveAll() { nodes.Clear(); }

	virtual Node3D& LoadSTL(const String& filepath) {
		points.Clear();
		FileIn in(filepath);
		if (!in) {
			LOG("Ошибка открытия файла: " + filepath);
			return *this;
		}

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
	
	void DrawObject() {
		if (vao && vbo) {
			glBindVertexArray(vao);
			
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glVertexPointer(3, GL_FLOAT, sizeof(float) * 6, (void*)0);
			glNormalPointer(GL_FLOAT, sizeof(float) * 6, (void*)(3 * sizeof(float)));
			
			glDrawArrays(GL_TRIANGLES, 0, points.GetCount() / 2);
			
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			
			glBindVertexArray(0);
		}
	}

	virtual void GLPaint(const mat4& pv, mat4 t, bool isSelectMode) {
		if (!vao) GLInit();
		glPushMatrix();
		// Преобразования модели
		transform = glm::scale(mat4(1.0f), scale);
		transform = glm::translate(transform, translate);
		transform = glm::rotate(transform, glm::radians(rotate.x), vec3(1, 0, 0));
		transform = glm::rotate(transform, glm::radians(rotate.y), vec3(0, 1, 0));
		transform = glm::rotate(transform, glm::radians(rotate.z), vec3(0, 0, 1));
		
		t = t * transform;
		
		for (Node3D& node : nodes) {
			node.GLPaint(pv, t, isSelectMode);
		}
		
		if (points.GetCount() > 0) {
			if (isSelectMode) {
				glLoadName(id);
			} else {
				GLint colorLoc = glGetUniformLocation(program, "u_color");
				if (isSelected) {
					glUniform4f(colorLoc, 1.f, 0.f, 0.f, 1.f);
				} else {
					glUniform4f(colorLoc, (float)color.GetR() / 255.f, (float)color.GetG() / 255.f, (float)color.GetB() / 255.f, 1.f);
				}
				GLint vpLoc = glGetUniformLocation(program, "u_projection_view");
				glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &pv[0][0]);
				GLint modelLoc = glGetUniformLocation(program, "u_model");
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &t[0][0]);
				glUseProgram(program);
			}
			DrawObject();
		}
		glPopMatrix();
	}

	virtual Node3D& SetScale(const vec3& s) {
		scale = s;
		// TODO
		return *this;
	}
	virtual Node3D& SetScale(const float s) { return SetScale({s, s, s}); }
	virtual Node3D& SetRotate(const vec3& p) {
		rotate = p;
		//TODO
		return *this;
	}
	virtual vec3 GetRotate() const { return rotate; }
	virtual Node3D& SetTranslate(const vec3& t) {
		translate = t;
		//TODO
		return *this;
	}
	virtual const vec3& GetTranslate() const & { return translate; }
	virtual Node3D& SetColor(const Color& c) { color = c; return *this; }
	virtual const Color& GetColor() const & { return color; }
	virtual const Bboxf GetBbox() const {
		Bboxf res{{FLT_MAX, FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX, -FLT_MAX}};
		for (const Node3D& node : nodes) {
			res += node.GetBbox();
		}
		if (!bbox.isEmpty() || nodes.GetCount() == 0) res += bbox;
		return res.Rotate(rotate).Translate(translate) * scale;
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
	
	void DrawBbox() {
		for (Node3D& node : nodes) {
			node.DrawBbox();
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
    
    glBufferData(GL_ARRAY_BUFFER, pointsCount * sizeof(vec3), points.begin(), GL_STATIC_DRAW);

    // Атрибут 0: Координаты (3 float)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (void*)0);

    // Атрибут 1: Нормали (3 float). Смещение 3 float (т.к. сначала идет x,y,z)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3) * 2, (void*)(sizeof(vec3)));

    glBindVertexArray(0);
    InitShaders();
	}
	
	virtual void GLDeinit() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}
	
	void LoadAsciiSTL(FileIn& in) {
		String line;
		vec3 normal, p;
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
		vec3 normal, p;
		// Читаем количество треугольников
		uint32_t triCount;
		in.Get(&triCount, sizeof(triCount));
		for (uint32_t i = 0; i < triCount; i++) {
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
	
	static void InitShaders() {
		if (program) return;
		// Vertex Shader
		const char* vs_src = R"(
			#version 330 core
			layout(location = 0) in vec3 aPos;
			uniform mat4 u_projection_view;
			uniform mat4 u_model;
			uniform vec4 u_color;
			out vec4 vColor;
			
			void main() {
				gl_Position = u_projection_view * u_model * vec4(aPos, 1.0);
				vColor = u_color;
			}
		)";
		// Fragment Shader
		const char* fs_src = R"(
			#version 330 core
			in vec4 vColor;
			out vec4 FragColor;
			
			//vec3 lightPos     = vec3(15.0, 15.0, 15.0);
	    //vec3 lightAmbient  = vec3(0.2, 0.2, 0.2);
	    //vec3 lightDiffuse  = vec3(0.6, 0.6, 0.6);
	    //vec3 lightSpecular = vec3(0.5, 0.5, 0.5);
	
	    // --- КОНСТАНТЫ МАТЕРИАЛА ---
	    //vec3 matAmbient   = vec3(0.2, 0.4, 0.7);
	    //vec3 matDiffuse   = vec3(0.3, 0.6, 0.9);
	    //vec3 matSpecular  = vec3(0.8, 0.8, 0.8);
	    //float matShininess = 50.0;
			
			void main() {
				// 1. Ambient (Фоновое)
		    //vec3 ambient = lightAmbient * matAmbient;
		
		    // 2. Diffuse (Рассеянное)
		    //vec3 norm = normalize(vNormal);
		    //vec3 lightDir = normalize(lightPos - vFragPos);
		    //float diff = max(dot(norm, lightDir), 0.0);
		    //vec3 diffuse = lightDiffuse * (diff * matDiffuse);
		
		    // 3. Specular (Блики)
		    //vec3 viewDir = normalize(u_viewPos - vFragPos);
		    //vec3 reflectDir = reflect(-lightDir, norm);
		    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
		    //vec3 specular = lightSpecular * (spec * matSpecular);
		    
		    // Итоговый цвет
				//vec3 result = ambient + diffuse + specular;
				//FragColor = vec4(result, 1.0);
				FragColor = vColor;
			}
		)";
		
		auto compile = [](GLenum type, const char* src) {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        return s;
    };

    GLuint vs = compile(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fs_src);

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    char buffer[512];
		glGetProgramInfoLog(program, 512, NULL, buffer);
		if (strlen(buffer) > 0) {
			LOG("Shader Error: " << buffer);
		}
	}
};

int Node3D::nextId = 1;
ArrayMap<String, Node3D*> Node3D::nodeTypes;
GLuint Node3D::program = 0;

INITBLOCK {
	Node3D::Register<Node3D>();
}

namespace Upp {
    void Jsonize(JsonIO& io, glm::vec3& v) {
        double x = v.x, y = v.y, z = v.z;
        io("x", x)("y", y)("z", z);
        if(io.IsLoading()) {
            v.x = (float)x;
            v.y = (float)y;
            v.z = (float)z;
        }
    }
}

#endif