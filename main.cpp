#define GLM_SWIZZLE
#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>

#include <iostream>
#include <vector>
#include <corecrt_math_defines.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <array>
#include <locale>
#include "camera.h"
#include "lights.h"
#include "object.h"

using namespace std;

// Камера
Camera cam;

// Аттрибуты
GLint Phong_coord;
GLint Phong_texcoord;
GLint Phong_normal;
GLint Phong_mvp;
GLint Phong_viewPos;

// Источники света
Point_Light pl;
Dir_Light dl;
Spot_Light sl;
Material mat;

// Объекты
vector<Object> objects;
array<GLuint, 1> Programs;
array<GLuint, 8> textures;
vector<Object*> another_tanks;
vector<Object*> trees;
vector<Object*> rocks;
vector<Object*> barrels;
PlayerTank* play_tank;

// Пули
Material bullet_mat;
Object* bullet;
bool bullet_fired;
glm::vec3 bullet_dir;
const float bullet_speed = 1.0f;
const float bullet_max_dst = 50.0f;

void ShaderLog(unsigned int shader);
void InitShader();
void LoadAttrib(GLuint prog, GLint& attrib, const char* attr_name);
void LoadUniform(GLuint prog, GLint& attrib, const char* attr_name);
void LoadTexture(GLenum tex_enum, GLuint& tex, const char* path);
void InitVBO();
void InitTextures();
void Init();
void Draw(sf::Window& window);
void ReleaseShader();
void ReleaseVBO();
void Release();

int load_obj(const char* filename, vector<Vertex>& out)
{
	vector<glm::vec3> vertexes;
	vector<glm::vec3> normals;
	vector<glm::vec2> uvs;

	ifstream in(filename, ios::in);
	if (!in)
	{
		cerr << "Can't open obj " << filename << endl;
		return 0;
	}

	string line;
	while (getline(in, line))
	{
		string s = line.substr(0, 2);
		if (s == "v ")
		{
			istringstream s(line.substr(2));
			glm::vec3 v;
			s >> v.x;
			s >> v.y;
			s >> v.z;
			vertexes.push_back(v);
		}
		else if (s == "vt")
		{
			istringstream s(line.substr(3));
			glm::vec2 uv;
			s >> uv.x;
			s >> uv.y;
			uvs.push_back(uv);
		}
		else if (s == "vn")
		{
			istringstream s(line.substr(3));
			glm::vec3 n;
			s >> n.x;
			s >> n.y;
			s >> n.z;
			normals.push_back(n);
		}
		else if (s == "f ")
		{
			istringstream s(line.substr(2));
			string s1, s2, s3;
			s >> s1;
			s >> s2;
			s >> s3;
			unsigned int v1, v2, v3, uv1, uv2, uv3, n1, n2, n3;
			sscanf_s(s1.c_str(), "%d/%d/%d", &v1, &uv1, &n1);
			sscanf_s(s2.c_str(), "%d/%d/%d", &v2, &uv2, &n2);
			sscanf_s(s3.c_str(), "%d/%d/%d", &v3, &uv3, &n3);
			Vertex ve1 = { vertexes[v1 - 1].x, vertexes[v1 - 1].y, vertexes[v1 - 1].z, uvs[uv1 - 1].x, -uvs[uv1 - 1].y, normals[n1 - 1].x, normals[n1 - 1].y, normals[n1 - 1].z };
			Vertex ve2 = { vertexes[v2 - 1].x, vertexes[v2 - 1].y, vertexes[v2 - 1].z, uvs[uv2 - 1].x, -uvs[uv2 - 1].y, normals[n2 - 1].x, normals[n2 - 1].y, normals[n2 - 1].z };
			Vertex ve3 = { vertexes[v3 - 1].x, vertexes[v3 - 1].y, vertexes[v3 - 1].z, uvs[uv3 - 1].x, -uvs[uv3 - 1].y, normals[n3 - 1].x, normals[n3 - 1].y, normals[n3 - 1].z };
			out.push_back(ve1);
			out.push_back(ve2);
			out.push_back(ve3);
		}
	}
	return out.size();
}

const GLchar** load_shader(const char* path)
{
	ifstream file(path, ios::in);
	string src;

	while (file.good())
	{
		string line;
		getline(file, line);
		src.append(line + "\n");
	}
	;
	char* out = new char[src.length() + 1];
	strcpy_s(out, src.length() + 1, src.c_str());
	return (const GLchar**)&out;
}

void update_bullet()
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(bullet_dir.x * bullet_speed, 0.0f, bullet_dir.z * bullet_speed));
	for (int i = 0; i < bullet->size(); i++)
	{
		glm::vec4 v = glm::vec4(bullet->vertexes[i].x, bullet->vertexes[i].y, bullet->vertexes[i].z, 1.0f);
		v = model * v;
		bullet->vertexes[i].x = v.x;
		bullet->vertexes[i].y = v.y;
		bullet->vertexes[i].z = v.z;
		x += v.x;
		y += v.y;
		z += v.z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, bullet->id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * bullet->size(), &bullet->vertexes[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	checkOpenGLerror();
	bullet->center = glm::vec3(x, y, z) / (float)bullet->size();
	pl.pos = bullet->center;
}

void Init()
{
	pl.enabled = false;
	pl.pos = glm::vec3(-3.12f, 8.27f, -2.83f);
	pl.ambient = glm::vec3(0.1f);
	pl.diffuse = glm::vec3(0.8f);
	pl.specular = glm::vec3(0.8f);
	pl.atten = glm::vec3(0.5f);

	dl.direction = glm::vec3(0.0f, -1.0f, 0.0f);
	dl.ambient = glm::vec3(1.0f);
	dl.diffuse = glm::vec3(1.0f);
	dl.specular = glm::vec3(1.0f);

	// Для фар
	sl.enabled = false;
	sl.pos = glm::vec3(-5.0f, -8.37f, -5.0f);
	sl.direction = glm::vec3(1.0f);
	sl.ambient = glm::vec3(1.0f);
	sl.diffuse = glm::vec3(1.0f);
	sl.specular = glm::vec3(1.0f);
	sl.cutoff = 12.5f;
	sl.atten = glm::vec3(0.1f, 0.1f, 0.1f);

	// Материал
	mat.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.specular = glm::vec3(0.5f, 0.5f, 0.5f);
	mat.emission = glm::vec3(0.0f, 0.0f, 0.0f);
	mat.shininess = 1.0f;

	// Пули
	bullet_mat.ambient = glm::vec3(1.0f, 1.0f, 1.0f);
	bullet_mat.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
	bullet_mat.specular = glm::vec3(1.0f, 1.0f, 1.0f);
	bullet_mat.emission = glm::vec3(1.0f, 1.0f, 1.0f);
	bullet_mat.shininess = 1.0f;
	bullet_fired = false;

	glEnable(GL_DEPTH_TEST);
	InitShader();
	InitVBO();
	InitTextures();
}

int main()
{
	srand(time(NULL));
	sf::Window window(sf::VideoMode(1000, 1000), "World of Tanks", sf::Style::Default, sf::ContextSettings(24));
	window.setVerticalSyncEnabled(true); 
	window.setActive(true); 
	glewInit();
	Init();
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				Release();
				return 0;
			}
			else if (event.type == sf::Event::Resized)
			{
				glViewport(0, 0, event.size.width, event.size.height);
			}
			else if (event.type == sf::Event::KeyPressed)
			{
				// Двигаем танк
				if (event.key.code == sf::Keyboard::W)
				{
					cam.My_Forward(play_tank->dir, play_tank->speed);
					play_tank->Forward();
					sl.pos = play_tank->center;
					sl.direction = -play_tank->dir;
				}
				if (event.key.code == sf::Keyboard::S)
				{
					cam.My_Bacward(play_tank->dir, play_tank->speed);
					play_tank->Backward();
					sl.pos = play_tank->center;
					sl.direction = -play_tank->dir;
				}
				if (event.key.code == sf::Keyboard::D)
				{
					play_tank->Rotate_Right();
					cam.Rot_Right(play_tank->center, play_tank->dir);
					sl.pos = play_tank->center;
					sl.direction = -play_tank->dir;
				}
				if (event.key.code == sf::Keyboard::A)
				{
					play_tank->Rotate_Left();
					cam.Rot_Left(play_tank->center, play_tank->dir);
					sl.pos = play_tank->center;
					sl.direction = -play_tank->dir;
				}

				// Двигаем камеру
				if (event.key.code == sf::Keyboard::I)
				{
					cam.Cam_Forward();
				}
				if (event.key.code == sf::Keyboard::K)
				{
					cam.Cam_Backward();
				}
				if (event.key.code == sf::Keyboard::L)
				{
					cam.Cam_Right();
				}
				if (event.key.code == sf::Keyboard::J)
				{
					cam.Cam_Left();
				}

				// Фары
				if (event.key.code == sf::Keyboard::Q)
				{
					sl.enabled = !sl.enabled;
				}

				// Сбросить к изначальному
				if (event.key.code == sf::Keyboard::F10)
				{
					for (auto& t : another_tanks)
						t->hit = false;

					for (auto& b : barrels)
						b->hit = false;

					for (auto& t : trees)
						t->hit = false;

					for (auto& r : rocks)
						r->hit = false;
				}
			}

			// Пуля
			if (event.key.code == sf::Keyboard::E)
			{
				if (!bullet_fired)
				{
					bullet = objects[6].copy();
					pl.enabled = true;
					pl.pos = play_tank->center;
					bullet_fired = true;
					bullet->dx = play_tank->center.x;
					bullet->dz = play_tank->center.z;
					bullet_dir = -play_tank->dir;
					bullet->Update();
				}
			}
		}
		
		if (bullet_fired)
		{
			if (glm::distance(bullet->center, play_tank->center) > bullet_max_dst)
			{
				bullet_fired = false;
				pl.enabled = false;
				bullet->destroy();
				delete bullet;
			}
			else
			{
				update_bullet();
				
				// проверяем, столкнулась ли пули с чем-то из объектов
				for (auto& t : another_tanks)
				{
					if (!t->hit)
					{
						if (t->HitBullet(bullet->center))
						{
							t->hit = true;
							bullet_fired = false;
							pl.enabled = false;
							bullet->destroy();
							delete bullet;
							break;
						}
					}
				}

				for (auto& b : barrels)
				{
					if (!b->hit)
					{
						if (b->HitBullet(bullet->center))
						{
							b->hit = true;
							bullet_fired = false;
							pl.enabled = false;
							bullet->destroy();
							delete bullet;
							break;
						}
					}
				}
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Draw(window);
		window.display();
	}
}

void LoadObject(int i, const char* path)
{
	GLuint id;
	glGenBuffers(1, &id);
	vector<Vertex> data;
	load_obj(path, data);
	Object obj(id, data);
	objects.push_back(obj);
	checkOpenGLerror();
}

// Пересечение с каким-то обьектом сцены
bool intersects(Object* obj)
{
	const float R = 3.0f;
	obj->Update();
	
	if (glm::distance(obj->center, play_tank->center) < R)
	{
		return true;
	}

	if (glm::distance(obj->center, objects[2].center) < R)
	{
		return true;
	}
	
	for (auto& t : another_tanks)
	{
		if (glm::distance(obj->center, t->center) < R)
			return true;
	}

	for (auto& b : barrels)
	{
		if (glm::distance(obj->center,b->center) < R)
			return true;
	}

	for (auto& t : trees)
	{
		if (glm::distance(obj->center, t->center) < R)
			return true;
	}

	for (auto& r : rocks)
	{
		if (glm::distance(obj->center, r->center) < R)
			return true;
	}

	return false;
}

void InitVBO()
{	
	//Загрузка объектов
	LoadObject(0, "models/Field.obj");
	LoadObject(1, "models/Tank.obj");
	LoadObject(2, "models/ChrTree.obj");
	LoadObject(3, "models/Barrel.obj");
	LoadObject(4, "models/Tree.obj");
	LoadObject(5, "models/Stone.obj");
	LoadObject(6, "models/Bullet.obj");
	LoadObject(7, "models/Teapot.obj");

	// Танк
	play_tank = new PlayerTank(objects[1]);
	sl.pos = play_tank->center;
	sl.direction = -play_tank->dir;

	// Ёлочка
	objects[2].dx = 10;
	objects[2].dz = 10;
	for (auto& obj : objects)
		obj.Update();

	// Еще танки, больше танков
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[1].copy();
		do 
		{
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->dy = rand() % 360;
		} while (intersects(o));
		another_tanks.push_back(o);
	}

	// Бочки
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[3].copy();
		do 
		{
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->dy = rand() % 360;
		} while (intersects(o));
		barrels.push_back(o);
	}

	// Деревья
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[4].copy();
		do 
		{
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->dy = rand() % 360;
		} while (intersects(o));
		trees.push_back(o);
	}

	// Глыбы и камни
	for (int i = 0; i < 3; i++)
	{
		Object* o = objects[5].copy();
		do
		{
			o->dx = rand() % 16;
			o->dz = rand() % 16;
			o->dy = rand() % 360;
		} while (intersects(o));
		rocks.push_back(o);
	}
}

void InitTextures()
{
	LoadTexture(GL_TEXTURE0, textures[0], "textures/Field.png");
	LoadTexture(GL_TEXTURE1, textures[1], "textures/Tank.png");
	LoadTexture(GL_TEXTURE2, textures[2], "textures/ChrTree.png");
	LoadTexture(GL_TEXTURE3, textures[3], "textures/Barrel.png");
	LoadTexture(GL_TEXTURE4, textures[4], "textures/Tree.png");
	LoadTexture(GL_TEXTURE5, textures[5], "textures/Stone.png");
	LoadTexture(GL_TEXTURE8, textures[6], "textures/bullet.jpg");
	LoadTexture(GL_TEXTURE6, textures[7], "textures/ruby.jpg");
}

void LoadAttrib(GLuint prog, GLint& attrib, const char* attr_name)
{
	attrib = glGetAttribLocation(prog, attr_name);
	if (attrib == -1)
	{
		std::cout << "could not bind attrib " << attr_name << std::endl;
		return;
	}
}

void LoadUniform(GLuint prog, GLint& attrib, const char* attr_name)
{
	attrib = glGetUniformLocation(prog, attr_name);
	if (attrib == -1)
	{
		std::cout << "could not bind uniform " << attr_name << std::endl;
		return;
	}
}

void LoadTexture(GLenum tex_enum, GLuint& tex, const char* path)
{
	glGenTextures(1, &tex);
	glActiveTexture(tex_enum);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	sf::Image img;
	if (!img.loadFromFile(path))
	{
		std::cout << "could not load texture " << path << std::endl;
		return;
	}

	sf::Vector2u size = img.getSize();
	int width = size.x;
	int height = size.y;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
	glGenerateMipmap(GL_TEXTURE_2D);
}

void InitShader()
{
	// Фонга
	GLuint PhongVShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(PhongVShader, 1, load_shader("shaders/phong.vert"), NULL);
	glCompileShader(PhongVShader);
	std::cout << "phong vertex shader \n";
	ShaderLog(PhongVShader);

	GLuint PhongFShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(PhongFShader, 1, load_shader("shaders/phong.frag"), NULL);
	glCompileShader(PhongFShader);
	std::cout << "phong fragment shader \n";
	ShaderLog(PhongFShader);

	Programs[0] = glCreateProgram();

	glAttachShader(Programs[0], PhongVShader);
	glAttachShader(Programs[0], PhongFShader);

	glLinkProgram(Programs[0]);

	int link1;
	glGetProgramiv(Programs[0], GL_LINK_STATUS, &link1);
	if (!link1)
	{
		std::cout << "could not link shader program" << std::endl;
		return;
	}

	LoadAttrib(Programs[0], Phong_coord, "coord");
	LoadAttrib(Programs[0], Phong_texcoord, "texcoord");
	LoadAttrib(Programs[0], Phong_normal, "normal");
	LoadUniform(Programs[0], Phong_mvp, "mvp");
	LoadUniform(Programs[0], Phong_viewPos, "viewPos");

	checkOpenGLerror();
}

void Draw(sf::Window& window)
{
	GLuint tex_loc;

	// Сцена
	glUseProgram(Programs[0]);
	tex_loc = glGetUniformLocation(Programs[0], "tex");
	pl.Load(Programs[0]);
	dl.Load(Programs[0]);
	sl.Load(Programs[0]);
	mat.Load(Programs[0]);
	glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
	glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
	glUniform1i(tex_loc, 0);
	glEnableVertexAttribArray(Phong_coord);
	glEnableVertexAttribArray(Phong_texcoord);
	glEnableVertexAttribArray(Phong_normal);
	glBindBuffer(GL_ARRAY_BUFFER, objects[0].id);
	glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, objects[0].size());
	glDisableVertexAttribArray(Phong_coord);
	glDisableVertexAttribArray(Phong_texcoord);
	glDisableVertexAttribArray(Phong_normal);
	glUseProgram(0);

	// Танк
	glUseProgram(Programs[0]);
	tex_loc = glGetUniformLocation(Programs[0], "tex");
	pl.Load(Programs[0]);
	dl.Load(Programs[0]);
	sl.Load(Programs[0]);
	mat.Load(Programs[0]);
	glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
	glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
	glUniform1i(tex_loc, 1);
	glEnableVertexAttribArray(Phong_coord);
	glEnableVertexAttribArray(Phong_texcoord);
	glEnableVertexAttribArray(Phong_normal);
	glBindBuffer(GL_ARRAY_BUFFER, play_tank->id);
	glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, play_tank->size());
	glDisableVertexAttribArray(Phong_coord);
	glDisableVertexAttribArray(Phong_texcoord);
	glDisableVertexAttribArray(Phong_normal);
	glUseProgram(0); 

	// Пули
	if (bullet_fired)
	{
		glUseProgram(Programs[0]);
		tex_loc = glGetUniformLocation(Programs[0], "tex");
		pl.Load(Programs[0]);
		dl.Load(Programs[0]);
		sl.Load(Programs[0]);
		bullet_mat.Load(Programs[0]);
		glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
		glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
		glUniform1i(tex_loc, 8);
		glEnableVertexAttribArray(Phong_coord);
		glEnableVertexAttribArray(Phong_texcoord);
		glEnableVertexAttribArray(Phong_normal);
		glBindBuffer(GL_ARRAY_BUFFER, bullet->id);
		glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, bullet->size());
		glDisableVertexAttribArray(Phong_coord);
		glDisableVertexAttribArray(Phong_texcoord);
		glDisableVertexAttribArray(Phong_normal);
		glUseProgram(0);
	}

	// Ёлочка
	glUseProgram(Programs[0]);
	tex_loc = glGetUniformLocation(Programs[0], "tex");
	pl.Load(Programs[0]);
	dl.Load(Programs[0]);
	sl.Load(Programs[0]);
	mat.Load(Programs[0]);
	glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
	glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
	glUniform1i(tex_loc, 2);
	glEnableVertexAttribArray(Phong_coord);
	glEnableVertexAttribArray(Phong_texcoord);
	glEnableVertexAttribArray(Phong_normal);
	glBindBuffer(GL_ARRAY_BUFFER, objects[2].id);
	glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, objects[2].size());
	glDisableVertexAttribArray(Phong_coord);
	glDisableVertexAttribArray(Phong_texcoord);
	glDisableVertexAttribArray(Phong_normal);
	glUseProgram(0);

	// Танки
	for (int i = 0; i < another_tanks.size(); i++)
	{
		if (!another_tanks[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 6);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, another_tanks[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, another_tanks[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0);
		}
	}

	// Бочки
	for (int i = 0; i < barrels.size(); i++)
	{
		if (!barrels[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 3);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, barrels[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, barrels[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0);
		}
	}

	// Деревья
	for (int i = 0; i < trees.size(); i++)
	{
		if (!trees[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 4);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, trees[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, trees[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0);
		}
	}

	// Глыбы и камни
	for (int i = 0; i < rocks.size(); i++)
	{
		if (!rocks[i]->hit)
		{
			glUseProgram(Programs[0]);
			tex_loc = glGetUniformLocation(Programs[0], "tex");
			pl.Load(Programs[0]);
			dl.Load(Programs[0]);
			sl.Load(Programs[0]);
			mat.Load(Programs[0]);
			glUniformMatrix4fv(Phong_mvp, 1, GL_FALSE, glm::value_ptr(cam.View_Point()));
			glUniform3fv(Phong_viewPos, 1, glm::value_ptr(cam.Pos));
			glUniform1i(tex_loc, 5);
			glEnableVertexAttribArray(Phong_coord);
			glEnableVertexAttribArray(Phong_texcoord);
			glEnableVertexAttribArray(Phong_normal);
			glBindBuffer(GL_ARRAY_BUFFER, rocks[i]->id);
			glVertexAttribPointer(Phong_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
			glVertexAttribPointer(Phong_texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			glVertexAttribPointer(Phong_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, rocks[i]->size());
			glDisableVertexAttribArray(Phong_coord);
			glDisableVertexAttribArray(Phong_texcoord);
			glDisableVertexAttribArray(Phong_normal);
			glUseProgram(0);
		}
	}
	checkOpenGLerror();
}

void Release()
{
	ReleaseShader();
	ReleaseVBO(); 
}

void ReleaseVBO()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	for (int i = 0; i < objects.size(); i++)
	{
		glDeleteBuffers(1, &objects[i].id);
	}
	for (int i = 0; i < barrels.size(); i++)
	{
		barrels[i]->destroy();
		delete barrels[i];
	}
	for (int i = 0; i < trees.size(); i++)
	{
		trees[i]->destroy();
		delete trees[i];
	}
	for (int i = 0; i < rocks.size(); i++)
	{
		rocks[i]->destroy();
		delete rocks[i];
	}
}

void ReleaseShader()
{
	glUseProgram(0);
	for (int i = 0; i < Programs.size(); i++)
	{
		glDeleteProgram(Programs[i]);
	}
}

void ShaderLog(unsigned int shader)
{
	int infologLen = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
	if (infologLen > 1)
	{
		int charsWritten = 0;
		std::vector<char> infoLog(infologLen);
		glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog.data());
		std::cout << "InfoLog: " << infoLog.data() << std::endl;
		exit(1);
	}
}

void checkOpenGLerror()
{
	GLenum errCode;
	const GLubyte* errString;
	if ((errCode = glGetError()) != GL_NO_ERROR)
	{
		errString = gluErrorString(errCode);
		std::cout << "OpenGL error: (" << errCode << ") " << errString << std::endl;
	}
}