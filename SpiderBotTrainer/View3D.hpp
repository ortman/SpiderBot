#ifndef _VIEW_3D_HPP_
#define _VIEW_3D_HPP_

#include "Foot3D.hpp"

using namespace Upp;

class View3D : public GLCtrl {
private:
	Point mouseLeftStart, mouseRightStart, mouseLeftClickPos;
  float scale = 1.0f;
  float distance = 2.0f;      // Начальное расстояние
  float azimuth = (float)(-M_PI / 4.0);    // Начальный угол
  float elevation = (float)(M_PI / 6.0);   // Начальный угол
  bool mouseLeftDown = false;
  bool mouseRightDown = false;
  
	Vector<Node3D*> nodes;
	Point3f cameraPos = {0.8f, -1.0f, 0.5f};
	Point3f cameraCenter = {0.0f, 0.0f, 0.0f};
  Point3f pivotPoint = {0.0f, 0.0f, 0.0f}; // Центр вращения
  
  // Обновляем позицию камеры на основе углов
  void UpdateCameraPosition() {
    cameraPos.x = pivotPoint.x + distance * cos(elevation) * sin(azimuth);
    cameraPos.y = pivotPoint.y + distance * cos(elevation) * cos(azimuth);
    cameraPos.z = pivotPoint.z + distance * sin(elevation);
    cameraCenter = pivotPoint;
  }
	
public:
	View3D() {
		UpdateCameraPosition();
	}
	
	View3D& Add(Node3D *node) {
		nodes.Add(node);
	  return *this;
	}
	
	View3D& ViewAll() {
	  Point3f min(FLT_MAX, FLT_MAX, FLT_MAX);
	  Point3f max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	  
	  for (Node3D* node : nodes) {
	    min.min(node->GetMin());
	    max.max(node->GetMax());
	  }
	  float size_x = max.x - min.x;
    float size_y = max.y - min.y;
    float size_z = max.z - min.z;

    //pivotPoint = (min + max) / 2.0f;  //TODO uncomment
    float size = std::max({size_x, size_y, size_z});
    distance = size * 2.0f; // Автоматический зум
    
    distance *= 1.8f; //TODO remove
    
    // Обновляем позицию камеры
    UpdateCameraPosition();
    return *this;
	}
  
	View3D& SetScale(float s) {
		scale = s;
		return *this;
	}
  
	int GetNodeId(const Point &p) {
		int selectedId = -1;
    ExecuteGL([&] {
      GLuint selectBuf[512] = {0};
      GLint viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);
      
      glSelectBuffer(512, selectBuf);
      glRenderMode(GL_SELECT);
      
      glInitNames();
      glPushName(0);
      
      // Настройка проекции
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      
      // Область выбора вокруг курсора
      gluPickMatrix(p.x, viewport[3] - p.y, 1.0, 1.0, viewport);
      Size sz = GetSize();
			gluPerspective(45.0, (double)sz.cx / sz.cy, 0.1, 2000.0);
      
      // Настройка вида (должна совпадать с GLPaint)
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
			gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, cameraCenter.x, cameraCenter.y, cameraCenter.z, 0., 0., 1.);
			glScalef(scale, scale, scale);
			
      // Подготовка к рендерингу для выбора
      glClear(GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);
      glDisable(GL_COLOR_MATERIAL);
      
      // Рисуем модели в режиме выбора
	    for (Node3D* node : nodes) {
	      glPushMatrix();
	      node->GLPaint(true);
	      glPopMatrix();
	    }
      glFlush();
      
      // Анализ результатов выбора
      GLint hits = glRenderMode(GL_RENDER);
      
      // Восстановление матриц
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      
      // Обработка попаданий
      if (hits <= 0) {
        selectedId = -1;
        return;
      }
      
      GLuint minDepth = 0xFFFFFFFF;
      GLuint* ptr = selectBuf;
      
      for (int i = 0; i < hits; ++i) {
        GLuint numNames = *ptr++;
        GLuint minZ = *ptr++;
        ptr++; // пропускаем maxZ
        
        if (numNames > 0) {
          GLuint id = *ptr;
          if (minZ < minDepth) {
            minDepth = minZ;
            selectedId = id;
          }
        }
        ptr += numNames;
      }
      
    });
    return selectedId;
	}
	
	Node3D* GetNode(int id) {
		Node3D* node = NULL;
		if (id < 0) return node;
		for (Node3D* n : nodes) {
			node = n->GetNode(id);
	    if (node != NULL) break;
	  }
		return node;
	}
	
	void SelectNode(Node3D* node) {
		bool finded = false;
		for (Node3D* n : nodes) {
	    n->Selected(n == node);
	    if (n == node) finded = true;
	  }
	  if (!finded && node != NULL) node->Selected(true);
	}
	
	void SelectNode(int id) {
		SelectNode(GetNode(id));
	}
	
private:
	virtual void MouseMove(Point p, dword keyflags) {
		//if (keyflags & K_MOUSELEFT) {
    if (mouseLeftDown) { // Вращение ЛКМ
      float dx = (float)(p.x - mouseLeftStart.x) * 0.01f;
      float dy = (float)(p.y - mouseLeftStart.y) * 0.01f;
      
      azimuth += dx;
      elevation += dy;
      
      // Ограничиваем вертикальный угол
      const float maxElevation = (float)M_PI/2 - 0.01f;
      elevation = std::max(-maxElevation, std::min(elevation, maxElevation));
      
      UpdateCameraPosition();
      mouseLeftStart = p;
      Refresh();
    // } else if (keyflags & K_MOUSERIGHT) {
    } else if (mouseRightDown) { // Панорамирование ПКМ
      float dx = (float)(p.x - mouseRightStart.x) * -0.001f;
      float dy = (float)(p.y - mouseRightStart.y) * 0.001f;
      
      Point3f dir = (cameraCenter - cameraPos).Normalize();
      Point3f right = dir.Cross(Point3f(0.0f, 0.0f, 1.0f)).Normalize();
      Point3f up = right.Cross(dir).Normalize();
      
      pivotPoint = pivotPoint + right * dx * distance + up * dy * distance;
      UpdateCameraPosition();
      mouseRightStart = p;
      Refresh();
    }
	}
	
	virtual void LeftDown(Point p, dword keyflags) {
    mouseLeftClickPos = mouseLeftStart = p;
    mouseLeftDown = true;
    SetCapture();
	}
	
	virtual void LeftUp(Point p, dword keyflags) {
    if (p == mouseLeftClickPos) {
      SelectNode(GetNodeId(p));
      Refresh();
    }
    mouseLeftDown = false;
    ReleaseCapture();
	}
	
	virtual void RightDown(Point p, dword keyflags) {
    mouseRightStart = p;
    mouseRightDown = true;
    SetCapture();
	}
	
	virtual void RightUp(Point p, dword keyflags) {
    mouseRightDown = false;
    ReleaseCapture();
	}
	
	virtual void MouseWheel(Point p, int zdelta, dword keyflags) {
    float factor = (zdelta > 0) ? 0.9f : 1.1f;
    distance *= factor;
    
    // Ограничиваем расстояние
    distance = std::max(0.1f, std::min(distance, 1000.0f));
    
    UpdateCameraPosition();
    Refresh();
	}
		
  virtual void GLResize(int w, int h) {
    glViewport(0, 0, w, h);
  }
  
  virtual void GLPaint() {
		// Настройка освещения
		GLfloat light_position[] = { 15.0f, 15.0f, 15.0f, 0.0f };
		GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 0.1f };
		GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
		GLfloat light_specular[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		
		// Настройка материала
		GLfloat mat_ambient[] = { 0.2f, 0.4f, 0.7f, 1.0f };
		GLfloat mat_diffuse[] = { 0.3f, 0.6f, 0.9f, 1.0f };
		GLfloat mat_specular[] = { 0.8f, 0.8f, 0.8f, 0.8f };
		GLfloat mat_shininess[] = { 50.0f };
 
		Point3f bgColor = {(float)SColorFace().GetR() / 255.0f, (float)SColorFace().GetG() / 255.0f, (float)SColorFace().GetB() / 255.0f};
    glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
		// Включаем буфер глубины и освещение
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glShadeModel(GL_SMOOTH);
		
		// Устанавливаем источник света
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
		
		// Устанавливаем параметры материала
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		
		// Настройка проекции
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		Size sz = GetSize();
		gluPerspective(45.0, (double)sz.cx / sz.cy, 0.1, 2000.0);
		
		// Настройка вида
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, cameraCenter.x, cameraCenter.y, cameraCenter.z, 0., 0., 1.);
		
    glScalef(scale, scale, scale);
    
    for (Node3D* node : nodes) {
      glPushMatrix();
      node->GLPaint(false);
      glPopMatrix();
    }
  }
};

#endif