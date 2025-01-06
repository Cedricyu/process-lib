#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Image.h"
#include <stdexcept>
#include <iostream>

// 構造函數
Image::Image(int w, int h, int c) : width(w), height(h), channels(c) {
    if (w <= 0 || h <= 0 || (c != 1 && c != 3 && c != 4)) {
        throw std::invalid_argument("Invalid image dimensions or channels.");
    }
    data.resize(w * h * c);
}

Image::Image(const std::vector<uint8_t>& rawData, int w, int h, int c)
    : width(w), height(h), channels(c), data(rawData) {
    if (rawData.size() != w * h * c) {
        throw std::invalid_argument("Raw data size does not match dimensions.");
    }
}

// 從 JPEG 文件加載影像
Image Image::loadFromJPG(const std::string& filename) {
    int w, h, c;
    uint8_t* imgData = stbi_load(filename.c_str(), &w, &h, &c, 0);
    if (!imgData) {
        throw std::runtime_error("Failed to load image: " + filename);
    }

    // 將數據複製到 std::vector
    std::vector<uint8_t> data(imgData, imgData + w * h * c);
    stbi_image_free(imgData);

    return Image(data, w, h, c);
}

// 保存影像為 JPEG
void Image::saveAsJPG(const std::string& filename, int quality) const {
    if (!stbi_write_jpg(filename.c_str(), width, height, channels, data.data(), quality)) {
        throw std::runtime_error("Failed to save image as JPEG: " + filename);
    }
    std::cout << "Image saved as " << filename << " with quality " << quality << "." << std::endl;
}

// 數據訪問
uint8_t& Image::at(int x, int y, int channel) {
    if (x < 0 || x >= width || y < 0 || y >= height || channel < 0 || channel >= channels) {
        throw std::out_of_range("Pixel access out of bounds.");
    }
    return data[(y * width + x) * channels + channel];
}

const uint8_t& Image::at(int x, int y, int channel) const {
    if (x < 0 || x >= width || y < 0 || y >= height || channel < 0 || channel >= channels) {
        throw std::out_of_range("Pixel access out of bounds.");
    }
    return data[(y * width + x) * channels + channel];
}
