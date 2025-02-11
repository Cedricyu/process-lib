#include <iostream>
#include "Image.h"
#include "ImageProcessing.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h> // 用於顯示文字
#include <vector>
#include <cstdint>
#include <algorithm>

#include <sstream>
#include <iomanip> // for std::setprecision

std::string formatFloat(float value, int decimalPlaces) {
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(decimalPlaces) << value;
	return stream.str();
}

// 初始化 SDL 字型系統（需要安裝 SDL_ttf）
bool initFont(TTF_Font*& font) {
	if (TTF_Init() == -1) {
		std::cerr << "Failed to initialize TTF: " << TTF_GetError() << std::endl;
		return false;
	}

	// 設置字型和大小
	font = TTF_OpenFont("arial.ttf", 18); // 確保存在 "arial.ttf" 字型文件
	if (!font) {
		std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
		return false;
	}
	return true;
}

// 繪製文字
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y, SDL_Color color) {
	SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect destRect = { x, y, surface->w, surface->h };
	SDL_RenderCopy(renderer, texture, nullptr, &destRect);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

void renderTextRight(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int rightX, int y, SDL_Color color) {
	int textWidth, textHeight;
	TTF_SizeText(font, text.c_str(), &textWidth, &textHeight); // 計算文字寬高
	SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect destRect = { rightX - textWidth, y, textWidth, textHeight }; // 靠右對齊
	SDL_RenderCopy(renderer, texture, nullptr, &destRect);

	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}



void displayImage() {

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return;
	}

	TTF_Font* font = nullptr;
	if (!initFont(font)) {
		SDL_Quit();
		return ;
	}


	// 複製原始圖像
	Image image = Image::loadFromJPG("paranoma.JPEG");
	std::cout << "Loaded image: " << image.getWidth() << "x" << image.getHeight() << ", " << image.getChannels() << " channels." << std::endl;

	int screenWidth = 1024;
	int screenHeight = 600;

	SDL_Window* window = SDL_CreateWindow(
		"Hello SDL2",
		SDL_WINDOWPOS_CENTERED,  // 水平居中
		SDL_WINDOWPOS_CENTERED,  // 垂直居中
		screenWidth,
		screenHeight,
		SDL_WINDOW_SHOWN         // 顯示窗口
	);

	if (!window) {
		std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		TTF_CloseFont(font);
		TTF_Quit();
		SDL_Quit();
		return;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window);
		SDL_Quit();
		return;
	}

	// 等待退出事件
	SDL_Event e;
	bool quit = false;

	bool dragging = false;
	int selectedParameter = 2; // 0: 調整亮度, 1: 調整對比度
	int brightness = 0;       // 初始亮度
	float contrast = 1.0f;    // 初始對比度
	float saturation = 1.0f;  // 初始飽和度
	int temperature = 0;      // 初始色溫

	SDL_Texture* texture = nullptr;

	// 初始化圖像的原始大小
	int imgWidth = image.getWidth();
	int imgHeight = image.getHeight();

	const int displayWidth = 800;
	const int displayHeight = 600;

	float scale = 1.0f; // 圖像縮放比例，初始為 1.0
	int centerX = imgWidth / 2; // 初始化中心點為圖像中心
	int centerY = imgHeight / 2;
	SDL_Rect srcRect = { 0, 0, imgWidth, imgHeight };

	bool cylindricalProjection = false; // 投影模式開關
	bool isProjectionApplied = false;   // 投影是否已經應用

	while (!quit) {

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				// Q 切換到調整亮度, W 切換到調整對比度
				if (e.key.keysym.sym == SDLK_q) {
					selectedParameter = 0; // 調整亮度
				}
				else if (e.key.keysym.sym == SDLK_w) {
					selectedParameter = 1; // 調整對比度
				}
				else if (e.key.keysym.sym == SDLK_p) {
					cylindricalProjection = true; // 啟用投影
				}

			}
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				// 設定新的縮放中心點
				if (e.button.button == SDL_BUTTON_LEFT) {
					int mouseX = e.button.x;
					int mouseY = e.button.y;

					// 將鼠標點轉換為圖像中的位置
					centerX = srcRect.x + static_cast<int>(mouseX * srcRect.w / displayWidth);
					centerY = srcRect.y + static_cast<int>(mouseY * srcRect.h / displayHeight);
				}
			}
			else if (e.type == SDL_MOUSEWHEEL) {
				// 滑鼠滾輪控制參數
				if (selectedParameter == 0) { // 調整亮度
					brightness += e.wheel.y * 5; // 每次滾動調整 5
					brightness = std::clamp(brightness, -100, 100); // 限制亮度範圍
				}
				else if (selectedParameter == 1) { // 調整對比度
					contrast += e.wheel.y * 0.1f; // 每次滾動調整 0.1
					contrast = std::clamp(contrast, 0.5f, 2.0f); // 限制對比度範圍
				}
				else if (selectedParameter == 2) { // 調整縮放比例
					scale += e.wheel.y * 0.1f; // 每次滾動縮放 10%
					scale = std::clamp(scale, 0.1f, 4.0f); // 限制縮放比例

					// 更新裁剪區域（源矩形）
					int newWidth = static_cast<int>(imgWidth / scale);
					int newHeight = static_cast<int>(imgHeight / scale);
					srcRect.w = newWidth;
					srcRect.h = newHeight;

					int maxX = std::max(0, imgWidth - newWidth);  // 確保最大值不為負
					int maxY = std::max(0, imgHeight - newHeight); // 確保最大值不為負

					srcRect.x = std::clamp(centerX - newWidth / 2, 0, maxX);
					srcRect.y = std::clamp(centerY - newHeight / 2, 0, maxY);
				}
			}
		}

		if (cylindricalProjection && !isProjectionApplied) {
			double R = 340;
			image = applyProjection(image, R, 2.0);
			isProjectionApplied = true;                       // 設置已完成標誌

			const std::string outputFilename = "output_paranoma.jpg";
			const int quality = 100;
			image.saveAsJPG(outputFilename, quality);
			std::cout << "apply CylindricalProjection !" << std::endl;
		} 

		Image modifiedImage = processImage(image, brightness, contrast, saturation, temperature);

		// 圖像的原始大小
		int imgWidth = modifiedImage.getWidth();
		int imgHeight = modifiedImage.getHeight();

		// 計算圖像顯示大小和位置
		SDL_Rect destRect;
		float areaAspect = static_cast<float>(displayWidth) / displayHeight;
		float imgAspect = static_cast<float>(imgWidth) / imgHeight;

		// 根據縮放比例調整圖像大小
		int scaledWidth = static_cast<int>(imgWidth * scale);
		int scaledHeight = static_cast<int>(imgHeight * scale);

		if (imgAspect > areaAspect) {
			// 圖像寬比高更長，寬度填滿
			destRect.w = std::min(scaledWidth, displayWidth); // 限制在顯示區域內
			destRect.h = static_cast<int>(destRect.w / imgAspect);
			destRect.x = (displayWidth - destRect.w) / 2;
			destRect.y = (displayHeight - destRect.h) / 2;
		}
		else {
			// 圖像高比寬更長，高度填滿
			destRect.h = std::min(scaledHeight, displayHeight); // 限制在顯示區域內
			destRect.w = static_cast<int>(destRect.h * imgAspect);
			destRect.x = (displayWidth - destRect.w) / 2;
			destRect.y = (displayHeight - destRect.h) / 2;
		}
		// 更新紋理
		if (texture) {
			SDL_DestroyTexture(texture); // 釋放舊紋理
		}
		texture = SDL_CreateTexture(
			renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
			modifiedImage.getWidth(), modifiedImage.getHeight()
		);
		SDL_UpdateTexture(texture, nullptr, modifiedImage.getData().data(), imgWidth * modifiedImage.getChannels());

		// 清屏
		SDL_RenderClear(renderer);

		// 繪製左側黑色背景
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_Rect leftArea = { 0, 0, displayWidth, displayHeight };
		SDL_RenderFillRect(renderer, &leftArea);

		// 繪製圖像到左側區域
		SDL_RenderCopy(renderer, texture, &srcRect, &destRect);

		// 更新畫面
		SDL_RenderPresent(renderer);

		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // 深灰色背景
		SDL_Rect paramRect = { 800, 0, 224, 600 };
		SDL_RenderFillRect(renderer, &paramRect);

		// 設定文字樣式
		SDL_Color titleColor = { 255, 255, 0, 255 }; // 黃色標題
		SDL_Color textColor = { 255, 255, 255, 255 }; // 白色普通文字

		// 繪製參數標題
		renderText(renderer, font, "Image Settings", 820, 20, titleColor);

		// 繪製參數文字（使用更好的間距）
		renderText(renderer, font, "Brightness:", 820, 80, textColor);
		renderTextRight(renderer, font, std::to_string(brightness), 1000, 80, textColor);

		renderText(renderer, font, "Contrast:", 820, 120, textColor);
		renderTextRight(renderer, font, formatFloat(contrast, 2), 1000, 120, textColor);

		renderText(renderer, font, "Saturation:", 820, 160, textColor);
		renderTextRight(renderer, font, formatFloat(saturation, 1), 1000, 160, textColor);

		renderText(renderer, font, "Temperature:", 820, 200, textColor);
		renderTextRight(renderer, font, std::to_string(temperature), 1000, 200, textColor);

		// 更新畫面
		SDL_RenderPresent(renderer);
	}

	TTF_CloseFont(font);
	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main() {
	displayImage();
	return 0;
}

