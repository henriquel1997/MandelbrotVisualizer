#include <stdlib.h>
#include <cstdio>
#include <complex>
#include "math.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const int MAX_ITERATIONS = 1000;

#pragma pack(2)
struct RGB_Pixel{
    unsigned char r{0};
    unsigned char g{0};
    unsigned char b{0};
};

struct Image {
    int width{0};
    int height{0};
    RGB_Pixel* pixels{nullptr};
};

Image initImage(int width, int height, bool initToZero = true);
void setPixel(int x, int y, RGB_Pixel pixel, Image* image);
void write(const char* filename, Image* image);
int getIterations(double x, double y, int maxIterations = MAX_ITERATIONS);
void mandelbrotAlgorithm(Image* image);

int main() {
    auto image = initImage(800, 600);
    mandelbrotAlgorithm(&image);
    write("teste.bmp", &image);
    free(image.pixels);
    return 0;
}

Image initImage(int width, int height, bool initToZero){
    Image image = {};
    image.width = width;
    image.height = height;
    image.pixels = (RGB_Pixel*)malloc(sizeof(RGB_Pixel) * width * height);
    if(initToZero){
        for(int i = 0; i < width*height; i++){
            image.pixels[i].r = 0;
            image.pixels[i].g = 0;
            image.pixels[i].b = 0;
        }
    }
    return image;
}

void write(const char* filename, Image* image){
    auto result = stbi_write_bmp(filename, image->width, image->height, 3, image->pixels);
    if(result){
        printf("Salvo!\n");
    }else{
        printf("Erro.\n");
    }
}

void setPixel(int x, int y, RGB_Pixel pixel, Image* image){
    auto pos = y*image->width + x;
    image->pixels[pos] = pixel;
}

void mandelbrotAlgorithm(Image* image){

    int historgram[MAX_ITERATIONS]{0};
    int fractal[image->width][image->height];

    double halfWidth = image->width*0.5;
    double halfHeight= image->height*0.5;
    for(int y = 0; y < image->height; y++){
        double yFractal = (y - halfHeight)/halfHeight;
        for(int x = 0; x < image->width; x++){
            double xFractal = (x - halfWidth - 200)/halfHeight;

            auto iterations = getIterations(xFractal, yFractal);
            if(iterations < MAX_ITERATIONS){
                historgram[iterations]++;
            }
            fractal[x][y] = iterations;
        }
    }

    int historgramTotal = 0;
    for(int count : historgram){
        historgramTotal += count;
    }

    for(int y = 0; y < image->height; y++){
        for(int x = 0; x < image->width; x++){

            RGB_Pixel pixel{};

            int iterations = fractal[x][y];

            if(iterations < MAX_ITERATIONS){
                double hue = 0;

                for(int i = 0; i < iterations; i++){
                    hue += double(historgram[i]);
                }

                hue /= historgramTotal;

                pixel.g = (unsigned char)pow(255, hue);
            }

            setPixel(x, y, pixel, image);
        }
    }

}

int getIterations(double x, double y, int maxIterations){
    std::complex<double> z = 0;
    std::complex<double> c(x, y);

    int iterations = 0;

    while (iterations < maxIterations){
        z = z*z + c;
        if(abs(z) > 2){
            break;
        }
        iterations++;
    }

    return iterations;
}