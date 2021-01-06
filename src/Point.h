#pragma once
#include"glm/glm.hpp"
#include<string>
#include<iostream>
class Point {
public:
	glm::vec3 position;
	Point() {
		position.x = position.y = position.z = 0.0;
	}

	Point(float x, float y, float z){
		position.x = x;
		position.y = y;
		position.z = z;
	}

	Point(glm::vec3 pos) {
		this->position = pos;
	}
	
	Point(glm::vec2 pos) {
		this->position = glm::vec3(pos.x, pos.y, 0.0);
	}

	void debug(std::string name="") {
		std::cout << name << "pos:(" << position.x << "," << position.y << "," << position.z << ")" << std::endl;
	}
private:
};