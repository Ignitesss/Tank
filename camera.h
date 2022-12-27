#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
private:

	float cam_pitch = 0.0f;
	float cam_yaw = -90.0f;
	const glm::vec3 cam_offset = glm::vec3(0.0f, 3.0f, 6.0f);

public:

	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Pos;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;

	float move_speed = 0.5f;
	float rot_speed = 2.0f;

	Camera()
	{
		Pos = cam_offset;
		Front = glm::vec3(0.0f, 0.0f, -1.0f);
		Up = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(Pos, Pos + Front, Up);
		proj = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
		model = glm::mat4(1.0f);
		cam_yaw = -90.0f;
		cam_pitch = -20.0f;
		Update();
	}
	
	void Cam_Forward()
	{
		Pos += move_speed * Front;
	}
	
	void Cam_Backward()
	{
		Pos -= move_speed * Front;
	}
	
	void Cam_Left()
	{
		Pos -= glm::normalize(glm::cross(Front, Up)) * move_speed;
	}
	
	void Cam_Right()
	{
		Pos += glm::normalize(glm::cross(Front, Up)) * move_speed;
	}

	void Update()
	{
		glm::vec3 newFront;
		newFront.x = cos(glm::radians(cam_yaw)) * cos(glm::radians(cam_pitch));
		newFront.y = sin(glm::radians(cam_pitch));
		newFront.z = sin(glm::radians(cam_yaw)) * cos(glm::radians(cam_pitch));
		Front = glm::normalize(newFront);
	}

	void My_Forward(const glm::vec3& td, const float ts)
	{
		Pos.x += ts * -td.x;
		Pos.y = cam_offset.y;
		Pos.z += ts * -td.z;
	}

	void My_Bacward(const glm::vec3& td, const float ts)
	{
		Pos.x += ts * td.x;
		Pos.y = cam_offset.y;
		Pos.z += ts * td.z;
	}

	void Rot_Right(const glm::vec3 tc, const glm::vec3& td)
	{
		Front.x = -td.x;
		Front.z = -td.z;
		Pos.x = tc.x;
		Pos.z = tc.z;
		Pos -= glm::length(cam_offset.xz()) * Front;
		Pos.y = cam_offset.y;
	}

	void Rot_Left(const glm::vec3 tc, const glm::vec3& td)
	{
		Front.x = -td.x;
		Front.z = -td.z;
		Pos.x = tc.x;
		Pos.z = tc.z;
		Pos -= glm::length(cam_offset.xz()) * Front;
		Pos.y = cam_offset.y;
	}

	void Yaw_Plus()
	{
		cam_yaw += rot_speed;
		Update();
	}
	
	void Yaw_Minus()
	{
		cam_yaw -= rot_speed;
		Update();
	}

	void Pitch_Plus()
	{
		cam_pitch += rot_speed;
		Update();
	}
	
	void Pitch_Minus()
	{
		cam_pitch -= rot_speed;
		Update();
	}

	void Perspective()
	{
		proj = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);	
	}
	
	glm::mat4 View_Point()
	{
		view = glm::lookAt(Pos, Pos + Front, Up);
		return proj * view * model;
	}

	glm::mat4 Model()
	{
		return model;
	}
	
	glm::mat4 View()
	{
		view = glm::lookAt(Pos, Pos + Front, Up);
		return view;
	}
	
	glm::mat4 Proj()
	{
		return proj;
	}
};