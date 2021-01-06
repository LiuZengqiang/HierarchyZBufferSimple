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
	}

	void setScrWidth(unsigned int scr_width) {
		scr_width_ = scr_width;
	};

	void setScrHeight(unsigned int scr_height) {
		scr_height_ = scr_height;
	};

	void init() {
		std::cout << "HINT::Pyramid initial pyramid." << std::endl;
		z_buffer_node_.clear();
		std::vector<PyramidNode*> z((int)scr_width_ * scr_height_, nullptr);

		z_buffer_node_.assign(z.begin(), z.end());

		root_ = buildPyramid(0, scr_width_ - 1, 0, scr_height_ - 1, nullptr);
	}

	~Pyramid() {};
	/// <summary>
	/// 
	/// </summary>
	/// <param name="p_x_l">待测试的多边形包围盒的 左边x 像素值</param>
	/// <param name="p_x_r">待测试的多边形包围盒的 右边x 像素值</param>
	/// <param name="p_y_b">待测试的多边形包围盒的 下边y 像素值</param>
	/// <param name="p_y_t">待测试的多边形包围盒的 上边y 像素值</param>
	/// <param name="max_z">待测试的多边形的 最大z 坐标值</param>
	/// <param name="index">待测试的面的index，用于debug的参数</param>
	/// <returns>true该多边形可以绘制/false该多边形显然不需要绘制</returns>
	bool isRender(unsigned int p_x_l, unsigned int p_x_r, unsigned int p_y_b, unsigned int p_y_t, float max_z, std::string index) {

		//std::cout << "surface " << index << " " << " max_z:" << max_z << std::endl;

		if (root_ == nullptr || scr_height_ == 0 || scr_width_ == 0) {
			std::cout << "ERROR::Pyramid::isRender The Pyramid is not initialed?(root_==null || scr_height==0 || scr_width_==0)." << std::endl;
			return false;
		}

		if (p_x_r > scr_width_ || p_x_l < 0 || p_y_b < 0 || p_y_t > scr_height_) {

			std::cout << "ERROR::Pyramid::isRender surface " << index << "(" << p_x_l << " - " << p_x_r << ", " << p_y_b << " - " << p_y_t << ")" << "is out of range(0, 0) - (" << scr_width_ << ", " << scr_height_ << ")" << std::endl;
			exit(0);
			return false;
		}


		PyramidNode* node = findFinestNode(p_x_l, p_x_r, p_y_b, p_y_t, max_z, root_);
	 
		if (node == nullptr) {
			std::cout << "ERROR::isRender The finest node is nullptr?" << std::endl;
			return false;
		}


		if (max_z >= node->depth_val) {
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