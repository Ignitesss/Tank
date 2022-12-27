#pragma once
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex
{
	// Координаты
	GLfloat x;
	GLfloat y;
	GLfloat z;

	// Координаты текстур
	GLfloat s;
	GLfloat t;

	// Нормали
	GLfloat nx;
	GLfloat ny;
	GLfloat nz;
};

ostream& operator<<(ostream& os, const glm::vec3 v)
{
	return os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

void checkOpenGLerror();

class Object
{
protected:

	// радиус действия пули
	const float bullet_rad = 1.0f;

	void ApplyTransform()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(dy), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(dx, 0.0f, dz));
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertexes[i].x, vertexes[i].y, vertexes[i].z, 1.0f);
			v = model * v;
			vertexes[i].x = v.x;
			vertexes[i].y = v.y;
			vertexes[i].z = v.z;
			
			x += vertexes[i].x;
			y += vertexes[i].y;
			z += vertexes[i].z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertexes[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
		center = glm::vec3(x, y, z) / (float)size();
	}

public:

	float dx;
	float dz;
	float dy;

	bool hit;
	GLuint id;
	glm::vec3 center;
	vector<Vertex> vertexes;

	Object(GLuint id, const vector<Vertex>& vertexes, float dx = 0.0f, float dz = 0.0f, float dy = 0.0f)
	{
		this->id = id;
		this->dx = dx;
		this->dz = dz;
		this->dy = dy;
		hit = false;
		this->vertexes = vertexes;
		ApplyTransform();
	}

	size_t size()
	{
		return vertexes.size();
	}

	Object* copy()
	{
		GLuint new_id;
		glGenBuffers(1, &new_id);
		return new Object(new_id, vertexes, dx, dz, dy);
	}

	void Update()
	{
		ApplyTransform();
	}

	bool HitBullet(const glm::vec3& bulletPos)
	{
		if (glm::distance(center, bulletPos) < bullet_rad)
			return true;
		return false;
	}
	
	void destroy()
	{
		glDeleteBuffers(1, &id);
	}
};


class PlayerTank
{
private:

	void setCenter()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		for (int i = 0; i < size(); i++)
		{
			x += vertexes[i].x;
			y += vertexes[i].y;
			z += vertexes[i].z;
		}
		center = glm::vec3(x, y, z) / (float)size();
	}

public:

	float dx;
	float dz;
	float dy;
	GLuint id;
	glm::vec4 dir;
	glm::vec3 center;
	vector<Vertex> vertexes;
	const float speed = 0.2f;
	const float rot_speed = 2.0f;
	
	PlayerTank(Object& obj)
	{
		dx = 0.0f;
		dz = 0.0f;
		dy = 0.0f;
		id = obj.id;
		vertexes = obj.vertexes;
		dir = { 0.0f, 0.0f, 1.0f, 0.0f};
		setCenter();
	}

	size_t size()
	{
		return vertexes.size();
	}

	void Move()
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(speed * dir.x * dz, 0.0f, speed * dir.z * dz));
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertexes[i].x, vertexes[i].y, vertexes[i].z, 1.0f);
			v = model * v;
			vertexes[i].x = v.x;
			vertexes[i].y = v.y;
			vertexes[i].z = v.z;

			x += vertexes[i].x;
			y += vertexes[i].y;
			z += vertexes[i].z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertexes[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
		center = glm::vec3(x, y, z) / (float)size();
	}

	void Rotate()
	{
		glm::vec3 pos = center;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(pos.x, 0.0f, pos.z));
		model = glm::rotate(model, glm::radians(dy), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-pos.x, 0.0f, -pos.z));
		dir = model * dir;
		for (int i = 0; i < size(); i++)
		{
			glm::vec4 v = glm::vec4(vertexes[i].x, vertexes[i].y, vertexes[i].z, 1.0f);
			v = model * v;
			vertexes[i].x = v.x;
			vertexes[i].y = v.y;
			vertexes[i].z = v.z;
		}
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * size(), &vertexes[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkOpenGLerror();
	}

	void Forward()
	{
		dz = -1.0f;
		Move();
	}

	void Backward()
	{
		dz = 1.0f;
		Move();
	}

	void Rotate_Left()
	{
		dy = rot_speed;
		Rotate();
	}

	void Rotate_Right()
	{
		dy = -rot_speed;
		Rotate();
	}
};
