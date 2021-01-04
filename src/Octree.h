#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<set>

// scene -> octree -> model
//	|------> pyramid


// 八叉树结构

// 八叉树节点
struct OctreeNode
{
	unsigned int id;
	std::string id_;	// 使用string 类型做id?


	// childern的szie等于8，各个子节点分布如下:
 	//   / 7  / 8 /|
	//  / 3  / 4 / |	
	// _________-|8|
	// |3, |4  |4|_|
	// |-------|_|6|
	// |1, |2  |2|/
	// _________/
	//
	std::vector<OctreeNode*> childern;

	std::vector<unsigned int> surfaces;

	OctreeNode* parent;


	float max_z;	// 包含的surface的最大值

	//左下角和长宽高?

	OctreeNode() {
		childern.assign(std::vector<OctreeNode*>(8, nullptr));
	}

};

class Octree
{
public:
	Octree() {};
	~Octree() {};

	// 初始化octree
	void build(unsigned int scr_width, unsigned int scr_height, unsigned int scr_depth, std::set<int>& surfaces) {

	};

	OctreeNode* getOctreeNode() {};


private:

	OctreeNode* buildOctree(unsigned int x_l, unsigned int x_r, unsigned int y_b, unsigned int y_t, unsigned int z_f, unsigned int z_b, std::set<unsigned int>& surfaces, OctreeNode* parent) {
		if (x_l > x_r || y_b > y_t || z_b > z_f || surfaces.empty()) {
			return nullptr;
		}

		OctreeNode* node = new OctreeNode();
		node->parent = parent;

		if (surfaces.size() <= threshold_) {
			node->surfaces.assign(surfaces.begin(), surfaces.end());
			return node;
		}


		unsigned int mid_x = (x_l + x_r) >> 1;
		unsigned int mid_y = (y_b + y_t) >> 1;
		unsigned int mid_z = (z_b + z_f) >> 1;

		
		for () {
			;
		}


		return nullptr;
	}

	void initOctreeNodeZ(OctreeNode* node) {
	};

	OctreeNode* root_;
	
	unsigned int threshold_ = 5;	// 

	unsigned int scr_width_ = -1;
	unsigned int scr_heignt_ = -1;
	unsigned int scr_depth_ = -1;

};