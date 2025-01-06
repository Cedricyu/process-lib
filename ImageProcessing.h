#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "Image.h"

// 灰階轉換
Image applyGrayscale(const Image& img);

// 應用模糊
Image applyBlur(const Image& img, int radius);

// 顏色反轉
Image applyInvertColors(const Image& img);

// 調整亮度
Image applyBrightness(const Image& img, int brightness);

#endif // IMAGE_PROCESSING_H
