#pragma once
#include<iostream>
#include<vector>
class ZBuffer {
public:

	ZBuffer(int width, int height) {
		win_width_ = width;
		win_height_ = height;

		// malloc memeroy
		data = (GLubyte*)malloc(static_cast<size_t>(width) * static_cast<size_t>(height) * 3 * sizeof(GLubyte));

		if (data == nullptr) {
			std::cout << "ERROR::ZBuffer:: malloc memory failed." << std::endl;
			exit(0);
		}
		else {
			memset(data, 0, static_cast<size_t>(win_width_) * static_cast<size_t>(win_height_) * 3 * sizeof(GLubyte));
		}
	};

	void setPixel(unsigned int x, unsigned y, std::vector<GLubyte > color) {
		if (x >= win_width_ || y >= win_height_) {
			std::cout << "ERROR::ZBuffer::SetPixel (" << x << "," << y << ") is out of rangle of (" << win_width_ << "," << win_height_ << ")." << std::endl;
			return;
		}
		if (color.size() < 3) {
			std::cout << "ERROR::ZBuffer::SetPixel parameter:color.size() is too small which is " << color.size() << std::endl;
			return;
		}
		data[3 * (y * win_width_ + x) + 0] = color[0];
		data[3 * (y * win_width_ + x) + 1] = color[1];
		data[3 * (y * win_width_ + x) + 2] = color[2];
	}

	GLubyte* getData() {
		return data;
	}
private:
	GLubyte* data;
	unsigned int w;
	unsigned int h;

	int win_width_;
	int win_height_;
};