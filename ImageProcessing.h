#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#include "Image.h"

// 灰階轉換
Image applyGrayscale(const Image& img);

// 應用模糊
Image applyBlur(const Image& img, int radius);

// 顏色反轉
Image applyInvertColors(const Image& img);

// 調整亮度
Image applyBrightness(const Image& img, int brightness);
Image applyContrast(const Image& img, float contrast);
Image applyColorTemperature(const Image& img, int temperature);
Image applySaturation(const Image& img, float saturation);
Image applyProjection(const Image& panorama, double R, float scaleFactor);

Image processImage(const Image& img, int brightness, float contrast, float saturation, int temperature);


#endif // IMAGE_PROCESSING_H
