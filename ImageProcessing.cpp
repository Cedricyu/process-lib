#include "ImageProcessing.h"
#include <vector>
#include <cmath>
#include <algorithm>

// 灰階轉換
Image applyGrayscale(const Image& img) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    if (img.getChannels() < 3) return img; // 不處理少於 3 通道的影像

    for (int i = 0; i < img.getWidth() * img.getHeight(); i++) {
        uint8_t r = srcData[i * img.getChannels()];
        uint8_t g = srcData[i * img.getChannels() + 1];
        uint8_t b = srcData[i * img.getChannels() + 2];
        uint8_t gray = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);

        destData[i * img.getChannels()] = gray;
        destData[i * img.getChannels() + 1] = gray;
        destData[i * img.getChannels() + 2] = gray;

        if (img.getChannels() == 4) { // 保持 alpha 通道
            destData[i * img.getChannels() + 3] = srcData[i * img.getChannels() + 3];
        }
    }
    return result;
}

// 模糊處理
Image applyBlur(const Image& img, int radius) {
    if (radius <= 0) return img;

    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    int size = 2 * radius + 1;

    for (int y = 0; y < img.getHeight(); y++) {
        for (int x = 0; x < img.getWidth(); x++) {
            for (int c = 0; c < img.getChannels(); c++) {
                int sum = 0, count = 0;

                // 平均周圍像素
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        int nx = std::clamp(x + kx, 0, img.getWidth() - 1);
                        int ny = std::clamp(y + ky, 0, img.getHeight() - 1);
                        sum += srcData[(ny * img.getWidth() + nx) * img.getChannels() + c];
                        count++;
                    }
                }

                destData[(y * img.getWidth() + x) * img.getChannels() + c] = sum / count;
            }
        }
    }
    return result;
}

// 顏色反轉
Image applyInvertColors(const Image& img) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    for (size_t i = 0; i < srcData.size(); i++) {
        destData[i] = 255 - srcData[i];
    }
    return result;
}

// 亮度調整
Image applyBrightness(const Image& img, int brightness) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    for (size_t i = 0; i < srcData.size(); i++) {
        int adjusted = static_cast<int>(srcData[i]) + brightness;
        destData[i] = static_cast<uint8_t>(std::clamp(adjusted, 0, 255));
    }
    return result;
}
