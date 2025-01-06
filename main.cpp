#include <iostream>
#include "Image.h"
#include "ImageProcessing.h"

int main() {
    try {
        // 從 JPEG 文件加載影像
        Image img = Image::loadFromJPG("input.jpg");
        std::cout << "Loaded image: " << img.getWidth() << "x" << img.getHeight() << ", " << img.getChannels() << " channels." << std::endl;

        // 應用亮度過濾器
        Image brighterImg = applyBrightness(img, 50);

        // 保存結果為 JPEG
        brighterImg.saveAsJPG("output_brightness.jpg", 90);

        std::cout << "Processed image saved as output_brightness.jpg." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
