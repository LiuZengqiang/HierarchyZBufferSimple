#pragma once
#include"glm\glm.hpp"
#include<iostream>
#include<vector>

struct Surface
{
	std::vector<int > indices;
	std::vector<int > nor_indices;
	
	glm::vec3 normal_ = {0.0f, 0.0f, 0.0f};

	Surface() {
		indices.clear();
		nor_indices.clear();
	}
	void debug() {

		std::cout << "size:" << indices.size() << std::endl;
		for (unsigned int i = 0; i < indices.size(); i++) {
			std::cout << indices[i] << " ";
		}
		std::cout << std::endl;
	}
};