//TODO::
//要把代码写成诗一样！！！

#pragma once
#include"Pyramid.h"
#include"Model.h"
#include"Octree.h"
#include<string>

class Scene
{
public:
	// 输入参数
	Scene(std::string model_path, glm::vec3 camera_pos, glm::vec3 light_pos, unsigned int scr_width, unsigned int scr_height, unsigned int scr_depth = 720, int mode = 0) {
		model_path_ = model_path;
		camera_pos_ = camera_pos;
		light_pos_ = light_pos;
		scr_width_ = scr_width;
		scr_height_ = scr_height;
		scr_depth_ = scr_depth;
		mode_ = mode;
	};
	~Scene() {};
	// 初始化
	void init() {

		std::cout << "HINT::Scene init." << std::endl;
		color_data_ = (GLubyte*)malloc((size_t)scr_height_ * scr_width_ * 3 * sizeof(GLubyte));
		if (color_data_ == nullptr) {
			std::cout << "ERROR::Scene::init Malloc memory to color_data_ fail." << std::endl;
			exit(0);
		}
		else {
			memset(color_data_, 0, (size_t)scr_height_ * scr_width_ * 3 * sizeof(GLubyte));
		}

		// init model
		model_.setModelPath(model_path_);
		model_.setCameraPos(camera_pos_);
		model_.setLightPos(light_pos_);
		model_.init();
		
		// init pyramid
		pyramid_.setScrWidth(scr_width_);
		pyramid_.setScrHeight(scr_height_);
		pyramid_.init();
		
		// 简单模式
		if (mode_==0) {
		
		}
		else {
		// 完整模式
			octree_ = new Octree(model_, scr_width_, scr_height_, scr_depth_);
			octree_->init();
			octree_root_ = octree_->getOctreeRoot();
		}
	};
	// beginRender
	void beginRender() {
	
		if (mode_ == 0) {
			std::cout << "HINT::Scene Simple begin beginRender." << std::endl;
			for (unsigned int i = 0; i < model_.sur_faces_.size(); i++) {
				renderSurface(i);
			}
			std::cout << "The number of all surfaces is:" << model_.getSurfacesNum() << std::endl;
			std::cout << "The number of dont render surfaces is:" << cnt_not_render_surfaces_sim_ << std::endl;
		}
		else {

			std::cout << "HINT::Scene Complete beginRender." << std::endl;
			
			inOrderTraveral(octree_root_);
			std::cout << "The number of all surfaces is:" << model_.getSurfacesNum() << std::endl;
			std::cout << "The number of dont render node is:" << cnt_not_render_node_com_ << std::endl;
			std::cout << "The number of dont render node surface is:" << cnt_not_render_node_surface_com_ << std::endl;
			std::cout << "The number of dont render surface is:" << cnt_not_render_surfaces_com_ << std::endl;
			std::cout << "The number of dont render all surfaces is:" << cnt_not_render_node_surface_com_ + cnt_not_render_surfaces_com_ << std::endl;
		}
	};
	GLubyte* getColorData() {
		return color_data_;
	}
private:
	/* Inorder traveral the octree and render node */
	void inOrderTraveral(OctreeNode* node) {
		if (node == nullptr) {
			return;
		}
		for (unsigned int i = 0; i < 4; i++) {
			inOrderTraveral(node->childern[i]);
		}
		if (node->surfaces_inter.empty() && node->surfaces_thresh.empty()) {
			;
		}
		else {

			// p_xy_lrbt是node矩形的左右下上边，不是多边形的左右下上边
			int n_x_l = node->lower_left_corner.x;
			int n_x_r = node->lower_left_corner.x + node->width - 1;
			int n_y_b = node->lower_left_corner.y;
			int n_y_t = node->lower_left_corner.y + node->height - 1;
			float n_max_z = node->max_z;

			if (pyramid_.isRender(n_x_l, n_x_r, n_y_b, n_y_t, n_max_z, node->id)) {

				for (unsigned int i = 0; i < node->surfaces_thresh.size(); i++) {
					renderSurface(node->surfaces_thresh[i]);
				}

				for (unsigned int i = 0; i < node->surfaces_inter.size(); i++) {
					renderSurface(node->surfaces_inter[i]);
				}
			}
			else {
				//std::cout << "HINT::Octree::inOrderTraveral The node:" << node->id << " dont be render." << std::endl;
				//std::cout << "\t The max z of node is " << n_max_z << "" << std::endl;
				cnt_not_render_node_com_++;
				cnt_not_render_node_surface_com_ += (int)(node->surfaces_inter.size() + node->surfaces_thresh.size());
			}
		}

		for (unsigned int i = 4; i < 8; i++) {
			inOrderTraveral(node->childern[i]);
		}
	}

	/* Render the surface i */
	void renderSurface(unsigned int i) {
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
			for (unsigned int x = p_x_l; x <= (unsigned int)p_x_r; x++) {
				for (unsigned int y = p_y_b; y <= (unsigned int)p_y_t; y++) {
					glm::ivec2 point(x, y);

					// 如果(x,y)在多边形里面
					if (global::pointInPolygon(point, po_points)) {

						float f_x = ((float)x) / (scr_width_ - 1) * 2.0f - 1.0f;
						float f_y = ((float)y) / (scr_height_ - 1) * 2.0f - 1.0f;
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

						}
						else if (z > pyramid_.getZ(x, y)) {
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
			if (mode_ == 0) {
				cnt_not_render_surfaces_sim_++;
			}
			else {
				cnt_not_render_surfaces_com_++;	
			}
		}
	}

	/* Set pixel color with mean color of previous and current color value*/
	void meanSetPixel(unsigned int x, unsigned int y, float color) {
		float c_r = color_data_[3 * (y * scr_width_ + x) + 0] + color * 255;
		c_r /= 2.0;
		c_r = std::min(c_r, 255.0f);
		color_data_[3 * (y * scr_width_ + x) + 0] = (GLubyte)(c_r);

		float c_g = color_data_[3 * (y * scr_width_ + x) + 1] + color * 255;
		c_g /= 2.0;
		c_g = std::min(c_g, 255.0f);
		color_data_[3 * (y * scr_width_ + x) + 1] = (GLubyte)(c_g);

		float c_b = color_data_[3 * (y * scr_width_ + x) + 2] + color * 255;
		c_b /= 2.0;
		c_b = std::min(c_b, 255.0f);
		color_data_[3 * (y * scr_width_ + x) + 2] = (GLubyte)(c_b);
	}

	/* Set pixel color with wiping neighbors pixel color */
	void setPixelAntialiasing(unsigned int x, unsigned int y, float color) {

		color_data_[3 * (y * scr_width_ + x) + 0] = (GLubyte)(color * 255);
		color_data_[3 * (y * scr_width_ + x) + 1] = (GLubyte)(color * 255);
		color_data_[3 * (y * scr_width_ + x) + 2] = (GLubyte)(color * 255);

		// 消除黑边锯齿
		// filter kernel
		// |0.0625 0.125 0.0625|
		// |0.125  1.0   0.125 |
		// |0.0625 0.125 0.0625|
		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				
				if ((dx == 0 && dy == 0) || (dx + x < 0 || dx + x >= scr_width_ || dy + y < 0 || dy + y >= scr_height_)) {
					continue;
				}
				else {
					if (pyramid_.getZ(dx + x, dy + y) <= -FLT_MAX) {
						if (dx * dy == 0) {
							int pre_co = color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0];
							int cur_co = (int)std::min(255.0f, pre_co + color * 255.0f * 0.125f);

							color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0] = (GLubyte)(cur_co);
							color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 1] = (GLubyte)(cur_co);
							color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 2] = (GLubyte)(cur_co);
						}
						else {
							int pre_co = color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0];
							int cur_co = (int)std::min(255.0f, pre_co + color * 255.0f * 0.0625f);
							color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 0] = (GLubyte)(cur_co);
							color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 1] = (GLubyte)(cur_co);
							color_data_[3 * ((dy + y) * scr_width_ + dx + x) + 2] = (GLubyte)(cur_co);

						}
					}
				}
			}
		}
	}

	int mode_ = 0;

	/* model */
	Model model_;
	/* pyramid */
	Pyramid pyramid_;
	
	/* octree */
	Octree* octree_;
	
	/* octree root */
	OctreeNode* octree_root_;
	

	/* color data of windows */
	GLubyte* color_data_;

	/* model file path */
	std::string model_path_;

	/* camera position */
	glm::vec3 camera_pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
	/* light position */
	glm::vec3 light_pos_ = glm::vec3(-1.0f, 1.0f, 1.0f);

	/* windows' width\height\depth */
	unsigned int scr_width_ = -1;
	unsigned int scr_height_ = -1;
	unsigned int scr_depth_ = -1;

	int cnt_not_render_node_com_ = 0;
	int cnt_not_render_node_surface_com_ = 0;
	int cnt_not_render_surfaces_com_ = 0;

	/* number of not render surfaces in simple render mode */
	int cnt_not_render_surfaces_sim_ = 0;
};