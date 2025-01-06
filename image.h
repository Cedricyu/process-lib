#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <cstdint>
#include <string>

class Image {
private:
    int width;                // 影像寬度
    int height;               // 影像高度
    int channels;             // 通道數 (1: 灰階, 3: RGB, 4: RGBA)
    std::vector<uint8_t> data; // 影像數據

public:
    // 構造函數
    Image(int w, int h, int c);
    Image(const std::vector<uint8_t>& rawData, int w, int h, int c);

    // 從 JPEG 文件加載影像
    static Image loadFromJPG(const std::string& filename);

    // 保存影像為 JPEG
    void saveAsJPG(const std::string& filename, int quality = 90) const;

    // 基本信息
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getChannels() const { return channels; }
    const std::vector<uint8_t>& getData() const { return data; }

    // 數據訪問
    uint8_t& at(int x, int y, int channel);
    const uint8_t& at(int x, int y, int channel) const;
};

#endif // IMAGE_H
