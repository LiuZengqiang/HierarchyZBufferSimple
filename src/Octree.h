#pragma once
#include"Model.h"
#include<iostream>
#include<vector>
#include<string>
#include<set>
#include<tuple>
#include<queue>

/// <summary>
/// Octree Node
/// </summary>
struct OctreeNode
{
	std::string id;
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

		// valid boundary
		int x_l = lower_left_corner.x;
		int x_r = lower_left_corner.x + width - 1;
		int y_b = lower_left_corner.y;
		int y_t = lower_left_corner.y + height - 1;
		int z_b = lower_left_corner.z - depth + 1;
		int z_f = lower_left_corner.z;

		// is the node only respent a pixel
		if (x_l == x_r && y_b == y_t && z_b == z_f) {
			return;
		}

		hasChild = true;

		glm::ivec3 l = lower_left_corner;
		// 0
		childern[0] = new OctreeNode(l, (width + 1) / 2, (height + 1) / 2, (depth + 1) / 2);
		childern[0]->id = id + "0";
		// 1
		l.x = lower_left_corner.x + childern[0]->width;
		if (l.x > x_r) {
			childern[1] = nullptr;
		}
		else {
			childern[1] = new OctreeNode(l, width - childern[0]->width, childern[0]->height, childern[0]->depth);
			childern[1]->id = id + "1";
		}

		// 2
		l.x = lower_left_corner.x;
		l.y = lower_left_corner.y + childern[0]->height;
		if (l.y > y_t) {
			childern[2] = nullptr;
		}
		else {
			childern[2] = new OctreeNode(l, childern[0]->width, height - childern[0]->height, childern[0]->depth);
			childern[2]->id = id + "2";
		}

		// 3
		l.x = lower_left_corner.x + childern[0]->width;
		l.y = lower_left_corner.y + childern[0]->height;
		if (l.x > x_r || l.y > y_t) {
			childern[3] = nullptr;
		}
		else {
			childern[3] = new OctreeNode(l, width - childern[0]->width, height - childern[0]->height, childern[0]->depth);
			childern[3]->id = id + "3";
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
			childern[4]->id = id + "4";
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
			childern[5]->id = id + "5";
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
			childern[6]->id = id + "6";
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
			childern[7]->id = id + "7";
		}
	}

	std::vector<unsigned int > surfaces_thresh;
	std::vector<unsigned int > surfaces_inter;

	float max_z = -FLT_MAX;

	//Lower left corner
	glm::ivec3 lower_left_corner = glm::ivec3(-1, -1, -1);

	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int depth = 0;
	OctreeNode(glm::ivec3 l, unsigned int w, unsigned int h, unsigned int d) : lower_left_corner(l), width(w), height(h), depth(d) {
		max_z = -FLT_MAX;
		surfaces_inter.clear();
		surfaces_thresh.clear();
		std::vector<OctreeNode*> temp(8, nullptr);
		childern.assign(temp.begin(), temp.end());
	}
};

class Octree
{
public:

	Octree(Model& model, unsigned int scr_width, unsigned int scr_height, unsigned int scr_depth) {

		model_ = model;
		scr_width_ = scr_width;
		scr_height_ = scr_height;
		scr_depth_ = scr_depth;
		root_ = new OctreeNode(glm::ivec3(0, 0, 0), scr_width_, scr_height_, scr_depth_);
		root_->id = "0";
	};
	~Octree() {};
	void init() {
		std::cout << "HINT::Octree init." << std::endl;
		// init octree
		build();
		initOctreeNodeZ();
	}

	OctreeNode* getOctreeRoot() {
		return root_;
	}

private:

	/* build octree */
	void build() {
		for (unsigned int i = 0; i < model_.sur_faces_.size(); i++) {
			insertSur(root_, i);
		}
	};

	/* Init octree node's z value */
	void initOctreeNodeZ() {

		std::cout << "HINT::Octree initial node z value." << std::endl;

		initOctreeNodeZ(root_);
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

	/* Insert surface_index into node or node's childen */
	void insertSur(OctreeNode* node, unsigned int surface_index) {

		if (node == nullptr) {
			std::cout << "ERROR::Octree::insertSur The node is nullptr." << std::endl;
			std::cout << "\t surface index:" << surface_index << std::endl;
		}
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


					insertSur(node->childern[temp_id_1], node->surfaces_thresh[i]);
				}
				node->surfaces_thresh.clear();
			}
			return;
		}
		else {	// 如果在一个小cube 中
			// 如果node已经有child了
			// 直接加到对应的child中去
			if (node == nullptr) {
				std::cout << "ERROR::Octree::insertSur The node is nullptr." << std::endl;
				std::cout << "\t surface index:" << surface_index << std::endl;
				exit(0);
			}
			if (node->hasChild) {

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

						insertSur(node->childern[temp_id_1], node->surfaces_thresh[i]);
					}
					node->surfaces_thresh.clear();

				}

			}
		}

	}

	/* Get point in which children cube of node */
	int getLocateCubeIdPoint(OctreeNode* node, glm::ivec3 point, unsigned int surface_index) {

		if (node == nullptr) {
			std::cout << "ERROR::Octree::getLocateCubeIdPoint. The node is nullptr." << std::endl;
			return -1;
		}
		// x
		if (point.x < node->lower_left_corner.x || point.x >= node->lower_left_corner.x + (int)node->width) {
			std::cout << "ERROR::Octree getLocateCubeIdPoint. The x of point is out of range of node represent." << std::endl;
			return -1;
		}
		// y
		if (point.y < node->lower_left_corner.y || point.y >= node->lower_left_corner.y + (int)node->height) {
			std::cout << "ERROR::Octree getLocateCubeIdPoint. The y of point is out of range of node represent." << std::endl;
			return -1;
		}
		// z
		if (point.z > node->lower_left_corner.z || point.z <= (node->lower_left_corner.z - (int)node->depth)) {
			std::cout << "ERROR::Octree getLocateCubeIdPoint. The z of point is out of range of node represent." << std::endl;
			std::cout << "\t point.z:" << point.z << " but the valid range is[" << node->lower_left_corner.z << "," << node->lower_left_corner.z - node->depth << ")" << std::endl;
			std::cout << "\t surface:" << surface_index << ". node:" << node->id << std::endl;
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
	/* Get surface in which children cube of node */
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

	/* model */
	Model model_;

	/* octree root */
	OctreeNode* root_;

	/* threshold of one octree node's capacity */
	unsigned int threshold_ = 5;

	/* windows' width\height\depth */
	unsigned int scr_width_ = -1;
	unsigned int scr_height_ = -1;
	unsigned int scr_depth_ = -1;
};