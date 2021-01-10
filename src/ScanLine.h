//本文尝试回答，马克思再共产党宣言中到底再宣什么？

// TODO::FIX!!!
#pragma once
#include"Model.h"
#include"glm\glm.hpp"
#include<vector>
#include<iostream>
#include<tuple>
#include<set>

// 活化边表
struct ActiveEdgeNode
{
	float x = -1;	// 虽然使float类型，但是是像素值
	float dx = 0.0f;	// d像素值
	int dy = -1;		// 像素值

	float z = FLT_MAX;	// 坐标值
	float dzx = FLT_MAX;
	float dzy = FLT_MAX;
	unsigned int id = -1;
	
	void debug() {
		std::cout << "x:" << x << " dx" << dx << "dy" << dy << ";id" << id << std::endl;
	}
};

// 多边形表
struct PolygonNode
{
	// a*x+b*y+c*z+d=0
	float a = 0.0f;
	float b = 0.0f;
	float c = 0.0f;
	float d = 0.f;
	/* surface id*/
	unsigned int id = -1;
	/* max_y - min_y + 1*/
	int dy = 0;
	void debug() {
		std::cout << "id:" << id << std::endl;
	}
	std::vector<ActiveEdgeNode*> active_edge_table;

};
// 边表
struct EdgeNode
{
	float x = -1.0f;
	float dx = 0.0f;
	int dy = -1;
	unsigned int id = -1;
	void debug() {
		std::cout << "x:" << x << " dx:" << dx << " dy:" << dy << " id:" << id << std::endl;
	}
};


class ScanLine
{
public:
	ScanLine(unsigned int scr_width, unsigned int scr_height, Model* model, GLubyte* color_data) {
		scr_width_ = scr_width;
		scr_height_ = scr_height;
		model_ = model;
		color_data_ = color_data;
	};
	~ScanLine() {

		for (int i = 0; i < (int)polygon_table_.size(); i++) {
			for (int j = 0; j < (int)polygon_table_[i].size(); j++) {
				if (polygon_table_[i][j]!=nullptr) {
					delete polygon_table_[i][j];
				}
			}
		}
		
		for (int i = 0; i < (int)edge_table_.size(); i++) {
			for (int j = 0; j < (int)edge_table_[i].size(); j++) {
				if (edge_table_[i][j]!=nullptr) {
					delete edge_table_[i][j];
				}
			}
		}
	};

	void debugPolygon() {
		for (int i = 0; i < (int)scr_height_; i++) {
			if (polygon_table_[i].empty()) {
				;
			}
			else {

				std::cout << "y:" << i << " " << polygon_table_[i].size() << std::endl;

				std::cout << "polygon:" << std::endl;

				for (int j = 0; j < polygon_table_[i].size(); j++) {
					float z = -(polygon_table_[i][j]->d + polygon_table_[i][j]->a * 0 + polygon_table_[i][j]->b * 1.0f) / polygon_table_[i][j]->c;
					std::cout << "\t" << polygon_table_[i][j]->id << " z:" << z << std::endl;
				}
				std::cout << std::endl;
			}
		}
	}

	void debugEdge() {

		for (int i = 0; i < (int)scr_height_; i++) {
			if (edge_table_[i].empty()) {
				continue;
			}
			std::cout << "y:" << i << std::endl;
			for (int j = 0; j < edge_table_[i].size(); j++) {
				std::cout << "\t" << "x:" << edge_table_[i][j]->x << "dx" << edge_table_[i][j]->dx << " dy" << edge_table_[i][j]->dy << " id" << edge_table_[i][j]->id << std::endl;
			}
			std::cout << std::endl;
		}
	}

	/* init */
	void init() {

		std::cout << "HINT::Scanline init." << std::endl;
		// 初始化z_buffer
		z_buffer_ = (float*)malloc(scr_width_ * sizeof(float));
		if (z_buffer_ == nullptr) {
			std::cout << "ERROR::Scanline::init z_buffer failed." << std::endl;
			exit(0);
		}

		// 建立分类多边形表
		polygon_table_.clear();
		polygon_table_.resize(scr_height_);
		edge_table_.clear();
		edge_table_.resize(scr_height_);

		int surface_size = (int)model_->sur_faces_.size();

		for (int i = 0; i < surface_size; i++) {

			// 构建分类边表
			int surface_indice_size = (int)model_->sur_faces_[i].indices.size();
			for (int j = 0; j < surface_indice_size; j++) {

				glm::vec3 p1 = model_->points_[model_->sur_faces_[i].indices[j]].position;

				glm::vec3 p2 = (j == model_->sur_faces_[i].indices.size() - 1) ? model_->points_[model_->sur_faces_[i].indices[0]].position : model_->points_[model_->sur_faces_[i].indices[j + 1]].position;

				// 使p2高于p1
				if (p1.y > p2.y) {
					glm::vec3 temp_p = p1;
					p1 = p2;
					p2 = temp_p;
				}

				// 新的边
				EdgeNode* e_node = new EdgeNode();
				// 新边的y坐标
				//int p_y_min = (int)std::round((1 + p1.y) / 2.0f * (scr_height_ - 1));
				//int p_y_max = (int)std::round((1 + p2.y) / 2.0f * (scr_height_ - 1));

				int p_y_min = global::coord2PixelY(p1.y);
				int p_y_max = global::coord2PixelY(p2.y);

				e_node->dy = p_y_max - p_y_min;

				// 无效边(水平边)
				if (e_node->dy <= 0) {
					delete e_node;
					continue;
				}

				std::pair<float, float> p_p_1(global::coord2PixelX(p1.x), global::coord2PixelY(p1.y));
				std::pair<float, float> p_p_2(global::coord2PixelX(p2.x), global::coord2PixelY(p2.y));

				e_node->x = p_p_2.first;

				e_node->id = i;

				e_node->dx = -(p_p_2.first - p_p_1.first) / (p_p_2.second - p_p_1.second);

				if (global::floatEqual(e_node->dx, 0.0f)) {
					e_node->dx = 0.0f;
				}
				edge_table_[p_y_max].emplace_back(e_node);
			}

			// 新多边形
			PolygonNode* p_node = new PolygonNode();

			glm::vec3 p0 = model_->points_[model_->sur_faces_[i].indices[0]].position;
			glm::vec3 p1 = model_->points_[model_->sur_faces_[i].indices[1]].position;
			glm::vec3 p2 = model_->points_[model_->sur_faces_[i].indices[2]].position;

			glm::vec3 p_p0(global::coord2PixelX(p0.x), global::coord2PixelY(p0.y), p0.z);
			glm::vec3 p_p1(global::coord2PixelX(p1.x), global::coord2PixelY(p1.y), p1.z);
			glm::vec3 p_p2(global::coord2PixelX(p2.x), global::coord2PixelY(p2.y), p2.z);

			p_p1 = p_p1 - p_p0;
			p_p2 = p_p2 - p_p0;

			glm::vec3 nor = glm::cross(p_p1, p_p2);

			p_node->a = nor.x;
			p_node->b = nor.y;
			p_node->c = nor.z;

			p_node->d = -(p_node->a * p_p0.x + p_node->b * p_p0.y + p_node->c * p_p0.z);

			p_node->id = i;

			float min_y, max_y;
			std::tie(min_y, max_y) = model_->getMinMaxY(i);

			int p_y_min = global::coord2PixelY(min_y);
			int p_y_max = global::coord2PixelY(max_y);
			p_node->dy = (p_y_max - p_y_min);
			polygon_table_[p_y_max].emplace_back(p_node);
		}
	}

	/* Render */
	void beginRender() {
		std::cout << "HINT::Scanline beginRender." << std::endl;

		active_polygon_table_.clear();

		// 扫描线
		for (int y = scr_height_ - 1; y >= 0; y--) {

			// 初始化z_buffer为最小值
			std::fill(z_buffer_, z_buffer_ + scr_width_, -FLT_MAX);

			// 遍历此时(y)的的分类多边形
			// 添加新的多边形
			int poly_table_y_size = (int)polygon_table_[y].size();
			for (int i = 0; i < poly_table_y_size; i++) {
				active_polygon_table_.emplace_back(polygon_table_[y][i]);
			}

			// 此时active_polygon_table_中的存了此时的 活化多边形表
			int act_poly_table_size = (int)active_polygon_table_.size();

			// 对每一个活化多边形
			//std::cout << "polygon size:" << cnt_act_poly_table << std::endl;
			for (int i = 0; i < act_poly_table_size; i++) {

				// 每个活化多边形
				//    活化多边形中存的是活化边
				PolygonNode* active_polygon = active_polygon_table_[i];

				// 更新每个活化多边形中的活化边
				updateActiveEdge(y, active_polygon);

				// 活化多变形中的活化边
				std::vector<ActiveEdgeNode*>& active_edge_table = active_polygon->active_edge_table;

				// 遍历每对 活化边对
				int act_edge_table_size = ((int)active_edge_table.size()) - 1;
				for (int j = 0; j < act_edge_table_size; j += 2) {

					ActiveEdgeNode* edge1 = active_edge_table[j];

					ActiveEdgeNode* edge2 = active_edge_table[j + 1];

					float zx = edge1->z;
					for (int x = (int)edge1->x; x <= (int)edge2->x; x++) {
						if (zx > z_buffer_[x] + Epsilon) {
							z_buffer_[x] = zx;
							setPixel(x, y, model_->color_[active_polygon->id]);
						}
						zx += edge1->dzx;
					}

					edge1->dy--;
					edge2->dy--;

					edge1->x += edge1->dx;
					edge2->x += edge2->dx;

					edge1->z += (edge1->dzx * edge1->dx + edge1->dzy);
					edge2->z += (edge2->dzx * edge2->dx + edge2->dzy);
				}


				// 删除无效的边
				int last = 0;
				int cnt_active_edge = (int)active_edge_table.size();
				try {
					for (int j = 0; j < cnt_active_edge; j++, last++) {

						while (active_edge_table[j]->dy <= 0)
						{

							delete active_edge_table[j];

							j++;
							if (j >= cnt_active_edge) {
								break;
							}
						}

						if (j >= cnt_active_edge) {
							break;
						}
						active_edge_table[last] = active_edge_table[j];
					}
				}
				catch (const char* msg) {
					std::cout << "ERROR:1" << msg << std::endl;
					exit(0);
				}
				active_edge_table.resize(last);
				active_polygon->dy--;
			}


			// 删除已经完成的活化多边形
			int last = 0;
			//int cnt_act_polygon = active_polygon_table_.size();
			try {
				for (int i = 0; i < act_poly_table_size; i++, last++) {
					while (active_polygon_table_[i]->dy <= 0)
					{
						delete active_polygon_table_[i];
						i++;
						if (i >= act_poly_table_size) {
							break;
						}
					}
					if (i >= act_poly_table_size) {
						break;
					}
					active_polygon_table_[last] = active_polygon_table_[i];
				}
				active_polygon_table_.resize(last);
			}
			catch (const char* msg) {
				std::cout << "ERROR:2" << msg << std::endl;
				exit(0);
			}

		}

	};

private:

	static bool cmp(const ActiveEdgeNode* a, const ActiveEdgeNode* b) {
		return global::floatEqual(a->x, b->x) ? (a->dx < b->dx) : (a->x < b->x);
	}
	/* update active edge in polygon */
	void updateActiveEdge(unsigned int y, PolygonNode* polygon) {

		for (int i = 0; i < edge_table_[y].size(); i++) {

			EdgeNode* e = edge_table_[y][i];

			if (e->id != polygon->id) {
				continue;
			}
			else {
				ActiveEdgeNode* active_edge = new ActiveEdgeNode();

				active_edge->x = e->x;

				active_edge->dx = e->dx;
				active_edge->dy = e->dy;

				active_edge->id = polygon->id;

				if (global::floatEqual(polygon->c, 0.0f)) {		// 法向平行xoy平面的surface
					delete active_edge;
					continue;
					/*active_edge->z = model_->getMaxZ(polygon->id);
					active_edge->dzx = 0.0f;
					active_edge->dzy = 0.0f;*/
				}
				else {
					active_edge->z = -(polygon->d + polygon->a * (e->x) + polygon->b * y) / polygon->c;
					//std::cout << "z:" << active_edge->z << std::endl;
					active_edge->dzx = -(polygon->a / polygon->c);
					active_edge->dzy = polygon->b / polygon->c;
				}
				polygon->active_edge_table.push_back(active_edge);
				// ?
				//e->id = -1;
			}
		}

		if (polygon->active_edge_table.empty()) {
			;
		}
		else {
			std::sort(polygon->active_edge_table.begin(), polygon->active_edge_table.end(), cmp);
		}
	}
	/* Set pixel color */
	void setPixel(unsigned int x, unsigned int y, float color) {
		if (x >= scr_width_ || y >= scr_height_ || x < 0 || y < 0) {
			std::cout << "ERROR::Scanline Out of range setPixel:(" << x << "," << y << ")" << std::endl;
			exit(0);
		}
		color_data_[3 * (y * scr_width_ + x) + 0] = (GLubyte)(color * 255);
		color_data_[3 * (y * scr_width_ + x) + 1] = (GLubyte)(color * 255);
		color_data_[3 * (y * scr_width_ + x) + 2] = (GLubyte)(color * 255);
	}

	/* model */
	Model* model_ = nullptr;

	/* color data of windows */
	GLubyte* color_data_ = nullptr;

	/* depth z buffer */
	float* z_buffer_ = nullptr;

	unsigned int scr_width_;
	unsigned int scr_height_;

	/* polygon table */
	std::vector<std::vector<PolygonNode*> > polygon_table_;

	/* edge table */
	std::vector<std::vector<EdgeNode*> > edge_table_;

	/* active polygon table */
	std::vector<PolygonNode* > active_polygon_table_;
};