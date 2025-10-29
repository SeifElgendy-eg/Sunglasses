#include "Image_Class.h"
#include <omp.h>
#include "gif.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <random>
#include <limits>
#include <cstdint>
#include <stdexcept>

namespace Config
{
const int MAX_IMAGE_DIMENSION = 16384;
const int MIN_IMAGE_DIMENSION = 1;
const int MORPH_BUCKET_SIZE = 16;
const int MORPH_HOLD_FRAMES = 15;
const int MORPH_GIF_DELAY = 5;
}

bool validateDimensions(int width, int height)
{
    if (width < Config::MIN_IMAGE_DIMENSION || height < Config::MIN_IMAGE_DIMENSION)
    {
        std::cerr << "Error: Image dimensions too small (min: " << Config::MIN_IMAGE_DIMENSION << ")" << std::endl;
        return false;
    }
    if (width > Config::MAX_IMAGE_DIMENSION || height > Config::MAX_IMAGE_DIMENSION)
    {
        std::cerr << "Error: Image dimensions too large (max: " << Config::MAX_IMAGE_DIMENSION << ")" << std::endl;
        return false;
    }
    return true;
}

void grayscale(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            int sum = image(col, row, 0) + image(col, row, 1) + image(col, row, 2);
            int gray = (sum + 1) / 3;
            image(col, row, 0) = image(col, row, 1) = image(col, row, 2) = gray;
        }
    }
}

void bnw(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            int num = (image(col, row, 0) + image(col, row, 1) + image(col, row, 2)) / 3;
            num = (num >= 128) ? 255 : 0;
            image(col, row, 0) = image(col, row, 1) = image(col, row, 2) = num;
        }
    }
}

void invert(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            image(col, row, 0) = 255 - image(col, row, 0);
            image(col, row, 1) = 255 - image(col, row, 1);
            image(col, row, 2) = 255 - image(col, row, 2);
        }
    }
}

void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode)
{
    int outWidth, outHeight;
    if (mode == 'f')
    {
        outWidth = image1.width;
        outHeight = image1.height;
    }
    else if (mode == 's')
    {
        outWidth = image2.width;
        outHeight = image2.height;
    }
    else
    {
        outWidth = std::min(image1.width, image2.width);
        outHeight = std::min(image1.height, image2.height);
    }

    outputImage = Image(outWidth, outHeight);

    for (int row = 0; row < outHeight; row++)
    {
        for (int col = 0; col < outWidth; col++)
        {
            for (int c = 0; c < 3; c++)
            {
                int val1 = (row < image1.height && col < image1.width) ? image1(col, row, c) : 0;
                int val2 = (row < image2.height && col < image2.width) ? image2(col, row, c) : 0;
                int merged = static_cast<int>(alpha * val1 + (1 - alpha) * val2);
                outputImage(col, row, c) = std::clamp(merged, 0, 255);
            }
        }
    }
}

void reflect(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width / 2; col++)
        {
            for (int k = 0; k < 3; k++)
            {
                unsigned int temp = image(col, row, k);
                image(col, row, k) = image(image.width - 1 - col, row, k);
                image(image.width - 1 - col, row, k) = temp;
            }
        }
    }
}

void rotate(Image &image, int degrees)
{
    degrees = ((degrees % 360) + 360) % 360;
    int numRotations = (degrees / 90) % 4;

    for (int r = 0; r < numRotations; r++)
    {
        Image rotated(image.height, image.width);

        for (int row = 0; row < image.height; row++)
        {
            for (int col = 0; col < image.width; col++)
            {
                rotated(image.height - 1 - row, col, 0) = image(col, row, 0);
                rotated(image.height - 1 - row, col, 1) = image(col, row, 1);
                rotated(image.height - 1 - row, col, 2) = image(col, row, 2);
            }
        }
        image = rotated;
    }
}

void dnl(Image &image, int percent)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            for (int c = 0; c < 3; c++)
            {
                int value = image(col, row, c);
                int newValue = value + (value * percent) / 100;
                image(col, row, c) = std::clamp(newValue, 0, 255);
            }
        }
    }
}

bool crop(Image &image, int x, int y, int width, int height)
{
    if (x < 0 || y < 0 || width <= 0 || height <= 0)
    {
        std::cerr << "Error: Invalid crop parameters" << std::endl;
        return false;
    }

    int startX = std::max(0, x);
    int startY = std::max(0, y);
    int cropWidth = std::min(width, image.width - startX);
    int cropHeight = std::min(height, image.height - startY);

    if (startX >= image.width || startY >= image.height || cropWidth <= 0 || cropHeight <= 0)
    {
        std::cerr << "Error: Crop area is outside image bounds" << std::endl;
        return false;
    }

    Image cropped(cropWidth, cropHeight);

    for (int row = 0; row < cropHeight; row++)
    {
        for (int col = 0; col < cropWidth; col++)
        {
            cropped(col, row, 0) = image(startX + col, startY + row, 0);
            cropped(col, row, 1) = image(startX + col, startY + row, 1);
            cropped(col, row, 2) = image(startX + col, startY + row, 2);
        }
    }

    image = cropped;
    return true;
}

void edges(Image &image)
{
    Image copy = image;

    const int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    for (int i = 0; i < image.height; i++)
    {
        for (int j = 0; j < image.width; j++)
        {
            int Ix[3] = {0, 0, 0};
            int Iy[3] = {0, 0, 0};

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni >= 0 && ni < image.height && nj >= 0 && nj < image.width)
                    {
                        int gx_val = Gx[di + 1][dj + 1];
                        int gy_val = Gy[di + 1][dj + 1];

                        for (int c = 0; c < 3; c++)
                        {
                            Ix[c] += copy(nj, ni, c) * gx_val;
                            Iy[c] += copy(nj, ni, c) * gy_val;
                        }
                    }
                }
            }

            for (int c = 0; c < 3; c++)
            {
                int mag = static_cast<int>(std::sqrt(Ix[c] * Ix[c] + Iy[c] * Iy[c]));
                image(j, i, c) = std::min(mag, 255);
            }
        }
    }
}

void blur(Image &image, int kernelSize)
{
    if (kernelSize < 1)
    {
        kernelSize = 1;
    }

    if (kernelSize % 2 == 0)
    {
        kernelSize++;
    }

    int radius = kernelSize / 2;
    Image copy = image;

    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            int sum[3] = {0, 0, 0};
            int count = 0;

            for (int dr = -radius; dr <= radius; dr++)
            {
                for (int dc = -radius; dc <= radius; dc++)
                {
                    int nr = row + dr;
                    int nc = col + dc;

                    if (nr >= 0 && nr < image.height && nc >= 0 && nc < image.width)
                    {
                        sum[0] += copy(nc, nr, 0);
                        sum[1] += copy(nc, nr, 1);
                        sum[2] += copy(nc, nr, 2);
                        count++;
                    }
                }
            }

            image(col, row, 0) = (sum[0] + count / 2) / count;
            image(col, row, 1) = (sum[1] + count / 2) / count;
            image(col, row, 2) = (sum[2] + count / 2) / count;
        }
    }
}

bool frame(Image &image, int thickness, int r, int g, int b, char style)
{
    if (thickness <= 0)
    {
        return false;
    }

    int maxThickness = std::min(image.width, image.height) / 2;
    if (thickness > maxThickness)
    {
        thickness = maxThickness;
    }

    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);

    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            bool inFrame = false;

            if (style == 's')
            {
                inFrame = (row < thickness || row >= image.height - thickness ||
                           col < thickness || col >= image.width - thickness);
            }

            if (inFrame)
            {
                image(col, row, 0) = r;
                image(col, row, 1) = g;
                image(col, row, 2) = b;
            }
        }
    }
    return true;
}

void tv(Image &img){
    //Create a seed for randomization.
    std::random_device rnd;
    std::mt19937 gen(rnd());
    //Sets the randomization range.
    std::uniform_int_distribution<> distr(-40, 40);

    for(int row = 0; row<img.height; row++){
        for(int col = 0; col<img.width; col++){
            //Add random values for each channel.
            img(col, row, 0)=std::max(0, std::min(255, img(col, row, 0)+distr(gen)));
            img(col, row, 1)=std::max(0, std::min(255, img(col, row, 1)+distr(gen)));
            img(col, row, 2)=std::max(0, std::min(255, img(col, row, 2)+distr(gen)));
            if(row%8==4||row%8==5||row%8==6||row%8==7){
                //Half the brightness of rows in groups of 4.
                img(col, row, 0)/=2;
                img(col, row, 1)/=2;
                img(col, row, 2)/=2;
            }
        }

    }
}

Image resizeImageInMemory(Image &image, int newWidth, int newHeight)
{
    if (!validateDimensions(newWidth, newHeight))
    {
        throw std::runtime_error("Invalid resize dimensions");
    }

    double scaleFactorX = static_cast<double>(image.width) / newWidth;
    double scaleFactorY = static_cast<double>(image.height) / newHeight;
    Image resizedImage(newWidth, newHeight);

    for (int row = 0; row < newHeight; row++)
    {
        for (int col = 0; col < newWidth; col++)
        {
            int oldX = static_cast<int>(col * scaleFactorX);
            int oldY = static_cast<int>(row * scaleFactorY);

            oldX = std::clamp(oldX, 0, image.width - 1);
            oldY = std::clamp(oldY, 0, image.height - 1);

            for (int k = 0; k < 3; k++)
            {
                resizedImage(col, row, k) = image(oldX, oldY, k);
            }
        }
    }
    return resizedImage;
}

void morphOptimized(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor,
                    std::vector<int> &pixelMapping)
{
    int width = targetImage.width;
    int height = targetImage.height;
    int pixelCount = width * height;

    const double maxPosDistance = std::sqrt(static_cast<double>(width * width + height * height));
    const double posNormalizationFactor = 441.0 / maxPosDistance;

    const int BUCKET_COUNT = Config::MORPH_BUCKET_SIZE * Config::MORPH_BUCKET_SIZE * Config::MORPH_BUCKET_SIZE;
    std::vector<std::vector<int>> colorBuckets(BUCKET_COUNT);

    // Build color buckets
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int idx = row * width + col;
            int r = sourceImage(col, row, 0) / 16;
            int g = sourceImage(col, row, 1) / 16;
            int b = sourceImage(col, row, 2) / 16;
            int bucketIdx = (r * Config::MORPH_BUCKET_SIZE * Config::MORPH_BUCKET_SIZE) +
                            (g * Config::MORPH_BUCKET_SIZE) + b;
            colorBuckets[bucketIdx].push_back(idx);
        }
    }

    std::vector<bool> pixelUsed(pixelCount, false);
    pixelMapping.resize(pixelCount);

    int pixelsProcessed = 0;
    omp_set_num_threads(std::max(1, omp_get_max_threads() - 2));
#pragma omp parallel for

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int targetR = targetImage(col, row, 0);
            int targetG = targetImage(col, row, 1);
            int targetB = targetImage(col, row, 2);
            int weight = weightsImage(col, row, 0);
            int targetFlatIdx = row * width + col;

            int targetBucketR = targetR / 16;
            int targetBucketG = targetG / 16;
            int targetBucketB = targetB / 16;

            double minDistance = std::numeric_limits<double>::max();
            int bestPixelIndex = -1;

            for (int dr = -3; dr <= 3; dr++)
            {
                for (int dg = -3; dg <= 3; dg++)
                {
                    for (int db = -3; db <= 3; db++)
                    {
                        int br = targetBucketR + dr;
                        int bg = targetBucketG + dg;
                        int bb = targetBucketB + db;

                        if (br < 0 || br >= Config::MORPH_BUCKET_SIZE ||
                            bg < 0 || bg >= Config::MORPH_BUCKET_SIZE ||
                            bb < 0 || bb >= Config::MORPH_BUCKET_SIZE)
                            continue;

                        int bucketIdx = (br * Config::MORPH_BUCKET_SIZE * Config::MORPH_BUCKET_SIZE) +
                                        (bg * Config::MORPH_BUCKET_SIZE) + bb;

                        // Check ALL pixels in this bucket
                        for (size_t i = 0; i < colorBuckets[bucketIdx].size(); i++)
                        {
                            int srcFlatIdx = colorBuckets[bucketIdx][i];

                            if (!pixelUsed[srcFlatIdx])
                            {
                                int srcRow = srcFlatIdx / width;
                                int srcCol = srcFlatIdx % width;

                                int r_diff = targetR - sourceImage(srcCol, srcRow, 0);
                                int g_diff = targetG - sourceImage(srcCol, srcRow, 1);
                                int b_diff = targetB - sourceImage(srcCol, srcRow, 2);
                                double colorDist = std::sqrt(static_cast<double>(
                                    r_diff * r_diff + g_diff * g_diff + b_diff * b_diff));

                                int pos_diff_col = col - srcCol;
                                int pos_diff_row = row - srcRow;
                                double positionDist = std::sqrt(static_cast<double>(
                                    pos_diff_col * pos_diff_col + pos_diff_row * pos_diff_row));
                                positionDist *= posNormalizationFactor;

                                double dist = (1.0 - blendFactor) * colorDist + blendFactor * positionDist;
                                dist *= (1.0 + static_cast<double>(weight) / 255.0);

                                if (dist < minDistance)
                                {
                                    minDistance = dist;
                                    bestPixelIndex = srcFlatIdx;
                                }
                            }
                        }
                    }
                }
            }

            if (bestPixelIndex == -1)
            {
                for (size_t bucketIdx = 0; bucketIdx < colorBuckets.size(); bucketIdx++)
                {
                    for (size_t i = 0; i < colorBuckets[bucketIdx].size(); i++)
                    {
                        int srcFlatIdx = colorBuckets[bucketIdx][i];

                        if (!pixelUsed[srcFlatIdx])
                        {
                            int srcRow = srcFlatIdx / width;
                            int srcCol = srcFlatIdx % width;

                            int r_diff = targetR - sourceImage(srcCol, srcRow, 0);
                            int g_diff = targetG - sourceImage(srcCol, srcRow, 1);
                            int b_diff = targetB - sourceImage(srcCol, srcRow, 2);
                            double colorDist = std::sqrt(static_cast<double>(
                                r_diff * r_diff + g_diff * g_diff + b_diff * b_diff));

                            int pos_diff_col = col - srcCol;
                            int pos_diff_row = row - srcRow;
                            double positionDist = std::sqrt(static_cast<double>(
                                pos_diff_col * pos_diff_col + pos_diff_row * pos_diff_row));
                            positionDist *= posNormalizationFactor;

                            double dist = (1.0 - blendFactor) * colorDist + blendFactor * positionDist;
                            dist *= (1.0 + static_cast<double>(weight) / 255.0);

                            if (dist < minDistance)
                            {
                                minDistance = dist;
                                bestPixelIndex = srcFlatIdx;
                            }
                        }
                    }
                }
            }

            if (bestPixelIndex != -1)
            {
                pixelMapping[targetFlatIdx] = bestPixelIndex;
                pixelUsed[bestPixelIndex] = true;
            }
            else
            {
                pixelMapping[targetFlatIdx] = targetFlatIdx;
            }

            pixelsProcessed++;
            if (pixelsProcessed % 5000 == 0)
            {
                std::cout << "Progress: " << (pixelsProcessed * 100 / pixelCount) << "% ("
                          << pixelsProcessed << "/" << pixelCount << " pixels)\r" << std::flush;
            }
        }
    }
    std::cout << "\nPixel mapping completed!" << std::endl;
}

void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor)
{
    if (!validateDimensions(targetImage.width, targetImage.height))
    {
        throw std::runtime_error("Invalid target image dimensions");
    }

    if (sourceImage.width != targetImage.width || sourceImage.height != targetImage.height)
    {
        sourceImage = resizeImageInMemory(sourceImage, targetImage.width, targetImage.height);
    }

    if (weightsImage.width != targetImage.width || weightsImage.height != targetImage.height)
    {
        weightsImage = resizeImageInMemory(weightsImage, targetImage.width, targetImage.height);
    }

    grayscale(weightsImage);

    std::vector<int> pixelMapping;
    morphOptimized(sourceImage, targetImage, weightsImage, blendFactor, pixelMapping);

    Image morphedImage(targetImage.width, targetImage.height);
    int width = targetImage.width;
    double alpha = blendFactor;

omp_set_num_threads(std::max(1, omp_get_max_threads() - 2));
#pragma omp parallel for
    for (int row = 0; row < targetImage.height; row++)
    {
        for (int col = 0; col < targetImage.width; col++)
        {
            int targetIdx = row * width + col;
            int sourceIdx = pixelMapping[targetIdx];
            int srcRow = sourceIdx / width;
            int srcCol = sourceIdx % width;

            int warpedR = sourceImage(srcCol, srcRow, 0);
            int warpedG = sourceImage(srcCol, srcRow, 1);
            int warpedB = sourceImage(srcCol, srcRow, 2);

            int targetR = targetImage(col, row, 0);
            int targetG = targetImage(col, row, 1);
            int targetB = targetImage(col, row, 2);

            morphedImage(col, row, 0) = static_cast<unsigned char>(
                std::clamp(static_cast<int>(alpha * warpedR + (1.0 - alpha) * targetR), 0, 255));
            morphedImage(col, row, 1) = static_cast<unsigned char>(
                std::clamp(static_cast<int>(alpha * warpedG + (1.0 - alpha) * targetG), 0, 255));
            morphedImage(col, row, 2) = static_cast<unsigned char>(
                std::clamp(static_cast<int>(alpha * warpedB + (1.0 - alpha) * targetB), 0, 255));
        }
    }

    sourceImage = morphedImage;
    std::cout << "Morphing completed with blend factor: " << blendFactor << std::endl;
}

void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage,
                   const std::string &outputPath, int frameCount, double blendFactor)
{
    std::cout << "Starting animated morph generation..." << std::endl;

    if (frameCount <= 1)
    {
        throw std::runtime_error("frameCount must be greater than 1 for animation");
    }

    if (sourceImage.width != targetImage.width || sourceImage.height != targetImage.height)
    {
        sourceImage = resizeImageInMemory(sourceImage, targetImage.width, targetImage.height);
    }

    if (weightsImage.width != targetImage.width || weightsImage.height != targetImage.height)
    {
        weightsImage = resizeImageInMemory(weightsImage, targetImage.width, targetImage.height);
    }

    grayscale(weightsImage);

    int width = targetImage.width;
    int height = targetImage.height;

    std::vector<int> finalPixelMapping;
    morphOptimized(sourceImage, targetImage, weightsImage, blendFactor, finalPixelMapping);

    int totalFrames = Config::MORPH_HOLD_FRAMES + frameCount + Config::MORPH_HOLD_FRAMES;
    std::cout << "Generating " << totalFrames << " animation frames..." << std::endl;

    GifWriter g;
    GifBegin(&g, outputPath.c_str(), width, height, Config::MORPH_GIF_DELAY);

    std::vector<uint8_t> initialFrameBuffer(width * height * 4);
    std::vector<uint8_t> finalFrameBuffer(width * height * 4);
    std::vector<uint8_t> currentFrameBuffer(width * height * 4);
    bool initialFrameSaved = false;
    bool finalFrameSaved = false;

    omp_set_num_threads(std::max(1, omp_get_max_threads() - 2));
#pragma omp parallel for
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int framePos = (row * width + col) * 4;
            initialFrameBuffer[framePos + 0] = sourceImage(col, row, 0);
            initialFrameBuffer[framePos + 1] = sourceImage(col, row, 1);
            initialFrameBuffer[framePos + 2] = sourceImage(col, row, 2);
            initialFrameBuffer[framePos + 3] = 255;
        }
    }
    initialFrameSaved = true;

    for (int frame = 0; frame < totalFrames; frame++)
    {
        float t;
        if (frame < Config::MORPH_HOLD_FRAMES)
        {
            t = 0.0f;
        }
        else if (frame >= Config::MORPH_HOLD_FRAMES + frameCount)
        {
            t = 1.0f;
        }
        else
        {
            int morphFrame = frame - Config::MORPH_HOLD_FRAMES;
            t = static_cast<float>(morphFrame) / (frameCount - 1);
        }

        if (frame < Config::MORPH_HOLD_FRAMES)
        {
            GifWriteFrame(&g, initialFrameBuffer.data(), width, height, Config::MORPH_GIF_DELAY);
        }
        else if (frame >= Config::MORPH_HOLD_FRAMES + frameCount)
        {
            if (!finalFrameSaved)
            {
                std::cerr << "Warning: Final frame not yet generated\n";
            }
            GifWriteFrame(&g, finalFrameBuffer.data(), width, height, Config::MORPH_GIF_DELAY);
        }
        else
        {
            for (int destRow = 0; destRow < height; destRow++)
            {
                for (int destCol = 0; destCol < width; destCol++)
                {
                    int destIdx = destRow * width + destCol;
                    int finalSourceIdx = finalPixelMapping[destIdx];

                    int finalSrcRow = finalSourceIdx / width;
                    int finalSrcCol = finalSourceIdx % width;

                    double sampleCol = destCol * (1.0 - t) + finalSrcCol * t;
                    double sampleRow = destRow * (1.0 - t) + finalSrcRow * t;

                    int srcCol = std::clamp(static_cast<int>(std::round(sampleCol)), 0, width - 1);
                    int srcRow = std::clamp(static_cast<int>(std::round(sampleRow)), 0, height - 1);

                    unsigned char R = sourceImage(srcCol, srcRow, 0);
                    unsigned char G = sourceImage(srcCol, srcRow, 1);
                    unsigned char B = sourceImage(srcCol, srcRow, 2);

                    int framePos = (destRow * width + destCol) * 4;
                    currentFrameBuffer[framePos + 0] = R;
                    currentFrameBuffer[framePos + 1] = G;
                    currentFrameBuffer[framePos + 2] = B;
                    currentFrameBuffer[framePos + 3] = 255;

                    if (frame == Config::MORPH_HOLD_FRAMES + frameCount - 1)
                    {
                        finalFrameBuffer[framePos + 0] = R;
                        finalFrameBuffer[framePos + 1] = G;
                        finalFrameBuffer[framePos + 2] = B;
                        finalFrameBuffer[framePos + 3] = 255;
                        finalFrameSaved = true;
                    }
                }
            }

            GifWriteFrame(&g, currentFrameBuffer.data(), width, height, Config::MORPH_GIF_DELAY);
        }

        if ((frame + 1) % 10 == 0 || frame == totalFrames - 1)
        {
            std::cout << "Progress: " << (frame + 1) << "/" << totalFrames << " frames\r" << std::flush;
        }
    }

    GifEnd(&g);
    std::cout << "\nAnimated GIF creation complete!" << std::endl;
}


Image pixelsort(Image &image, int threshold, int x_s, int x_e , int y_s, int y_e,char mode ){
    for(int col=x_s; col<x_e; col++){
        int curSt = 0;
        std::vector<std::vector<int>> pxls;
        for(int row=y_s; row<y_e; row++){
            if((image(col, row, 0)+image(col, row, 1)+image(col, row, 2))/3<threshold){
                std::vector<int> pxl{image(col, row, 0), image(col, row, 1), image(col, row, 2)};
                pxls.push_back(pxl);
            }
            else{
                if(!pxls.size()){
                    curSt=row+1;
                    continue;
                }

                if (mode == 'b' ){
                std::sort(pxls.begin(), pxls.end(), [](std::vector<int> &a, std::vector<int> &b){return (a[0]+a[1]+a[2])<(b[0]+b[1]+b[2]);});
                }
                else if (mode == 'd' ){
                std::sort(pxls.begin(), pxls.end(), [](std::vector<int> &a, std::vector<int> &b){return (a[0]+a[1]+a[2])>(b[0]+b[1]+b[2]);});
                }
                for(int i = curSt; i<row; i++){
                    image(col, i, 0)=pxls[i-curSt][0];
                    image(col, i, 1)=pxls[i-curSt][1];
                    image(col, i, 2)=pxls[i-curSt][2];
                }
                curSt=row+1;
                pxls.clear();
            }
        }
    }
    return image;
}
