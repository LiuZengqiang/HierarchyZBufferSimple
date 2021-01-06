#pragma once
#include"Point.h"
#include"glm/glm.hpp"
#include<vector>


#define Epsilon 0.00001f
#define Pi 3.14159265359

namespace global {
	
	// float 类型是否相等
	bool floatEqual(const float& val_1, const float& val_2) {
		return std::abs(val_1 - val_2) < Epsilon;
	}
	
	// double 类型是否相等
	bool doubleEqual(const double& val_1, const double& val_2) {
		return std::abs(val_1 - val_2) < Epsilon;
	}
	
	// vec3 类型是否相等
	bool vec3Equal(const glm::vec3& val_1, const glm::vec3& val_2) {
		return floatEqual(val_1.x, val_2.x) && floatEqual(val_1.y, val_2.y) && floatEqual(val_1.z, val_2.z);
	}

	// 像素点是否在线段上
	bool pointOnSegmentLine(const glm::ivec2& point, const glm::ivec2 seg_point_1, glm::ivec2 seg_point_2) {

		// 点是否是端点
		if (point == seg_point_1 || point == seg_point_2) {
			return true;
		}

		if (seg_point_1.x == seg_point_2.x) {
			return (point.x == seg_point_1.x) && (point.y <= std::max(seg_point_1.y, seg_point_2.y)) && (point.y >= std::min(seg_point_1.y, seg_point_2.y));
		}

		if (seg_point_1.y == seg_point_2.y) {
			return (point.y == seg_point_1.y) && (point.x <= std::max(seg_point_1.x, seg_point_2.x)) && (point.x >= std::min(seg_point_1.x, seg_point_2.x));
		}

		if (point.x > std::max(seg_point_1.x, seg_point_2.x) || point.x<std::min(seg_point_1.x, seg_point_2.x) || point.y>std::max(seg_point_1.y, seg_point_2.y) || point.y < std::min(seg_point_1.y, seg_point_2.y)) {
			return false;
		}

		// 求距离h
		glm::vec3 p((float)(point.x) + 0.5, (float)(point.y) + 0.5, 0.0);
		glm::vec3 s_p_1((float)(seg_point_1.x) + 0.5, (float)(seg_point_1.y) + 0.5, 0.0);
		glm::vec3 s_p_2((float)(seg_point_2.x) + 0.5, (float)(seg_point_2.y) + 0.5, 0.0);

		if (vec3Equal(s_p_1, s_p_2)) {
			std::cout << "s_p_1 equal s_p_2" << std::endl;
			return vec3Equal(s_p_1, p);
		}
		glm::vec3 PA = s_p_1 - p;
		glm::vec3 PB = s_p_2 - p;
		float S = glm::length(glm::cross(PA, PB));
		float h = S / glm::length(s_p_2 - s_p_1);
		return h < Epsilon;
	}
	
	// 像素点向x正方向发出的射线是否和线段相交，相交返回交点，不相交返回ivec2(INT_MIN, INT_MIN)
	glm::ivec2 pointIntersectLine(const glm::ivec2& point, const glm::ivec2& seg_point_1, const glm::ivec2& seg_point_2) {

		glm::ivec2 ret(INT_MIN, INT_MIN);

		// 如果点在线上
		if (pointOnSegmentLine(point, seg_point_1, seg_point_2)) {
			ret.x = point.x;
			ret.y = point.y;
			return ret;
		}

		// 显然不与线段相交
		if (point.y<std::min(seg_point_1.y, seg_point_2.y) || point.y>std::max(seg_point_1.y, seg_point_2.y)) {
			return ret;
		}

		// 如果线段是一个点
		if (seg_point_1 == seg_point_2) {
			if (point == seg_point_1) {
				ret.x = point.x;
				ret.y = point.y;
			}
			return ret;
		}

		if (seg_point_1.x == seg_point_2.x) {
			if (point.x <= seg_point_1.x) {
				ret.x = seg_point_1.x;
				ret.y = point.y;
				return ret;
			}
			else {
				return ret;
			}
		}

		if (seg_point_1.y == seg_point_2.y) {
			if (point.x <= std::min(seg_point_1.x, seg_point_2.x)) {
				ret.x = seg_point_2.x;
				ret.y = seg_point_2.y;
				return ret;
			}
			else {
				return ret;
			}
		}

		glm::vec2 p((float)(point.x) + 0.5f, (float)(point.y) + 0.5f);
		glm::vec2 s_p_1((float)(seg_point_1.x) + 0.5f, (float)(seg_point_1.y) + 0.5f);
		glm::vec2 s_p_2((float)(seg_point_2.x) + 0.5f, (float)(seg_point_2.y) + 0.5f);

		float ret_x = (s_p_2.x - s_p_1.x) / (s_p_2.y - s_p_1.y) * (p.y - s_p_1.y) + s_p_1.x;

		if (ret_x < p.x) {
			return ret;
		}
		ret.x = (int)std::floor(ret_x);
		ret.y = point.y;
		return ret;
	}

	// 像素点是否在多边形上(里)
	bool pointInPolygon(const glm::ivec2& point, const std::vector<glm::ivec2 >& po_points) {

		if (po_points.size() < 3) {
			std::cout << "ERROR::globalFunction::point2DinPolygon2d The po_points size() smaller than 3." << std::endl;
			return false;
		}

		glm::ivec2 pre = po_points.back();
		glm::ivec2 now;
		// 点在线上
		for (unsigned int i = 0; i < po_points.size(); i++) {
			now = po_points[i];
			if (pointOnSegmentLine(point, pre, now)) {
				return true;
			}
			pre = now;
		}

		// 求交点个数
		unsigned int cnt = 0;
		pre = po_points.back();

		for (unsigned int i = 0; i < po_points.size(); i++) {

			now = po_points[i];

			glm::ivec2 ret = pointIntersectLine(point, pre, now);

			if (ret.x==INT_MIN) {
				;
			}
			else {
				if (point.y == std::min(now.y, pre.y)) {
					;
				}
				else
				{
					cnt++;
				}
			
			}
			pre = now;
			continue;


			if (ret.x == INT_MIN) {
				;
			}
			else {
				if (ret == now) {

					if (now.y == pre.y) {
						glm::ivec2 pre_pre;
						size_t pre_pre_index = i + po_points.size() - 1;

						while (po_points[pre_pre_index % po_points.size()].y == now.y) {
							pre_pre_index--;
						}
						pre_pre = po_points[pre_pre_index % po_points.size()];
						if (pre_pre.y == now.y) {
							std::cout << "ERROR::global pre_pre.y equal now.y ." << std::endl;
							std::cout << "\t now:(" << now.x << "," << now.y << " pre_pre:(" << pre_pre.x << "," << pre_pre.y << ")" << std::endl;
							std::cout << "\t po_points:";
							for (auto it : po_points) {
								std::cout << "(" << it.x << "," << it.y << ") ";
							}
							std::cout << std::endl;
							exit(0);
						}
						glm::ivec2 next = po_points[((size_t)i + 1) % po_points.size()];

						if ((pre_pre.y - now.y) * (next.y - now.y) >= 0) {
							;
						}
						else {
							cnt++;
						}
					}
					else if (now.x == pre.x) {	// 如果是竖直的线
						;
					}
					else {
						cnt++;
					}

					//// pre = pew, now = now
					//glm::ivec2 next = po_points[((size_t)i + 1) % po_points.size()];
					//
					//if (point.x == 0 && point.y == 359 && po_points[1].x == 6) {
					//	std::cout << "ret" << ret.x << " " << ret.y << std::endl;
					//}

					//
					//if ((next.y - now.y) * (pre.y - now.y) > 0) {
					//	cnt++;
					//	if (point.x == 0 && point.y == 359 && po_points[1].x == 6) {
					//		std::cout << "++" << std::endl;
					//	}
					//}
					//else { // 在异侧 +0
					//	;
					//}
				}
				else {
					cnt++;
				}
			}
			pre = now;
		}
		return cnt % 2;
	}

	// 坐标点z负方向发出的射线与多边形的交点，返回交点的z值
	float interSectZ(glm::vec3 origin, glm::vec3 a, glm::vec3 b, glm::vec3 c) {

		glm::vec3 E1 = b - a;
		glm::vec3 E2 = c - a;
		glm::vec3 p_vec = glm::cross(glm::vec3(0.0f, 0.0f, -1.0f), E2);

		float det = glm::dot(E1, p_vec);
		if (floatEqual(det, 0.0)) {
			//std::cout << "ERROR::global::interSect Polygon parallel direction." << std::endl;
			return -FLT_MAX;
		}

		glm::vec3 nor = glm::cross(E1, E2);

		// 点法式方程
		// nor.x(X-Xa) + nor.y(Y-Ya) + nor.z(Z-Za) = 0
		if (floatEqual(nor.z, 0.0)) {
			//std::cout << "ERROR::global::interSect Normal.z = 0.0;" << std::endl;
			return -FLT_MAX;
		}
		float z = -1.0f * (nor.x * (origin.x - a.x) + nor.y * (origin.y - a.y)) / nor.z + a.z;
		return z;
	}
}