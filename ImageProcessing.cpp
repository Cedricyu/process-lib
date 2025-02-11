#include "ImageProcessing.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cstdint>
#include <omp.h>


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

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(srcData.size()); i++) {
        int adjusted = static_cast<int>(srcData[i]) + brightness;
        destData[i] = (adjusted < 0) ? 0 : (adjusted > 255 ? 255 : adjusted); // 手動 clamp
    }
    return result;
}

Image applyContrast(const Image& img, float contrast) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(srcData.size()); i++) {
        int adjusted = static_cast<int>(128 + (srcData[i] - 128) * contrast);
        destData[i] = (adjusted < 0) ? 0 : (adjusted > 255 ? 255 : adjusted); // 手動 clamp
    }
    return result;
}

Image applySaturation(const Image& img, float saturation) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    if (img.getChannels() < 3) {
        return img; // 若圖片不是 RGB，則不處理飽和度
    }

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(srcData.size()); i += img.getChannels()) {
        float r = srcData[i] / 255.0f;
        float g = srcData[i + 1] / 255.0f;
        float b = srcData[i + 2] / 255.0f;

        float gray = 0.299f * r + 0.587f * g + 0.114f * b;

        r = std::clamp(gray + (r - gray) * saturation, 0.0f, 1.0f);
        g = std::clamp(gray + (g - gray) * saturation, 0.0f, 1.0f);
        b = std::clamp(gray + (b - gray) * saturation, 0.0f, 1.0f);

        destData[i] = static_cast<uint8_t>(r * 255);
        destData[i + 1] = static_cast<uint8_t>(g * 255);
        destData[i + 2] = static_cast<uint8_t>(b * 255);
    }
    return result;
}

Image applyColorTemperature(const Image& img, int temperature) {
    Image result(img.getWidth(), img.getHeight(), img.getChannels());
    const auto& srcData = img.getData();
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());

    if (img.getChannels() < 3) {
        return img; // 若圖片不是 RGB，則不處理色溫
    }

    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(srcData.size()); i += img.getChannels()) {
        int r = srcData[i];
        int g = srcData[i + 1];
        int b = srcData[i + 2];

        r = std::clamp(r + temperature, 0, 255);
        b = std::clamp(b - temperature, 0, 255);

        destData[i] = static_cast<uint8_t>(r);
        destData[i + 1] = static_cast<uint8_t>(g);
        destData[i + 2] = static_cast<uint8_t>(b);
    }
    return result;
}

Image applyProjection(const Image& panorama, double R, float scaleFactor) {
    const int width = panorama.getWidth();
    const int height = panorama.getHeight();
    const int channels = panorama.getChannels();

    const float aspectRatio = 9.0f / 1.0f; // 目標比例 9:1
    const int newHeight = height; // 保持高度不變
    const int newWidth = static_cast<int>(newHeight * aspectRatio); // 動態計算寬度

    const int gridRows = 100; // 網格行數
    const int gridCols = 100; // 網格列數

    const int gridWidth = newWidth / gridCols;
    const int gridHeight = newHeight / gridRows;

    const float cx = width / 2.0f;
    const float cy = height / 2.0f;

    // 加載遮罩圖像
    Image mask = Image::loadFromJPG("paranoma_mask.JPG");
    const auto& maskData = mask.getData();
    if (mask.getWidth() != width || mask.getHeight() != height) {
        throw std::runtime_error("Mask size does not match panorama size");
    }

    // 初始化網格頂點
    std::vector<std::vector<std::pair<float, float>>> grid(gridRows + 1, std::vector<std::pair<float, float>>(gridCols + 1));
    for (int row = 0; row <= gridRows; row++) {
        for (int col = 0; col <= gridCols; col++) {
            grid[row][col] = { col * gridWidth, row * gridHeight };
        }
    }

    // 定義每個網格的重要性
    std::vector<std::vector<float>> gridImportance(gridRows + 1, std::vector<float>(gridCols + 1, 1.0f));
    for (int row = 0; row <= gridRows; row++) {
        for (int col = 0; col <= gridCols; col++) {
            int maskX = static_cast<int>((col * gridWidth) * (static_cast<float>(width) / newWidth));
            int maskY = static_cast<int>((row * gridHeight) * (static_cast<float>(height) / newHeight));

            // 確保索引在範圍內
            if (maskX >= 0 && maskY >= 0 && maskX < width && maskY < height) {
                int maskIndex = (maskY * width + maskX) * mask.getChannels();

                // 將 RGB 遮罩轉為灰度值
                float grayValue = 0.2989f * maskData[maskIndex] +    // R 通道
                    0.5870f * maskData[maskIndex + 1] + // G 通道
                    0.1140f * maskData[maskIndex + 2];  // B 通道

                // 如果灰度值超過閾值，則設置為高重要性
                if (grayValue > 128) {
                    gridImportance[row][col] = 1.2f; // 高重要性區域，變形影響較小
                }
            }
        }
    }

    for (int row = 1; row < gridRows; row++) {
        for (int col = 1; col < gridCols; col++) {
            gridImportance[row][col] = (
                gridImportance[row - 1][col] +
                gridImportance[row + 1][col] +
                gridImportance[row][col - 1] +
                gridImportance[row][col + 1] +
                gridImportance[row][col]
                ) / 5.0f;  // 計算鄰近 5 點的平均值
        }
    }

    // 計算每個頂點的變形位置
    for (int row = 0; row <= gridRows; row++) {
        for (int col = 0; col <= gridCols; col++) {
            float x = grid[row][col].first * (static_cast<float>(width) / newWidth); // 按原圖比例縮放
            float y = grid[row][col].second;

            float dx = (x - cx) / width;
            float dy = (y - cy) / height;
            float r = std::sqrt(dx * dx + dy * dy);

            // 限制 r 的範圍，避免數值過大或過小
            float rMax = 0.5f; // 最大半徑
            float rMin = 0.01f; // 最小半徑
            r = std::clamp(r, rMin, rMax);

            // 修正 scale 的計算公式，加入 epsilon 防止分母為零
            float epsilon = 0.01f;
            float scale = std::log(1.0f + r) / (r + epsilon);

            // 按重要性調整縮放比例
            scale *= gridImportance[row][col];

            // 計算變形後的頂點位置，並進行範圍限制
            grid[row][col].first = std::clamp(cx + scale * dx * width, 0.0f, (float)(width - 1));
            grid[row][col].second = std::clamp(cy + scale * dy * height, 0.0f, (float)(height - 1));
        }
    }

    // 應用變形並生成輸出圖像
    Image result(newWidth, newHeight, channels);
    auto& destData = const_cast<std::vector<uint8_t>&>(result.getData());
    const auto& srcData = panorama.getData();

    #pragma omp parallel for
    for (int row = 0; row < gridRows; row++) {
        for (int col = 0; col < gridCols; col++) {
            auto topLeft = grid[row][col];
            auto topRight = grid[row][col + 1];
            auto bottomLeft = grid[row + 1][col];
            auto bottomRight = grid[row + 1][col + 1];

            for (int y = row * gridHeight; y < (row + 1) * gridHeight; y++) {
                for (int x = col * gridWidth; x < (col + 1) * gridWidth; x++) {
                    float alphaX = (x - col * gridWidth) / (float)gridWidth;
                    float alphaY = (y - row * gridHeight) / (float)gridHeight;

                    float warpedX = topLeft.first * (1 - alphaX) * (1 - alphaY) +
                        topRight.first * alphaX * (1 - alphaY) +
                        bottomLeft.first * (1 - alphaX) * alphaY +
                        bottomRight.first * alphaX * alphaY;

                    float warpedY = topLeft.second * (1 - alphaX) * (1 - alphaY) +
                        topRight.second * alphaX * (1 - alphaY) +
                        bottomLeft.second * (1 - alphaX) * alphaY +
                        bottomRight.second * alphaX * alphaY;

                    warpedX = std::clamp(warpedX, 0.0f, (float)(width - 1));
                    warpedY = std::clamp(warpedY, 0.0f, (float)(height - 1));

                    int srcX = static_cast<int>(warpedX);
                    int srcY = static_cast<int>(warpedY);

                    for (int c = 0; c < channels; c++) {
                        destData[(y * newWidth + x) * channels + c] =
                            srcData[(srcY * width + srcX) * channels + c];
                    }
                }
            }
        }
    }

    return result;
}

Image processImage(const Image& img, int brightness, float contrast, float saturation, int temperature) {
    Image result = applyBrightness(img, brightness);
    result = applyContrast(result, contrast);
    result = applySaturation(result, saturation);
    result = applyColorTemperature(result, temperature);
    return result;
}