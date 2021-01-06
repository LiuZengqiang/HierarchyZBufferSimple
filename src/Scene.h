// TODO:
// 可以使用类似滤波的算法减少锯齿
#pragma once

#include"Model.h"
#include"Pyramid.h"
#include"GL\freeglut.h"
#include"GL\glut.h"
#include<iostream>
#include<string>
class Scene
{
public:
	Scene() {};
	Scene(unsigned int scr_width, unsigned int scr_height, std::string model_path) {
		scr_height_ = scr_height;
		scr_width_ = scr_width;
		model_path_ = model_path;
	};
	~Scene() {};

	void init() {

		std::cout << "HINT::Scene init." << std::endl;
		z_buffer_data_ = (GLubyte*)malloc((size_t)scr_height_ * scr_width_ * 3 * sizeof(GLubyte));
		if (z_buffer_data_ == nullptr) {
			std::cout << "ERROR::Scene::init Malloc memory to z_buffer_data_ fail." << std::endl;
			exit(0);
		}
		else {
			memset(z_buffer_data_, 0, (size_t)scr_height_ * scr_width_ * 3 * sizeof(GLubyte));
		}

		model_.setModelPath(model_path_);
		model_.setCameraPos(camera_pos_);
		model_.setLightPos(light_pos_);
		model_.init();

		pyramid_.setScrWidth(scr_width_);
		pyramid_.setScrHeight(scr_height_);
		pyramid_.init();

		SCENE_INIT = true;
	}

	// 开始比较深度绘制
	void beginRender() {

		if (!SCENE_INIT) {
			std::cout << "ERROR::Scene::beginRender Scene do not initialed, please call scene.init() ." << std::endl;
			return;
		}

		std::cout << "HINT::Scene begin render." << std::endl;

		int not_render_face_cnt = 0;

		int mean_pixel_cnt = 0;

		for (unsigned int i = 0; i < model_.sur_faces_.size(); i++) {


			std::pair<float, float> min_max_x = model_.getMinMaxX(i);
			std::pair<float, float> min_max_y = model_.getMinMaxY(i);
			float max_z = model_.getMaxZ(i);

			// 得到对应的像素位置
			int p_x_l = (int)std::round((1 + min_max_x.first) / 2.0f * (scr_width_ - 1));
			int p_x_r = (int)std::round((1 + min_max_x.second) / 2.0f * (scr_width_ - 1));
			int p_y_b = (int)std::round((1 + min_max_y.first) / 2.0f * (scr_height_ - 1));
			int p_y_t = (int)std::round((1 + min_max_y.second) / 2.0f * (scr_height_ - 1));

			// 输入为像素坐标和z值
			if (pyramid_.isRender(p_x_l, p_x_r, p_y_b, p_y_t, max_z, std::to_string(i))) {
				// update Pyramid 和 z_buffer_data_;
				// 更新pyramid只需要知道z_buffer_node_，然后再updatePramid(z_buffer_node_[x], z);
				
				
				std::vector<glm::ivec2> po_points;	//po_points里面存的是多边形的各个点像素值，用来遍历包围盒中的各个像素是否再多边形内

				for (unsigned int j = 0; j < model_.sur_faces_[i].indices.size(); j++) {
					int p_index = model_.sur_faces_[i].indices[j];
					int temp_x = (int)std::round((1.0f + model_.points_[p_index].position.x) / 2.0f * (scr_width_ - 1));
					int temp_y = (int)std::round((1.0f + model_.points_[p_index].position.y) / 2.0f * (scr_height_ - 1));
					glm::ivec2 temp_p(temp_x, temp_y);
					po_points.push_back(temp_p);
				}


				// TODO::使用扫描线算法?

				//for (unsigned int y = p_y_b; y <= p_y_t; y++) {
				//	//
				//	if (z>pyramid_z) {
				//		绘制
				//	}
				//	else {
				//		不绘制
				//	}
				//}

				for (unsigned int x = p_x_l; x <= (unsigned int)p_x_r; x++) {
					for (unsigned int y = p_y_b; y <= (unsigned int)p_y_t; y++) {
						glm::ivec2 point(x, y);

						// 如果(x,y)在多边形里面
						if (global::pointInPolygon(point, po_points)) {

							float f_x = ((float)x) / (scr_width_ - 1) * 2.0 - 1.0;
							float f_y = ((float)y) / (scr_height_ - 1) * 2.0 - 1.0;
							glm::vec3 ori(f_x, f_y, 0.0);
							int p_index = model_.sur_faces_[i].indices[0];
							glm::vec3 a(model_.points_[p_index].position.x, model_.points_[p_index].position.y, model_.points_[p_index].position.z);
							p_index = model_.sur_faces_[i].indices[1];
							glm::vec3 b(model_.points_[p_index].position.x, model_.points_[p_index].position.y, model_.points_[p_index].position.z);
							p_index = model_.sur_faces_[i].indices[2];
							glm::vec3 c(model_.points_[p_index].position.x, model_.points_[p_index].position.y, model_.points_[p_index].position.z);

							float z = global::interSectZ(ori, a, b, c);
							float pry_z = pyramid_.getZ(x, y);

							if (global::floatEqual(z, pry_z)) {
								this->meanSetPixel(x, y, model_.color_[i]);
								mean_pixel_cnt++;
							}
							else if (z > pyramid_.getZ(x, y)) {
								//this->setPixel(x, y, model_.color_[i]);
								setPixelAntialiasing(x, y, model_.color_[i]);
								pyramid_.updatePyramid(x, y, z);
							}
						}
						else {

						}
					}
				}
			}
			else {
				//std::cout << "HINT::Scene::beginRender Not render surface " << i << " ." << std::endl;
				not_render_face_cnt++;
			}
		}
		//showDepth();
		std::cout << "The number of all surfaces is :" << model_.sur_faces_.size() << std::endl;
		std::cout << "The number of dont render surfaces is :" << not_render_face_cnt << std::endl;
		std::cout << "The number of mean pixel is :" << mean_pixel_cnt << std::endl;
	};

	GLubyte* getZBufferData() {
		return z_buffer_data_;
	}

	void setLightPosition(glm::vec3 light_pos) {
		light_pos_ = glm::normalize(light_pos);
	};

	void setCameraPosition(glm::vec3 camera_pos) {
		camera_pos_ = glm::normalize(camera_pos);
	};

private:

	void showDepth() {
		float min_z = FLT_MAX;
		float max_z = -FLT_MAX;

		for (unsigned int x = 0; x < scr_width_; x++) {
			for (unsigned int y = 0; y < scr_height_; y++) {
				float z = pyramid_.getNodeAddress(x, y)->depth_val;
				if (z == -FLT_MAX) {
					continue;
				}
				else {
					min_z = std::min(min_z, z);
					max_z = std::max(max_z, z);
				}
			}
		}
		for (unsigned int x = 0; x < scr_width_; x++) {
			for (unsigned int y = 0; y < scr_height_; y++) {
				float z = pyramid_.getNodeAddress(x, y)->depth_val;
				if (z == max_z) {
					setPixel(x, y, 1.0);
					continue;
				}
				if (z == min_z || z == -FLT_MAX) {
					setPixel(x, y, 0.0);
				}
				else {
					setPixel(x, y, (z - min_z) / (max_z - min_z));
				}
			}
		}

	}
	// 判断多边形是否在win内部，线与多边形是否相交(交点在多边形内部，2D平面内)，
	// zbuffer指定像素颜色设置(光照)

	void setPixel(unsigned int x, unsigned int y, float color) {
		z_buffer_data_[3 * (y * scr_width_ + x) + 0] = (GLubyte)(color * 255);
		z_buffer_data_[3 * (y * scr_width_ + x) + 1] = (GLubyte)(color * 255);
		z_buffer_data_[3 * (y * scr_width_ + x) + 2] = (GLubyte)(color * 255);
	}

	void meanSetPixel(unsigned int x, unsigned int y, float color) {
		float c_r = z_buffer_data_[3 * (y * scr_width_ + x) + 0] + color * 255;
		c_r /= 2.0;
		c_r = std::min(c_r, 255.0f);
		z_buffer_data_[3 * (y * scr_width_ + x) + 0] = (GLubyte)(c_r);

		float c_g = z_buffer_data_[3 * (y * scr_width_ + x) + 1] + color * 255;
		c_g /= 2.0;
		c_g = std::min(c_g, 255.0f);
		z_buffer_data_[3 * (y * scr_width_ + x) + 1] = (GLubyte)(c_g);

		float c_b = z_buffer_data_[3 * (y * scr_width_ + x) + 2] + color * 255;
		c_b /= 2.0;
		c_b = std::min(c_b, 255.0f);
		z_buffer_data_[3 * (y * scr_width_ + x) + 2] = (GLubyte)(c_b);
	}


	void setPixelAntialiasing(unsigned int x, unsigned int y, float color) {

		z_buffer_data_[3 * (y * scr_width_ + x) + 0] = (GLubyte)(color * 255);
		z_buffer_data_[3 * (y * scr_width_ + x) + 1] = (GLubyte)(color * 255);
		z_buffer_data_[3 * (y * scr_width_ + x) + 2] = (GLubyte)(color * 255);

		// 消除黑边锯齿
		// |0.0625 0.125 0.0625|
		// |0.125  1.0   0.125 |
		// |0.0625 0.125 0.0625|
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				if (dx == 0 && dy == 0) {
					continue;
				}
				if (dx + x < 0 || dx + x >= scr_width_ || dy + y < 0 || dy + y >= scr_height_) {
					continue;
				}
				else {
					if (pyramid_.getZ(dx + x, dy + y) <= -FLT_MAX) {
						if (dx * dy == 0) {
							int pre_co = z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0];
							int cur_co = (int)std::min(255.0f, pre_co + color * 255.0f * 0.125f);

							z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0] = (GLubyte)(cur_co);
							z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 1] = (GLubyte)(cur_co);
							z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 2] = (GLubyte)(cur_co);
						}
						else {
							int pre_co = z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0];
							int cur_co = (int)std::min(255.0f, pre_co + color * 255.0f * 0.0625f);
							z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0] = (GLubyte)(cur_co);
							z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 1] = (GLubyte)(cur_co);
							z_buffer_data_[3 * ((dy + y) * scr_width_ + dx + x) + 2] = (GLubyte)(cur_co);

						}
					}
				}
			}
		}
	}

	std::string model_path_ = "";

	Model model_;
	Pyramid pyramid_;

	glm::vec3 camera_pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 light_pos_ = glm::vec3(-1.0f, 1.0f, 1.0f);

	unsigned int scr_height_ = 512;
	unsigned int scr_width_ = 512;
	// 存储的是颜色值
	GLubyte* z_buffer_data_;

	bool SCENE_INIT = false;
};