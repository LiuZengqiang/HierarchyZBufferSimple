// 四叉树
#pragma once
#include"tool\globalFunction.cpp"
#include"GL\freeglut.h"
#include"GL\glut.h"
#include<vector>
#include<iostream>
#include<queue>
struct PyramidNode
{
	// 所表示的屏幕
	int x_l;
	int x_r;
	int y_b;
	int y_t;
	float depth_val;
	std::vector<PyramidNode*> children;
	PyramidNode* parent;
	PyramidNode(int xl, int xr, int yb, int yt, PyramidNode* par) {
		x_l = xl;
		x_r = xr;
		y_b = yb;
		y_t = yt;
		parent = par;
		// -------------------
		// | child0 | child1 |
		// -------------------
		// | child2 | child3 |
		// -------------------
		children = std::vector<PyramidNode*>(4, nullptr);
		depth_val = -FLT_MAX;

	}
};

class Pyramid
{
public:
	Pyramid() {
		root_ = nullptr;
		scr_height_ = scr_width_ = 0;
	}

	void build(unsigned int scr_width, unsigned int scr_height) {
		scr_height_ = scr_height;
		scr_width_ = scr_width;
		
		z_buffer_node_.clear();
		std::vector<PyramidNode*> z(scr_width * scr_height, nullptr);
		z_buffer_node_.assign(z.begin(), z.end());

		root_ = buildPyramid(0, scr_width - 1, 0, scr_height - 1, nullptr);
	}

	~Pyramid() {};

	void debug() {

		std::cout << "pyramid depth is:" << getHeight(root_) << std::endl;

		/*	std::cout << "-----hirerachy travel tree-----" << std::endl;
			std::queue<PyramidNode*> que;
			que.push(root_);
			while (!que.empty()) {
				std::queue<PyramidNode*> temp_que;

				while (!que.empty()){
					PyramidNode* n = que.front();
					que.pop();

					std::cout << n->depth_val << " ";
					for (int i = 0; i < 4; i++) {
						if (n->children[i] != nullptr) {
							temp_que.push(n->children[i]);
						}
					}
				}
				std::cout << std::endl;
				que = temp_que;
			}
			std::cout << "-------------------------------" << std::endl;*/
	};

	// 用来debug的函数
	int getHeight(PyramidNode* root) {
		if (root == nullptr) {
			return 0;
		}

		int ret = getHeight(root->children[0]) + 1;
		ret = std::max(getHeight(root->children[1]) + 1, ret);
		ret = std::max(getHeight(root->children[2]) + 1, ret);
		ret = std::max(getHeight(root->children[3]) + 1, ret);

		return ret;
	}


	bool isRender(int p_x_l, int p_x_r, int p_y_b, int p_y_t, float max_z, unsigned int index) {

		//std::cout << "surface " << index << " " << " max_z:" << max_z << std::endl;

		if (root_ == nullptr || scr_height_ == 0 || scr_width_ == 0) {
			std::cout << "ERROR::Pyramid::isRender The Pyramid is not initialed?(root_==null || scr_height==0 || scr_width_==0)." << std::endl;
			return false;
		}

		if (p_x_r > scr_width_ || p_x_l < 0 || p_y_b<0 || p_y_t>scr_height_) {

			std::cout << "ERROR::Pyramid::isRender surface " << index << "(" << p_x_l << " - " << p_x_r << ", " << p_y_b << " - " << p_y_t << ")" << "is out of range(0, 0) - (" << scr_width_ << ", " << scr_height_ << ")" << std::endl;
			exit(0);
			return false;
		}


		PyramidNode* node = findFinestNode(p_x_l, p_x_r, p_y_b, p_y_t, max_z, root_);
		/*	std::cout << "HINT:: surface " << index << ": (" << p_x_l << "," << p_y_b << "),(" << p_x_r << "," << p_y_t << ")." << std::endl;
			std::cout << "\tnode:(" << node->x_l << "," << node->y_b << "),(" << node->x_r << "," << node->y_t << ")." << std::endl;*/

		if (node == nullptr) {
			std::cout << "ERROR::isRender The finest node is nullptr?" << std::endl;
			return false;
		}


		if (max_z >= node->depth_val) {

			//TODO::complete update z_buffer_data and pyramid.z_buffer_node_;

			// 更新pyramid
			// 需要知道polygon的所有顶点信息,点node
			// 然后更新node节点窗口中所有的像素点
		/*	for (int x = node->x_l; x <= node->x_r; x++) {
				for (int y = node->y_b; y <= node->y_t; y++) {

					if ((x,y) in polygon and pyramid.z(x,y) < polygon(x,y)) {

						pyramid.update(node(x,y), polygon(x,y).z);
						z_buffer._data = polygon(x, y).color;
					}
					else {
						continue;
					}
				}
			}*/
			//updatePyramid(node, max_z);
			return true;
		}
		return false;

	}

	float getZ(unsigned int x, unsigned int y) {
		return z_buffer_node_[(size_t)y * scr_width_ + x]->depth_val;
	};
	void updatePyramid(unsigned int x, unsigned int y, float z) {
		PyramidNode* node = getNodeAddress(x, y);
		updatePyramid(node, z);
	}
	PyramidNode* getNodeAddress(unsigned int x, unsigned int y) {
		if (z_buffer_node_.size() <= 0) {
			std::cout << "ERROR::Pyramid::getNodeAdd z_buffer_node_ is nullptr." << std::endl;
			return nullptr;
		}
		return z_buffer_node_[(size_t)y * scr_width_ + x];
	}
private:
	void updatePyramid(PyramidNode* root, float z) {
		if (root == nullptr) {
			return;
		}

		root->depth_val = z;
		updatePyramid(root);
	}
	void updatePyramid(PyramidNode* root) {
		if (root->parent == nullptr) {
			return;
		}

		double pre_depth_val = root->parent->depth_val;

		std::vector<PyramidNode*> children = root->parent->children;

		float cur_depth_val = children[0] == nullptr ? FLT_MAX : children[0]->depth_val;
		cur_depth_val = std::min(cur_depth_val, children[1] == nullptr ? FLT_MAX : children[1]->depth_val);
		cur_depth_val = std::min(cur_depth_val, children[2] == nullptr ? FLT_MAX : children[2]->depth_val);
		cur_depth_val = std::min(cur_depth_val, children[3] == nullptr ? FLT_MAX : children[3]->depth_val);

		if (global::doubleEqual(pre_depth_val, cur_depth_val)) {	// stop update;
			return;
		}
		else
		{
			root->parent->depth_val = cur_depth_val;
			updatePyramid(root->parent);
		}
	};
	PyramidNode* findFinestNode(int p_x_l, int p_x_r, int p_y_b, int p_y_t, float p_max_z, PyramidNode* root) {
		if (root == nullptr) {
			std::cout << "ERROR::Pyramid::findFinestNode the root is nullptr." << std::endl;
			return nullptr;
		}

		// 如果当前节点最小深度大于p_z那么，显然不用继续往下搜索finestNode
		if (root->depth_val >= p_max_z) {
			return root;
		}

		int x_l = root->x_l;
		int x_r = root->x_r;
		int y_b = root->y_b;
		int y_t = root->y_t;
		int x_mid = (x_l + x_r) >> 1;
		int y_mid = (y_b + y_t) >> 1;
		if ((x_mid >= p_x_l && x_mid <= p_x_r) || (y_mid >= p_y_b && y_mid <= p_y_t)) {
			return root;
		}
		else if (p_x_l >= x_l && p_x_l <= x_mid && p_x_r >= x_l && p_x_r <= x_mid && p_y_b > y_mid && p_y_b <= y_t && p_y_t > y_mid && p_y_t <= y_t) {
			return findFinestNode(p_x_l, p_x_r, p_y_b, p_y_t, p_max_z,root->children[0]);
		}
		else if (p_x_l > x_mid && p_x_l <= x_r && p_x_r > x_mid && p_x_r <= x_r && p_y_b > y_mid && p_y_b <= y_t && p_y_t > y_mid && p_y_t <= y_t) {
			return findFinestNode(p_x_l, p_x_r, p_y_b, p_y_t, p_max_z,root->children[1]);
		}
		else if (p_x_l >= x_l && p_x_l <= x_mid && p_x_r >= x_l && p_x_r <= x_mid && p_y_b >= y_b && p_y_b <= y_mid && p_y_t >= y_b && p_y_t <= y_mid) {
			return findFinestNode(p_x_l, p_x_r, p_y_b, p_y_t, p_max_z,root->children[2]);
		}
		else if (p_x_l > x_mid && p_x_l <= x_r && p_x_r > x_mid && p_x_r <= x_r && p_y_b >= y_b && p_y_b <= y_mid && p_y_t >= y_b && p_y_t <= y_mid) {
			return findFinestNode(p_x_l, p_x_r, p_y_b, p_y_t, p_max_z, root->children[3]);
		}
		else {
			std::cout << "ERROR::Pyramid::findFinestPyramidNode." << std::endl;
			std::cout << "p_x_l:" << p_x_l << " p_x_r : " << p_x_r << " p_y_b : " << p_y_b << " p_y_t : " << p_y_t << std::endl;
			std::cout << "root: x_l:" << x_l << " x_mid:" << x_mid << " x_r:" << x_r << " y_b:" << y_b << " y_mid:" << y_mid << " y_t:" << y_t << std::endl;
			return nullptr;
		}
	}
	PyramidNode* buildPyramid(int x_l, int x_r, int y_b, int y_t, PyramidNode* parent) {

		if (x_l > x_r || y_b > y_t) {	// invalid
			return nullptr;
		}

		PyramidNode* p_node = new PyramidNode(x_l, x_r, y_b, y_t, parent);

		if (x_l == x_r && y_b == y_t) {	// one pixel
			z_buffer_node_[(size_t)y_b * scr_width_ + x_l] = p_node;
		}
		else if (x_l == x_r) {	// one column
			int y_mid = (y_b + y_t) >> 1;
			p_node->children[0] = buildPyramid(x_l, x_r, y_b, y_mid, p_node);
			p_node->children[2] = buildPyramid(x_l, x_r, y_mid + 1, y_t, p_node);
		}
		else if (y_b == y_t) {	// one raw
			int x_mid = (x_l + x_r) >> 1;
			p_node->children[0] = buildPyramid(x_l, x_mid, y_b, y_t, p_node);
			p_node->children[1] = buildPyramid(x_mid + 1, x_r, y_b, y_t, p_node);
		}
		else {					// one block
			int x_mid = (x_l + x_r) >> 1;
			int y_mid = (y_b + y_t) >> 1;
			p_node->children[0] = buildPyramid(x_l, x_mid, y_mid + 1, y_t, p_node);
			p_node->children[1] = buildPyramid(x_mid + 1, x_r, y_mid + 1, y_t, p_node);
			p_node->children[2] = buildPyramid(x_l, x_mid, y_b, y_mid, p_node);
			p_node->children[3] = buildPyramid(x_mid + 1, x_r, y_b, y_mid, p_node);
		}
		return p_node;
	};
	unsigned int scr_height_ = 512;
	unsigned int scr_width_ = 512;
	PyramidNode* root_ = nullptr;
	std::vector<PyramidNode* > z_buffer_node_;
};