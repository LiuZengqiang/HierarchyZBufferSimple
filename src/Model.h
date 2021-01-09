#pragma once
#include"Point.h"
#include"Surface.h"
#include"tool\globalFunction.cpp"
#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>
#include<float.h>
#include<ctime>
class Model
{
public:
	Model() {
	};

	Model(std::string path) {
		path_ = path;
		loadModel();
	};

	~Model() {};

	void setModelPath(std::string path) {
		path_ = path;
	}
	void setCameraPos(glm::vec3 camera_pos) {
		camera_pos_ = glm::normalize(camera_pos);

	};
	void setLightPos(glm::vec3 light_pos) {
		light_pos_ = glm::normalize(light_pos);
	};

	/* init model:init camera and light position,load model, scale model */
	void init() {

		initCameraLightPos();

		loadModel();

		scaleModel();
	}

	/* get surface index min and max X */
	std::pair<float, float> getMinMaxX(unsigned int index) {
		if (index < 0 || index >= sur_faces_.size()) {
			std::cout << "ERROR::Model::getSurfaceMaxZ the index:" << index << " is out of range which is should in [0," << sur_faces_.size() - 1 << ")." << std::endl;
			return std::pair<float, float>(0.0, 0.0);
		}
		float ret_max_x = -FLT_MAX;
		float ret_min_x = FLT_MAX;
		int size = (int)sur_faces_[index].indices.size();

#pragma omp parallel for
		for (int i = 0; i < size; i++) {
#pragma omp critical
			{
				ret_max_x = std::max(ret_max_x, points_[sur_faces_[index].indices[i]].position.x);
				ret_min_x = std::min(ret_min_x, points_[sur_faces_[index].indices[i]].position.x);
			}
		}

		return std::pair<float, float>(ret_min_x, ret_max_x);
	}
	/* get surface index min and max Y */
	std::pair<float, float> getMinMaxY(unsigned int index) {
		if (index < 0 || index >= sur_faces_.size()) {
			std::cout << "ERROR::Model::getSurfaceMaxZ the index:" << index << " is out of range which is should in [0," << sur_faces_.size() - 1 << ")." << std::endl;
			return std::pair<float, float>(0.0, 0.0);
		}
		float ret_max_y = -FLT_MAX;
		float ret_min_y = FLT_MAX;
		int size = (int)sur_faces_[index].indices.size();
#pragma omp parallel for
		for (int i = 0; i < size; i++) {
#pragma omp critical
			{
				ret_max_y = std::max(ret_max_y, points_[sur_faces_[index].indices[i]].position.y);
				ret_min_y = std::min(ret_min_y, points_[sur_faces_[index].indices[i]].position.y);
			}
		}
		return std::pair<float, float>(ret_min_y, ret_max_y);

	}
	/* get surface index min and max Z */
	std::pair<float, float> getMinMaxZ(unsigned int index) {
		if (index < 0 || index >= sur_faces_.size()) {
			std::cout << "ERROR::Model::getSurfaceMaxZ the index:" << index << " is out of range which is should in [0," << sur_faces_.size() - 1 << ")." << std::endl;
			return std::pair<float, float>(0.0, 0.0);
		}
		float ret_max_z = -FLT_MAX;
		float ret_min_z = FLT_MAX;
		int size = (int)sur_faces_[index].indices.size();
#pragma omp parallel for		
		for (int i = 0; i < size; i++) {
#pragma omp critical			
			{
				ret_max_z = std::max(ret_max_z, points_[sur_faces_[index].indices[i]].position.z);
				ret_min_z = std::min(ret_min_z, points_[sur_faces_[index].indices[i]].position.z);
			}
		}
		return std::pair<float, float>(ret_min_z, ret_max_z);
	}
	/* get model min and max Z */
	std::pair<float, float> getModelMinMaxZ() {
		if (max_z_ == FLT_MAX || min_z_ == -FLT_MAX) {
			initMinMaxXYZ();
		}
		return std::pair<float, float>(min_z_, max_z_);
	};
	/* get surface index max Z */
	float getMaxZ(unsigned int index) {
		if (index < 0 || index >= sur_faces_.size()) {
			std::cout << "ERROR::Model::getSurfaceMaxZ the index:" << index << " is out of range which is should in [0," << sur_faces_.size() - 1 << ")." << std::endl;
			return 0.0;
		}

		float ret = -FLT_MAX;

		for (unsigned int i = 0; i < sur_faces_[index].indices.size(); i++) {

			ret = std::max(ret, points_[sur_faces_[index].indices[i]].position.z);
		}
		return ret;
	}
	/* get number of all surfaces */
	int getSurfacesNum() {
		return num_surfaces_;
	}

	std::vector<Point > points_;
	std::vector<Surface > sur_faces_;
	std::vector<glm::vec3 > normals_;

	std::vector<float> color_;

	float max_x_ = -FLT_MAX;
	float max_y_ = -FLT_MAX;
	float max_z_ = -FLT_MAX;
	float min_x_ = FLT_MAX;
	float min_y_ = FLT_MAX;
	float min_z_ = FLT_MAX;

private:
	void initCameraLightPos() {
		std::cout << "HINT::Model initial camera and light position." << std::endl;
		cos_alpha_ = glm::dot(glm::vec3(0.0, 0.0, 1.0), glm::normalize(glm::vec3(camera_pos_.x, 0.0, camera_pos_.z)));
		sin_alpha_ = std::sqrt(1.0 - cos_alpha_ * cos_alpha_ < 0.0 ? 0.0 : 1.0 - cos_alpha_ * cos_alpha_) * (camera_pos_.x >= 0 ? -1.0 : 1.0);

		cos_beta_ = glm::dot(glm::normalize(glm::vec3(camera_pos_.x, 0.0, camera_pos_.z)), camera_pos_);
		sin_beta_ = std::sqrt(1.0 - cos_beta_ * cos_beta_ < 0.0 ? 0.0 : 1.0 - cos_beta_ * cos_beta_) * (camera_pos_.y >= 0 ? -1.0 : 1.0);

		// 旋转light_pos
		float z = light_pos_.z;
		float x = light_pos_.x;
		light_pos_.z = (float)(cos_alpha_ * z + sin_alpha_ * x * -1.0f);
		light_pos_.x = (float)(sin_alpha_ * z + cos_alpha_ * x);
		z = light_pos_.z;
		float y = light_pos_.y;
		light_pos_.z = (float)(cos_beta_ * z + sin_beta_ * y * -1.0f);
		light_pos_.y = (float)(sin_beta_ * z + cos_beta_ * y);
	};

	void loadModel() {

		std::cout << "HINT::Model load model." << std::endl;
		clock_t t = clock();

		points_.clear();
		sur_faces_.clear();
		normals_.clear();

		std::ifstream model_file;
		model_file.open(path_);
		if (!model_file.good()) {
			std::cout << "ERROR::Importer::loadModel open file:\'" << path_ << "\' failed." << std::endl;
			return;
		}
		else {
			std::string line_data;
			int line_id = 0;
			while (std::getline(model_file, line_data)) {

				line_id++;
				unsigned int index = 0;
				// remove ' '
				std::string next_word = getNextStr(line_data, index);

				if (next_word == "") {
					continue;
				}

				if (next_word[0] == '#') {	// pass comment
					continue;
				}
				else if (next_word == "v") {	// v
					Point p;

					std::vector<float> pos;

					while ((next_word = getNextStr(line_data, index)) != "") {

						float val = (float)std::atof(next_word.c_str());
						pos.push_back(val);
					}
					if (pos.size() != 3) {
						std::cout << "ERROR::Importer loadModel. The point positions parameter is error. Get " << pos.size() << " parameters." << std::endl;
						return;
					}

					p.position.x = (float)(sin_alpha_ * pos[2] + cos_alpha_ * pos[0]);
					p.position.z = (float)(cos_alpha_ * pos[2] + sin_alpha_ * pos[0] * (-1.0));
					pos[2] = p.position.z;

					p.position.z = (float)(cos_beta_ * pos[2] + sin_beta_ * pos[1] * (-1.0));
					p.position.y = (float)(sin_beta_ * pos[2] + cos_beta_ * pos[1]);

					if (global::floatEqual(p.position.x, 0.0f)) {
						p.position.x = 0.0f;
					}
					if (global::floatEqual(p.position.y, 0.0f)) {
						p.position.y = 0.0f;
					}
					if (global::floatEqual(p.position.z, 0.0f)) {
						p.position.z = 0.0f;
					}
					points_.push_back(p);
				}
				else if (next_word == "f") {	// f

					num_surfaces_++;

					Surface f;
					// position indices
					std::vector<int> indices;

					// normal indices
					std::vector<int> nor_indices;

					while ((next_word = getNextStr(line_data, index)) != "") {

						int j = 0;
						while (j < next_word.length() && next_word[j] != '/') {
							j++;
						}
						int index = std::atoi(next_word.substr(0, j).c_str());
						indices.push_back(index);

						j++;
						// 提取normal
						if (j >= next_word.length()) {
							continue;
						}
						//
						while (j < next_word.length() && next_word[j] != '/') {
							j++;
						}
						j++;
						if (j >= next_word.length()) {
							continue;
						}
						// 此时nex_w[j-1]=='/'
						int k = j;

						while (k < next_word.length() && next_word[k] != ' ' && next_word[k] != '/') {
							k++;
						}

						int n_index = std::atoi(next_word.substr(j, k - j).c_str());
						nor_indices.push_back(n_index);
					}

					if (indices.size() < 3) {
						std::cout << "ERROR::Importer::loadModel file error indices too little at" << line_id << " whare indices is " << indices.size() << std::endl;
						return;
					}

					// 处理负数index
					for (unsigned int i = 0; i < indices.size(); i++) {
						if (indices[i] < 0) {
							indices[i] = (int)points_.size() - indices[i];
						}
					}
					// 处理负数nor_index
					for (unsigned int i = 0; i < nor_indices.size(); i++) {
						if (nor_indices[i] < 0) {
							nor_indices[i] = (int)normals_.size() - nor_indices[i];
						}
					}


					for (unsigned int i = 0; i < indices.size(); i++) {
						f.indices.push_back(indices[i] - 1);
					}
					for (unsigned int i = 0; i < nor_indices.size(); i++) {
						f.nor_indices.push_back(nor_indices[i] - 1);
					}

					glm::vec3 normal(0.0f, 0.0f, 0.0f);

					glm::vec3 real_nor(0.0f, 0.0f, 0.0f);

					glm::vec3 p1(points_[f.indices[0]].position.x, points_[f.indices[0]].position.y, points_[f.indices[0]].position.z);
					glm::vec3 p2(points_[f.indices[1]].position.x, points_[f.indices[1]].position.y, points_[f.indices[1]].position.z);
					glm::vec3 p3(points_[f.indices[2]].position.x, points_[f.indices[2]].position.y, points_[f.indices[2]].position.z);

					glm::vec3 v1 = p2 - p1;
					glm::vec3 v2 = p3 - p2;
					real_nor = glm::normalize(glm::cross(v1, v2));

					if (f.nor_indices.empty()) {
						normal = real_nor;
					}
					else {
						glm::vec3 temp_nor(0.0f, 0.0f, 0.0f);
						for (unsigned int j = 0; j < f.nor_indices.size(); j++) {
							temp_nor += normals_[f.nor_indices[j]];
						}
						temp_nor.x = temp_nor.x / f.nor_indices.size();
						temp_nor.y = temp_nor.y / f.nor_indices.size();
						temp_nor.z = temp_nor.z / f.nor_indices.size();

						normal = glm::normalize(temp_nor);
					}

					float c = 0.0f;
					// negative surface
					if (normal.z < 0.0) {
						;
					}
					else {

						double cos_alpha = glm::dot(normal, glm::normalize(light_pos_));
						if (cos_alpha < 0.0f) {
							// diffuse add ambient
							c = 0.0f + 0.1f;
						}
						else {
							// diffuse add ambient
							c = std::min((float)std::pow(cos_alpha, 2) + 0.1f, 1.0f);
						}

						f.normal_ = real_nor;
						sur_faces_.push_back(f);
						color_.push_back(c);
					}

				}
				else if (next_word == "vn") {

					std::vector<float> nor;

					while ((next_word = getNextStr(line_data, index)) != "") {
						float val = (float)std::atof(next_word.c_str());
						nor.push_back(val);
					}
					if (nor.size() != 3) {
						std::cout << "ERROR::Importer loadModel. The point positions parameter is error. Get " << nor.size() << " parameters." << std::endl;
						return;
					}

					//glm::vec3 n(nor[0], nor[1], nor[2]);

					glm::vec3 n(0.0f, 0.0f, 0.0f);

					n.x = (float)(sin_alpha_ * nor[2] + cos_alpha_ * nor[0]);
					n.z = (float)(cos_alpha_ * nor[2] + sin_alpha_ * nor[0] * (-1.0));
					nor[2] = n.z;

					n.z = (float)(cos_beta_ * nor[2] + sin_beta_ * nor[1] * (-1.0));
					n.y = (float)(sin_beta_ * nor[2] + cos_beta_ * nor[1]);

					if (global::floatEqual(n.x, 0.0f)) {
						n.x = 0.0f;
					}
					if (global::floatEqual(n.y, 0.0f)) {
						n.y = 0.0f;
					}
					if (global::floatEqual(n.z, 0.0f)) {
						n.z = 0.0f;
					}

					normals_.push_back(n);

				}
				else {		// other, no means
				}
			}
		}
		std::cout << "HINT::Model Load model cost :" << float(clock() - t) << "ms." << std::endl;
	}

	void scaleModel() {

		std::cout << "HINT::Model scale model." << std::endl;

		initMinMaxXYZ();

		std::cout << "before scale, min_z_:" << min_z_ << "max_z_:" << max_z_ << std::endl;

		// 只需要考虑x,y不需要考虑z的平移
		float scale = -1.0f;

		if (max_x_ - min_x_ > max_y_ - min_y_) {

			scale = 2.0f / (max_x_ - min_x_);
			// 放缩
			for (unsigned int i = 0; i < points_.size(); i++) {

				// x,y放缩再平移
				points_[i].position.x = points_[i].position.x * scale - (min_x_ * scale + 1.0f);
				points_[i].position.y = points_[i].position.y * scale - ((max_y_ + min_y_) / 2.0f) * scale;
				//// z只放缩 再平移(平移到(-x, 0))
				points_[i].position.z = points_[i].position.z * scale;
				points_[i].position.z -= (max_z_ * scale);
				if (points_[i].position.z >= 0.0f) {
					points_[i].position.z = 0.0f;
				}

			}
		}
		else {
			scale = 2.0f / (max_y_ - min_y_);
			// 放缩
			for (unsigned int i = 0; i < points_.size(); i++) {

				// x,y放缩再平移
				points_[i].position.x = points_[i].position.x * scale - (max_x_ + min_x_) / 2.0f * scale;
				points_[i].position.y = points_[i].position.y * scale - (min_y_ * scale + 1.0f);
				//// z只放缩 再平移(平移到(-x, 0))
				points_[i].position.z = points_[i].position.z * scale;
				points_[i].position.z -= (max_z_ * scale);
				if (points_[i].position.z >= 0.0f) {
					points_[i].position.z = 0.0f;
				}
			}
		}
		initMinMaxXYZ();
		std::cout << "after scale, min_z_:" << min_z_ << "max_z_:" << max_z_ << std::endl;

	};

	std::string getNextStr(std::string& str, unsigned int& index) {
		unsigned int j = index;
		while (j < str.length() && str[j] == ' ') {
			j++;
		}

		if (j >= str.length()) {
			return "";
		}
		unsigned int k = j;
		while (k < str.length() && str[k] != ' ') {
			k++;
		}
		index = k;

		std::string ret = str.substr(j, k - j);
		return ret;
	}

	void initMinMaxXYZ() {
		min_x_ = FLT_MAX;
		min_y_ = FLT_MAX;
		min_z_ = FLT_MAX;

		max_x_ = -FLT_MAX;
		max_y_ = -FLT_MAX;
		max_z_ = -FLT_MAX;
		int size = (int)points_.size();
#pragma omp parallel for
		for (int i = 0; i < size; i++) {
#pragma omp critical			
			{
				max_x_ = std::max(max_x_, points_[i].position.x);
				max_y_ = std::max(max_y_, points_[i].position.y);
				max_z_ = std::max(max_z_, points_[i].position.z);

				min_x_ = std::min(min_x_, points_[i].position.x);
				min_y_ = std::min(min_y_, points_[i].position.y);
				min_z_ = std::min(min_z_, points_[i].position.z);
			}
		}
	};

	std::string path_ = "";

	int num_surfaces_ = 0;



	glm::vec3 light_pos_ = glm::vec3(-1.0f, 1.0f, 1.0f);	// 光源的位置，用来设置表面颜色。
	glm::vec3 camera_pos_ = glm::vec3(0.0f, 0.0f, 1.0f);

	double cos_alpha_ = 1.0f;
	double sin_alpha_ = 0.0f;
	double cos_beta_ = 1.0f;
	double sin_beta_ = 0.0f;
};