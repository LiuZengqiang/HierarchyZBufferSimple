#pragma once
#include"Model.h"
#include"Scene.h"
#include<iostream>
#include<vector>
#include<string>
#include<set>
#include <tuple>
#include<queue>

// scene -> octree -> model
//	|------> pyramid


// 八叉树结构

// 八叉树节点
struct OctreeNode
{
	std::string id_;

	// childern的szie等于8(0-7)，各个子节点分布如下:
	//   / 6  / 7 /|
	//  / 2  / 3 / |	
	// _________-|7|
	// |2, |3  |3|_|
	// |------|_|5|
	// |0, |1  |1|/
	// _________/
	//
	std::vector<OctreeNode*> childern;
	bool hasChild = false;

	// x[0, width)
	// y[0, height)
	// z[0, -depth)
	void createChildren() {

		// 合法边界
		int x_l = lower_left_corner.x;
		int x_r = lower_left_corner.x + width - 1;

		int y_b = lower_left_corner.y;
		int y_t = lower_left_corner.y + height - 1;

		int z_b = lower_left_corner.z - depth + 1;
		int z_f = lower_left_corner.z;


		if (x_l == x_r && y_b == y_t && z_b == z_f) {
			return;
		}

		// 要满足靠左、靠前，靠下
		hasChild = true;


		glm::ivec3 l = lower_left_corner;
		// 0
		childern[0] = new OctreeNode(l, (width + 1) / 2, (height + 1) / 2, (depth + 1) / 2);
		childern[0]->id_ = id_ + "0";
		// 1
		l.x = lower_left_corner.x + childern[0]->width;
		if (l.x > x_r) {
			childern[1] = nullptr;
		}
		else {
			childern[1] = new OctreeNode(l, width - childern[0]->width, childern[0]->height, childern[0]->depth);
			childern[1]->id_ = id_ + "1";
		}

		// 2
		l.x = lower_left_corner.x;
		l.y = lower_left_corner.y + childern[0]->height;
		if (l.y > y_t) {
			childern[2] = nullptr;
		}
		else {
			childern[2] = new OctreeNode(l, childern[0]->width, height - childern[0]->height, childern[0]->depth);
			childern[2]->id_ = id_ + "2";
		}

		// 3
		l.x = lower_left_corner.x + childern[0]->width;
		l.y = lower_left_corner.y + childern[0]->height;
		if (l.x > x_r || l.y > y_t) {
			childern[3] = nullptr;
		}
		else {
			childern[3] = new OctreeNode(l, width - childern[0]->width, height - childern[0]->height, childern[0]->depth);
			childern[3]->id_ = id_ + "3";
		}
		// 4
		l.x = lower_left_corner.x;
		l.y = lower_left_corner.y;
		l.z = lower_left_corner.z - childern[0]->depth;
		if (l.z < z_b) {
			childern[4] = nullptr;
		}
		else {
			childern[4] = new OctreeNode(l, childern[0]->width, childern[0]->height, depth - childern[0]->depth);
			childern[4]->id_ = id_ + "4";
		}

		// 5
		if (childern[1] == nullptr || childern[4] == nullptr) {
			childern[5] = nullptr;
		}
		else {
			l.x = childern[1]->lower_left_corner.x;
			l.y = childern[1]->lower_left_corner.y;
			l.z = childern[4]->lower_left_corner.z;
			childern[5] = new OctreeNode(l, childern[1]->width, childern[1]->height, childern[4]->depth);
			childern[5]->id_ = id_ + "5";
		}
		// 6
		if (childern[2] == nullptr || childern[4] == nullptr) {
			childern[6] = nullptr;
		}
		else {
			l.x = childern[2]->lower_left_corner.x;
			l.y = childern[2]->lower_left_corner.y;
			l.z = childern[4]->lower_left_corner.z;
			childern[6] = new OctreeNode(l, childern[2]->width, childern[2]->height, childern[4]->depth);
			childern[6]->id_ = id_ + "6";
		}
		// 7
		if (childern[3] == nullptr || childern[4] == nullptr) {
			childern[7] = nullptr;
		}
		else {
			l.x = childern[3]->lower_left_corner.x;
			l.y = childern[3]->lower_left_corner.y;
			l.z = childern[4]->lower_left_corner.z;
			childern[7] = new OctreeNode(l, childern[3]->width, childern[3]->height, childern[4]->depth);
			childern[7]->id_ = id_ + "7";
		}
	}

	std::vector<unsigned int > surfaces_thresh;	// 默认的节点中可以包含小于threshold数量的多边形
	std::vector<unsigned int > surfaces_inter;	// 与该节点相交的多变形

	OctreeNode* parent;

	float max_z = -FLT_MAX;	// 包含的surface的最大值

	//左下角和长宽高?
	glm::ivec3 lower_left_corner = glm::ivec3(-1, -1, -1);
	//Lower left corner
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int depth = 0;
	OctreeNode(glm::ivec3 l, unsigned int w, unsigned int h, unsigned int d) : lower_left_corner(l), width(w), height(h), depth(d) {
		max_z = -FLT_MAX;
		parent = nullptr;
		surfaces_inter.clear();
		surfaces_thresh.clear();
		std::vector<OctreeNode*> temp(8, nullptr);
		childern.assign(temp.begin(), temp.end());
	}
	void debug() {

		std::cout << "node:" << id_ << std::endl;
		std::cout << "\t(" << lower_left_corner.x << "," << lower_left_corner.y << "," << lower_left_corner.z << ") ";
		std::cout << "width:" << width << " height:" << height << " depth:" << depth << std::endl;
		std::cout << "\t surface_thresh:";
		for (unsigned int i = 0; i < surfaces_thresh.size(); i++) {
			std::cout << surfaces_thresh[i] << " ";
		}
		std::cout << "\t surface_inter:";
		for (unsigned int i = 0; i < surfaces_inter.size(); i++) {
			std::cout << surfaces_inter[i] << " ";
		}
		std::cout << std::endl;
	}

	bool isPixel() {
		return width == 1 && height == 1 && depth == 1;
	}
};

class Octree
{
public:
	Octree(unsigned int scr_width, unsigned int scr_height, unsigned int scr_depth, std::string model_path, glm::vec3 camer_pos, glm::vec3 light_pos) {
		scr_width_ = scr_width;
		scr_height_ = scr_height;
		scr_depth_ = scr_depth;
		model_path_ = model_path;

		camera_pos_ = camer_pos;
		light_pos_ = light_pos;

		root_ = new OctreeNode(glm::ivec3(0, 0, 0), scr_width_, scr_height_, scr_depth_);
		root_->id_ = "0";

	};
	~Octree() {};

	void init() {

		z_buffer_data_ = (GLubyte*)malloc((size_t)scr_height_ * scr_width_ * 3 * sizeof(GLubyte));
		if (z_buffer_data_ == nullptr) {
			std::cout << "ERROR::Octree::init Malloc memory to z_buffer_data_ fail." << std::endl;
			exit(0);
		}
		else {
			memset(z_buffer_data_, 0, (size_t)scr_height_ * scr_width_ * 3 * sizeof(GLubyte));
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

		// init octree
		build();

	}

	void debugHierarichy() {
		std::cout << "----- Hierarchy Travel Begin-----" << std::endl;
		std::queue<OctreeNode* > que;
		int cnt = 0;
		que.push(root_);

		while (!que.empty()) {
			OctreeNode* temp = que.front();
			que.pop();

			if (temp->surfaces_inter.empty() && temp->surfaces_thresh.empty()) {
				;
			}
			else {
				cnt += temp->surfaces_inter.size();
				cnt += temp->surfaces_thresh.size();
				temp->debug();
			}



			for (int i = 0; i < 8; i++) {
				if (temp->childern[i] != nullptr) {
					que.push(temp->childern[i]);
				}
			}
		}
		std::cout << "----- Hierarchy Travel End-----" << std::endl;

		std::cout << "The number of all surfaces is:" << model_.sur_faces_.size() << std::endl;
		std::cout << "The number of surfaces in octree is:" << cnt << std::endl;

	}
	void debugNodeZ() {
		std::cout << "----- Hierarchy Travel node z Begin-----" << std::endl;
		std::queue<OctreeNode* > que;

		que.push(root_);

		while (!que.empty()) {
			OctreeNode* temp = que.front();
			que.pop();
			if (temp->surfaces_inter.empty() && temp->surfaces_thresh.empty()) {
				;
			}
			else {
				std::cout << temp->id_ << ": z:" << pointZ2PicelZ(temp->max_z) << std::endl;
			}
			for (int i = 0; i < 8; i++) {
				if (temp->childern[i] != nullptr) {
					que.push(temp->childern[i]);
				}
			}
		}
		std::cout << "----- Hierarchy Travel node z End-----" << std::endl;
	}

	// 初始化各个节点OctTreeNode的z值
	void initOctreeNodeZ() {
		initOctreeNodeZ(root_);
	}

	GLubyte* getZBufferData() {
		return z_buffer_data_;
	}

	void inOrderTraversal() {
		std::cout << "HINT::Octree inOrderTraveral." << std::endl;
		inOrderTraversal(root_);
	}
private:
	// 初始化octree
	void build() {

		for (unsigned int i = 0; i < model_.sur_faces_.size(); i++) {
			insertSur(root_, i);
		}
	};
	void inOrderTraversal(OctreeNode* node) {
		if (node == nullptr) {
			return;
		}
		for (unsigned int i = 0; i < 4; i++) {
			inOrderTraversal(node->childern[i]);
		}

		// 判断当前node是否需要绘制
		// TODO::20210105
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

			if (pyramid_.isRender(n_x_l, n_x_r, n_y_b, n_y_t, n_max_z, node->id_)) {

				// 可能绘制，那么就遍历node中的多边形，挨个绘制
				for (unsigned int i = 0; i < node->surfaces_thresh.size(); i++) {
					renderPolygon(node->surfaces_thresh[i]);
				}

				for (unsigned int i = 0; i < node->surfaces_inter.size(); i++) {
					renderPolygon(node->surfaces_inter[i]);
				}
			}
			else {
				std::cout << "HINT::Octree::inOrderTraveral The node:" << node->id_ << " dont be render." << std::endl;
				std::cout << "\t The max z of node is " << n_max_z << "" << std::endl;
			}
		}


		for (unsigned int i = 4; i < 8; i++) {
			inOrderTraversal(node->childern[i]);
		}
	}


	void initOctreeNodeZ(OctreeNode* node) {
		if (node == nullptr) {
			return;
		}

		if (node->surfaces_inter.empty() && node->surfaces_thresh.empty()) {
			;
		}
		else {
			float z = -FLT_MAX;
			for (unsigned int i = 0; i < node->surfaces_inter.size(); i++) {
				z = std::max(z, model_.getMaxZ(node->surfaces_inter[i]));
			}
			for (unsigned int i = 0; i < node->surfaces_thresh.size(); i++) {
				z = std::max(z, model_.getMaxZ(node->surfaces_thresh[i]));
			}
			node->max_z = z;
		}

		for (unsigned int i = 0; i < 8; i++) {
			initOctreeNodeZ(node->childern[i]);
		}
	}

	// 绘制单个多边形
	void renderPolygon(unsigned int i) {
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
	}

	void insertSur(OctreeNode* node, unsigned int surface_index) {

		if (node==nullptr) {
			std::cout << "ERROR::Octree::insertSur The node is nullptr." << std::endl;
			std::cout << "\t surface index:" << surface_index << std::endl;
		}

		/*if (surface_index==580) {
			model_.debugSurface(surface_index);
		}*/

		// 判断多边形的所有点是否在同一个小立方体中
		// 得到多边形各个点所在的正方形编号

		// 这可以看作两个点：左下角和右上角零个点
		// 如果着两个点在同一个小立方体中，那么...
		// 不然，说明与node立方体的评分面相交，就将该surface放到node中
		std::pair<int, int> cube_id = getLocateCubeId(node, surface_index);
		int id_1 = cube_id.first;
		int id_2 = cube_id.second;

		// 如果跨越分界面
		if (id_1 != id_2) {

			node->surfaces_inter.push_back(surface_index);

			// 总数大于threshhold并且hasChilde==false,把surface_thresh中的多边形分到各个子节点中
			if (node->hasChild == false && (node->surfaces_inter.size() + node->surfaces_thresh.size() > threshold_)) {
				node->createChildren();

				for (unsigned int i = 0; i < node->surfaces_thresh.size(); i++) {
					int temp_id_1, temp_id_2;
					std::tie(temp_id_1, temp_id_2) = getLocateCubeId(node, node->surfaces_thresh[i]);
					if (temp_id_1 != temp_id_2 || temp_id_1 == -1) {
						std::cout << "ERROR::insertSur temp_id_1==-1 or temp_id_1(" << temp_id_1 << ") != temp_id_2(" << temp_id_2 << ")." << std::endl;
						return;
					}

					if (node->childern[temp_id_1] == nullptr) {
						node->debug();
						exit(0);
					}
					insertSur(node->childern[temp_id_1], node->surfaces_thresh[i]);
				}
				node->surfaces_thresh.clear();
			}
			return;
		}
		else {	// 如果在一个小cube 中
			// 如果node已经有child了
			// 直接加到对应的child中去
			if (node->hasChild) {
				if (node->childern[id_1] == nullptr) {
					node->debug();
					exit(1);
				}
				insertSur(node->childern[id_1], surface_index);
			}
			else if (node->surfaces_thresh.size() + node->surfaces_inter.size() < threshold_) {	// 如果node的data数量小于阈值
				node->surfaces_thresh.push_back(surface_index);
			}
			else {	//	大于阈值

				node->createChildren();
				if (node->hasChild == false) {	//说明node只代表一个像素
					node->surfaces_inter.push_back(surface_index);
				}
				else {
					insertSur(node->childern[id_1], surface_index);

					for (unsigned int i = 0; i < node->surfaces_thresh.size(); i++) {
						int temp_id_1, temp_id_2;
						std::tie(temp_id_1, temp_id_2) = getLocateCubeId(node, node->surfaces_thresh[i]);
						if (temp_id_1 != temp_id_2 || temp_id_1 == -1) {
							std::cout << "ERROR::insertSur temp_id_1==-1 or temp_id_1(" << temp_id_1 << ") != temp_id_2(" << temp_id_2 << ")." << std::endl;
							return;
						}
						if (node->childern[temp_id_1] == nullptr) {
							node->debug();
							exit(3);
						}
						insertSur(node->childern[temp_id_1], node->surfaces_thresh[i]);
					}
					node->surfaces_thresh.clear();

				}

			}
		}

	}

	int getLocateCubeIdPoint(OctreeNode* node, glm::ivec3 point, unsigned int surface_index) {

		if (node == nullptr) {
			std::cout << "ERROR::Octree::getLocateCubeIdPoint. The node is nullptr." << std::endl;
			return -1;
		}
		// x
		if (point.x < node->lower_left_corner.x || point.x >= node->lower_left_corner.x + node->width) {
			std::cout << "ERROR::Octree getLocateCubeIdPoint. The x of point is out of range of node represent." << std::endl;
			return -1;
		}
		// y
		if (point.y < node->lower_left_corner.y || point.y >= node->lower_left_corner.y + node->height) {
			std::cout << "ERROR::Octree getLocateCubeIdPoint. The y of point is out of range of node represent." << std::endl;
			return -1;
		}
		// z
		if (point.z > node->lower_left_corner.z || point.z <= (node->lower_left_corner.z - (int)node->depth)) {
			std::cout << "ERROR::Octree getLocateCubeIdPoint. The z of point is out of range of node represent." << std::endl;
			std::cout << "\t point.z:" << point.z << " but the valid range is[" << node->lower_left_corner.z << "," << node->lower_left_corner.z - node->depth << ")" << std::endl;
			std::cout << "\t surface:" << surface_index << ". node:" << node->id_ << std::endl;
			std::cout << node->lower_left_corner.z << " " << (int)node->depth << " " << (node->lower_left_corner.z - (int)node->depth) << std::endl;
			return -1;
		}

		int ret = 0;
		// mid_x是分界面右/上/后的坐标
		int mid_x = (node->lower_left_corner.x + (int)((node->width + 1) / 2));
		int mid_y = (node->lower_left_corner.y + (int)((node->height + 1) / 2));
		int mid_z = (node->lower_left_corner.z - (int)((node->depth + 1) / 2));

		if (point.x >= mid_x) {
			ret = ret | 1;
		}
		if (point.y >= mid_y) {
			ret = ret | 2;
		}
		if (point.z <= mid_z) {
			ret = ret | 4;
		}
		return ret;
	}


	int pointZ2PicelZ(float z) {

		std::pair<float, float> min_max_model_z = model_.getModelMinMaxZ();
		int ret = (int)std::round((z - min_max_model_z.first) / (min_max_model_z.second - min_max_model_z.first) * (scr_depth_ - 1) - scr_depth_ + 1);
		return ret;

	}

	std::pair<int, int> getLocateCubeId(OctreeNode* node, unsigned int surface_index) {
		std::pair<float, float> min_max_x = model_.getMinMaxX(surface_index);
		std::pair<float, float> min_max_y = model_.getMinMaxY(surface_index);

		std::pair<float, float> min_max_z = model_.getMinMaxZ(surface_index);
		std::pair<float, float> min_max_model_z = model_.getModelMinMaxZ();

		// 多边形的最大最小xyz点
		// 转为像素值
		int p_x_l = (int)std::round((1 + min_max_x.first) / 2.0f * (scr_width_ - 1));
		int p_x_r = (int)std::round((1 + min_max_x.second) / 2.0f * (scr_width_ - 1));
		int p_y_b = (int)std::round((1 + min_max_y.first) / 2.0f * (scr_height_ - 1));
		int p_y_t = (int)std::round((1 + min_max_y.second) / 2.0f * (scr_height_ - 1));

		// p_z_b,p_z_f --->[-depth+1,0]

		int p_z_b = (int)std::round((min_max_z.first - min_max_model_z.first) / (min_max_model_z.second - min_max_model_z.first) * (scr_depth_ - 1) - scr_depth_ + 1);
		int p_z_f = (int)std::round((min_max_z.second - min_max_model_z.first) / (min_max_model_z.second - min_max_model_z.first) * (scr_depth_ - 1) - scr_depth_ + 1);

		glm::ivec3 lower_left_point(p_x_l, p_y_b, p_z_f);
		glm::ivec3 upper_right_point(p_x_r, p_y_t, p_z_b);

		std::pair<int, int> ret(getLocateCubeIdPoint(node, lower_left_point, surface_index), getLocateCubeIdPoint(node, upper_right_point, surface_index));

		return ret;
	}


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


	Model model_;

	Pyramid pyramid_;

	OctreeNode* root_;

	GLubyte* z_buffer_data_;

	unsigned int threshold_ = 5;	// 

	unsigned int scr_width_ = -1;
	unsigned int scr_height_ = -1;
	unsigned int scr_depth_ = -1;

	std::string model_path_;


	glm::vec3 camera_pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 light_pos_ = glm::vec3(-1.0f, 1.0f, 1.0f);
};