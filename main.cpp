#include <stdlib.h>
#include <cstdio>
#include <complex>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const int MAX_ITERATIONS = 1000;

struct RGB{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

struct Image {
    int width{0};
    int height{0};
    RGB* pixels{nullptr};
};

struct Vec2 {
    double x;
    double y;
};

struct Zoom {
    Vec2 point;
    double scale;
};

struct Range {
    double percent;
    RGB color;
};

Image initImage(int width, int height, bool initToZero = true);
void setPixel(int x, int y, RGB pixel, Image* image);
void write(const char* filename, Image* image);
int getIterations(double x, double y, int maxIterations = MAX_ITERATIONS);
void mandelbrotAlgorithm(Image* image, Zoom zoom, Range* ranges, int rangesSize);
Zoom calculateCenter(Zoom* zoomArray, unsigned int arraySize, Image* image);
Vec2 getFractalCoord(int x, int y, Zoom zoom, Image* image);
int getIterationRangeNumber(int iterations, Range *ranges, int rangesSize);
RGB operator-(RGB first, RGB second);
RGB operator+(RGB first, RGB second);
RGB operator*(RGB color, double value);
RGB operator*(double value, RGB color);

int main() {
    auto image = initImage(800, 600);

    Range ranges[4];
    ranges[0] = {0.0, (RGB){0, 0, 0}};
    ranges[1] = {0.3, (RGB){255, 0, 0}};
    ranges[2] = {0.5, (RGB){255, 255, 0}};
    ranges[3] = {1.0, (RGB){255, 255, 255}};

    Zoom zoomArray[3];
    zoomArray[0] = {(Vec2){0, 0}, 4.0/image.width};
    zoomArray[1] = {(Vec2){295, 202}, 0.1};
    zoomArray[2] = {(Vec2){312, 304}, 0.1};

    auto center = calculateCenter(&zoomArray[0], 3, &image);

    mandelbrotAlgorithm(&image, center, &ranges[0], 4);
    write("teste.bmp", &image);
    free(image.pixels);
    return 0;
}

Image initImage(int width, int height, bool initToZero){
    Image image = {};
    image.width = width;
    image.height = height;
    image.pixels = (RGB*)malloc(sizeof(RGB) * width * height);
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

void setPixel(int x, int y, RGB pixel, Image* image){
    auto pos = y*image->width + x;
    image->pixels[pos] = pixel;
}

void mandelbrotAlgorithm(Image* image, Zoom zoom, Range* ranges, int rangesSize){
    int histogram[MAX_ITERATIONS]{0};
    int fractal[image->width][image->height];

    for(int y = 0; y < image->height; y++){
        for(int x = 0; x < image->width; x++){
            auto coord = getFractalCoord(x, y, zoom, image);

            auto iterations = getIterations(coord.x, coord.y);
            if(iterations < MAX_ITERATIONS){
                histogram[iterations]++;
            }
            fractal[x][y] = iterations;
        }
    }

    //Calculating the range totals
    int rangeTotals[rangesSize - 1];

    for(int i = 0; i < rangesSize - 1; i++){
        rangeTotals[i] = 0;
    }

    int rangeIndex = 0;
    for(int i = 0; i < MAX_ITERATIONS; i++) {
        if(i >= ranges[rangeIndex+1].percent * MAX_ITERATIONS){
            rangeIndex++;
        }
        rangeTotals[rangeIndex] += histogram[i];
    }

    //Setting the color of each pixel
    for(int y = 0; y < image->height; y++){
        for(int x = 0; x < image->width; x++){

            int iterations = fractal[x][y];

            RGB pixel{0, 0, 0};

            if(iterations < MAX_ITERATIONS){
                int pixelsInRange = 0;

                int rangeNumber = getIterationRangeNumber(iterations, ranges, rangesSize);
                int rangeTotal = rangeTotals[rangeNumber];

                int rangeStart = int(ranges[rangeNumber].percent * MAX_ITERATIONS);
                for(int i = rangeStart; i < iterations; i++){
                    pixelsInRange += histogram[i];
                }

                RGB startColor = ranges[rangeNumber].color;
                RGB endColor = ranges[rangeNumber + 1].color;
                RGB colorDiff = endColor - startColor;

                pixel = startColor + (colorDiff*(double(pixelsInRange)/rangeTotal));

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

Zoom calculateCenter(Zoom* zoomArray, unsigned int arraySize, Image* image){
    Zoom center{};
    center.point.x = image->width*0.5;
    center.point.y = image->height*0.5;
    center.scale = 1;

    for(int i = 0; i < arraySize; i++){
        Zoom zoom = zoomArray[i];
        center.point.x += (zoom.point.x - image->width*0.5) * center.scale;
        center.point.y += (zoom.point.y - image->height*0.5) * center.scale;
        center.scale *= zoom.scale;
    }

    return center;
}

Vec2 getFractalCoord(int x, int y, Zoom zoom, Image* image){
    Vec2 fractalCoord{};
    auto halfWidth = image->width*0.5;
    auto halfHeight = image->height*0.5;
    fractalCoord.x = (x - halfWidth)*zoom.scale + zoom.point.x;
    fractalCoord.y = (y - halfHeight)*zoom.scale + zoom.point.y;
    return  fractalCoord;
}

int getIterationRangeNumber(int iterations, Range *ranges, int rangesSize){
    if(ranges != nullptr && rangesSize > 0){
        for(int i = 0; i < rangesSize - 1; i++){
            Range rangeStart = ranges[i];
            Range rangeEnd = ranges[i+1];
            if(iterations >= rangeStart.percent * MAX_ITERATIONS && iterations < rangeEnd.percent * MAX_ITERATIONS){
                return i;
            }
        }
    }

    return 0;
}

RGB operator-(RGB first, RGB second){
    RGB result = {};
    result.r = first.r - second.r;
    result.g = first.g - second.g;
    result.b = first.b - second.b;
    return result;
}

RGB operator+(RGB first, RGB second){
    RGB result = {};
    result.r = first.r + second.r;
    result.g = first.g + second.g;
    result.b = first.b + second.b;
    return result;
}

RGB operator*(RGB color, double value){
    RGB result = {};
    result.r = (unsigned char)(color.r * value);
    result.g = (unsigned char)(color.g * value);
    result.b = (unsigned char)(color.b * value);
    return result;
}

RGB operator*(double value, RGB color){
    RGB result = {};
    result.r = (unsigned char)(color.r * value);
    result.g = (unsigned char)(color.g * value);
    result.b = (unsigned char)(color.b * value);
    return result;
}