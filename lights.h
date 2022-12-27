#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <string>
#include <iostream>

using namespace std;


class Material
{
public: 

	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
	glm::vec3 emission;
	float shininess;

	void Load(GLuint program)
	{
		std::string prefix = "material.";
		glUniform3fv(glGetUniformLocation(program, (prefix + "ambient").c_str()), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, (prefix + "diffuse").c_str()), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, (prefix + "specular").c_str()), 1, glm::value_ptr(specular));
		glUniform3fv(glGetUniformLocation(program, (prefix + "emission").c_str()), 1, glm::value_ptr(emission));
		glUniform1f(glGetUniformLocation(program, (prefix + "shininess").c_str()), shininess);
	}
};

class Point_Light
{
public:

	bool enabled;
	glm::vec3 pos;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
	glm::vec3 atten;

	void Load(GLuint program)
	{
		std::string prefix = "pointl.";
		glUniform1i(glGetUniformLocation(program, (prefix + "enabled").c_str()), enabled);
		glUniform3fv(glGetUniformLocation(program, (prefix + "pos").c_str()), 1, glm::value_ptr(pos));
		glUniform3fv(glGetUniformLocation(program, (prefix + "ambient").c_str()), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, (prefix + "diffuse").c_str()), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, (prefix + "specular").c_str()), 1, glm::value_ptr(specular));
		glUniform3fv(glGetUniformLocation(program, (prefix + "atten").c_str()), 1, glm::value_ptr(atten));
	}
	void Config()
	{
		cout << "Point Light Config" << endl;
		cout << "Position: (cur.: " << pos.x << ", " << pos.y << ", " << pos.z << ")" << endl;
		cin >> pos.x >> pos.y >> pos.z;
		cout << "Ambient: (cur.: " << ambient.x << ", " << ambient.y << ", " << ambient.z << ")" << endl;
		cin >> ambient.x >> ambient.y >> ambient.z;
		cout << "Diffuse: (cur.: " << diffuse.x << ", " << diffuse.y << ", " << diffuse.z << ")" << endl;
		cin >> diffuse.x >> diffuse.y >> diffuse.z;
		cout << "Specular: (cur.: " << specular.x << ", " << specular.y << ", " << specular.z << ")" << endl;
		cin >> specular.x >> specular.y >> specular.z;
		cout << "Attenuation: (cur.: " << atten.x << ", " << atten.y << ", " << atten.z << ")" << endl;
		cin >> atten.x >> atten.y >> atten.z;
	}
};

class Dir_Light
{
public:

	glm::vec3 direction;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;

	void Load(GLuint program)
	{
		std::string prefix = "dirl.";
		glUniform3fv(glGetUniformLocation(program, (prefix + "direction").c_str()), 1, glm::value_ptr(direction));
		glUniform3fv(glGetUniformLocation(program, (prefix + "ambient").c_str()), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, (prefix + "diffuse").c_str()), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, (prefix + "specular").c_str()), 1, glm::value_ptr(specular));
	}

	void Config()
	{
		cout << "Directional Light Config" << endl;
		cout << "Direction: (cur.: " << direction.x << ", " << direction.y << ", " << direction.z << ")" << endl;
		cin >> direction.x >> direction.y >> direction.z;
		cout << "Ambient: (cur.: " << ambient.x << ", " << ambient.y << ", " << ambient.z << ")" << endl;
		cin >> ambient.x >> ambient.y >> ambient.z;
		cout << "Diffuse: (cur.: " << diffuse.x << ", " << diffuse.y << ", " << diffuse.z << ")" << endl;
		cin >> diffuse.x >> diffuse.y >> diffuse.z;
		cout << "Specular: (cur.: " << specular.x << ", " << specular.y << ", " << specular.z << ")" << endl;
		cin >> specular.x >> specular.y >> specular.z;
	}
};

class Spot_Light
{
public:

	bool enabled;
	glm::vec3 pos;
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float cutoff;
	glm::vec3 atten;

	void Load(GLuint program)
	{
		std::string prefix = "spotl.";
		glUniform1i(glGetUniformLocation(program, (prefix + "enabled").c_str()), enabled);
		glUniform3fv(glGetUniformLocation(program, (prefix + "pos").c_str()), 1, glm::value_ptr(pos));
		glUniform3fv(glGetUniformLocation(program, (prefix + "direction").c_str()), 1, glm::value_ptr(direction));
		glUniform3fv(glGetUniformLocation(program, (prefix + "ambient").c_str()), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, (prefix + "diffuse").c_str()), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, (prefix + "specular").c_str()), 1, glm::value_ptr(specular));
		glUniform1f(glGetUniformLocation(program, (prefix + "cutoff").c_str()), cutoff);
		glUniform3fv(glGetUniformLocation(program, (prefix + "atten").c_str()), 1, glm::value_ptr(atten));
	}

	void Config()
	{
		cout << "Spot Light Config" << endl;
		cout << "Position: (cur.: " << pos.x << ", " << pos.y << ", " << pos.z << ")" << endl;
		cin >> pos.x >> pos.y >> pos.z;
		cout << "Direction: (cur.: " << direction.x << ", " << direction.y << ", " << direction.z << ")" << endl;
		cin >> direction.x >> direction.y >> direction.z;
		cout << "Ambient: (cur.: " << ambient.x << ", " << ambient.y << ", " << ambient.z << ")" << endl;
		cin >> ambient.x >> ambient.y >> ambient.z;
		cout << "Diffuse: (cur.: " << diffuse.x << ", " << diffuse.y << ", " << diffuse.z << ")" << endl;
		cin >> diffuse.x >> diffuse.y >> diffuse.z;
		cout << "Specular: (cur.: " << specular.x << ", " << specular.y << ", " << specular.z << ")" << endl;
		cin >> specular.x >> specular.y >> specular.z;
		cout << "Cutoff: (cur.: " << cutoff << ")" << endl;
		cin >> cutoff;
		cout << "Attenuation: (cur.: " << atten.x << ", " << atten.y << ", " << atten.z << ")" << endl;
		cin >> atten.x >> atten.y >> atten.z;
	}
};