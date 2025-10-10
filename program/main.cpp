/*
Section: s12

Team Details:
Marwan Mohamed Hassan    20240735
Mohamed Talat Sayed      20240734
Seifeldeen Hatem Moahmed 20242424

Filters:
Marwan  2,5,8,11
Mohamed 1,4,7,10
Seif    3,6,9,12
*/

#include <Image_Class.h>
#include "gif.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <limits>
#include <cstdint>
#include <memory>
#include <stdexcept>

// Configuration Constants
namespace Config
{
    const int MAX_IMAGE_DIMENSION = 16384; // Maximum allowed dimension
    const int MIN_IMAGE_DIMENSION = 1;     // Minimum allowed dimension
    const int MORPH_BUCKET_SIZE = 16;      // Color quantization for morphing
    const int MORPH_HOLD_FRAMES = 15;      // Hold frames at start/end of animation
    const int MORPH_GIF_DELAY = 5;         // Delay between GIF frames (centiseconds)
    const int MORPH_BASE_CHECKS = 200;     // Base number of pixel checks
    const int MORPH_WEIGHT_CHECKS = 800;   // Additional checks based on weight
    const int MORPH_FALLBACK_CHECKS = 500; // Global fallback search limit
    const double MORPH_EARLY_STOP = 10.0;  // Early stopping threshold
}

// Function Declarations
void grayscale(Image &image);
void bnw(Image &image);
void invert(Image &image);
void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode);
void reflect(Image &image);
void rotate(Image &image, int degrees);
void dnl(Image &image, int percent);
bool crop(Image &image, int x, int y, int width, int height);
bool frame(Image &image, int thickness, int r, int g, int b, char style = 's');
void edges(Image &image);
void blur(Image &image, int kernelSize);
void resizeImage(Image &image, const std::string &imageName, int newWidth = -1, int newHeight = -1,
                 double scaleFactorX = -1, double scaleFactorY = -1);
Image resizeImageInMemory(Image &image, int newWidth, int newHeight);
void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor = 0.5);
void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage, const std::string &outputPath,
                   int frameCount = 30, double blendFactor = 0.5);
void morphOptimized(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor,
                    std::vector<int> &pixelMapping);

// Helper function to validate image dimensions
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

// Morph implementation with Warping + Blending
void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor)
{
    // Step 1: Validate and resize images to match target dimensions
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

    // Step 2: Use optimized morphing algorithm to create the pixel warp map
    std::vector<int> pixelMapping;
    morphOptimized(sourceImage, targetImage, weightsImage, blendFactor, pixelMapping);

    // Step 3: Apply pixel mapping (WARP) and BLEND the warped source with target
    Image morphedImage(targetImage.width, targetImage.height);
    int width = targetImage.width;
    double alpha = blendFactor;

    for (int row = 0; row < targetImage.height; row++)
    {
        for (int col = 0; col < targetImage.width; col++)
        {
            int targetIdx = row * width + col;
            int sourceIdx = pixelMapping[targetIdx];
            int srcRow = sourceIdx / width;
            int srcCol = sourceIdx % width;

            // Get Warped Source Color
            int warpedR = sourceImage(srcCol, srcRow, 0);
            int warpedG = sourceImage(srcCol, srcRow, 1);
            int warpedB = sourceImage(srcCol, srcRow, 2);

            // Get Target Color
            int targetR = targetImage(col, row, 0);
            int targetG = targetImage(col, row, 1);
            int targetB = targetImage(col, row, 2);

            // Color Blending: alpha * Warped_Source + (1.0 - alpha) * Target
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

void printUsage(const char *programName)
{
    std::cout << "Usage: " << programName << " <input_image> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --grayscale              Convert image to grayscale\n";
    std::cout << "  --bnw                    Convert image to black and white\n";
    std::cout << "  --invert                 Invert image colors\n";
    std::cout << "  --reflect                Reflect image horizontally\n";
    std::cout << "  --rotate [degrees]       Rotate image (default: 90)\n";
    std::cout << "  --lighten [percent]      Lighten image (default: 20%)\n";
    std::cout << "  --darken [percent]       Darken image (default: 20%)\n";
    std::cout << "  --crop [x y w h]         Crop image (default: center 50%)\n";
    std::cout << "  --frame [thick r g b]    Add frame (default: 20px black)\n";
    std::cout << "  --edges                  Apply edge detection\n";
    std::cout << "  --blur [kernel_size]     Apply blur (default: 3)\n";
    std::cout << "  --merge <image2> <alpha> <mode>  Merge with another image\n";
    std::cout << "                           alpha: 0.0-1.0, mode: f/s/m\n";
    std::cout << "  --resize [w h]           Resize image by width and height\n";
    std::cout << "  --morph <target> [weights]  Morph source to target (optional: weights)\n";
    std::cout << "  --blend <0.0-1.0>        Set morph blend (0=match target, 1=keep source)\n";
    std::cout << "  --animate [frames]       Create animated GIF (default: 30 frames)\n";
    std::cout << "  -o <output_file>         Specify output filename (default: output.png)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " input.png --grayscale -o gray.png\n";
    std::cout << "  " << programName << " input.png --rotate 90 -o rotated.png\n";
    std::cout << "  " << programName << " input.png --blur 5 -o blurred.png\n";
    std::cout << "  " << programName << " source.png --morph target.png weights.png -o morphed.png\n";
    std::cout << "  " << programName << " source.png --morph target.png -o morphed.png  # no weights\n";
    std::cout << "  " << programName << " source.png --morph target.png --blend 0.8 -o result.png\n";
    std::cout << "  " << programName << " source.png --morph target.png --animate -o anim.gif\n";
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return -1;
    }

    Image img(argv[1]);
    if (img.imageData == nullptr)
    {
        std::cerr << "Error: Could not load image '" << argv[1] << "'" << std::endl;
        return -2;
    }

    if (!validateDimensions(img.width, img.height))
    {
        return -2;
    }

    std::string outputFile = "output.png";
    bool filterApplied = false;
    double morphBlendFactor = 0.5;
    bool animateMode = false;
    int animateFrames = 30;

    // Use smart pointers for automatic memory management
    std::unique_ptr<Image> pendingTargetImg;
    std::unique_ptr<Image> pendingWeightsImg;
    bool morphPending = false;

    // Parse command-line arguments
    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];

        // Convert flag to single character for switch
        char flag = '\0';
        if (arg == "--grayscale")
            flag = 'g';
        else if (arg == "--bnw")
            flag = 'b';
        else if (arg == "--invert")
            flag = 'i';
        else if (arg == "--reflect")
            flag = 'r';
        else if (arg == "--rotate")
            flag = 'R';
        else if (arg == "--lighten")
            flag = 'l';
        else if (arg == "--darken")
            flag = 'd';
        else if (arg == "--crop")
            flag = 'c';
        else if (arg == "--frame")
            flag = 'f';
        else if (arg == "--edges")
            flag = 'e';
        else if (arg == "--blur")
            flag = 'B';
        else if (arg == "--merge")
            flag = 'm';
        else if (arg == "-o")
            flag = 'o';
        else if (arg == "--resize")
            flag = 'z';
        else if (arg == "--morph")
            flag = 'M';
        else if (arg == "--blend")
            flag = 'X';
        else if (arg == "--animate")
            flag = 'A';
        else if (arg == "--help" || arg == "-h")
            flag = 'h';

        try
        {
            switch (flag)
            {
            case 'g': // grayscale
                grayscale(img);
                filterApplied = true;
                break;

            case 'b': // black and white
                bnw(img);
                filterApplied = true;
                break;

            case 'i': // invert
                invert(img);
                filterApplied = true;
                break;

            case 'r': // reflect
                reflect(img);
                filterApplied = true;
                break;

            case 'R': // rotate
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    int degrees = std::atoi(argv[++i]);
                    rotate(img, degrees);
                }
                else
                {
                    rotate(img, 90); // default: 90 degrees
                }
                filterApplied = true;
                break;

            case 'l': // lighten
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    int percent = std::atoi(argv[++i]);
                    dnl(img, percent);
                }
                else
                {
                    dnl(img, 20); // default: lighten by 20%
                }
                filterApplied = true;
                break;

            case 'd': // darken
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    int percent = std::atoi(argv[++i]);
                    dnl(img, -percent);
                }
                else
                {
                    dnl(img, -20); // default: darken by 20%
                }
                filterApplied = true;
                break;

            case 'c': // crop
                if (i + 4 < argc)
                {
                    int x = std::atoi(argv[++i]);
                    int y = std::atoi(argv[++i]);
                    int w = std::atoi(argv[++i]);
                    int h = std::atoi(argv[++i]);
                    if (crop(img, x, y, w, h))
                    {
                        filterApplied = true;
                    }
                }
                else
                {
                    // default: crop to center 50% of image
                    int w = img.width / 2;
                    int h = img.height / 2;
                    int x = img.width / 4;
                    int y = img.height / 4;
                    if (crop(img, x, y, w, h))
                    {
                        filterApplied = true;
                    }
                }
                break;

            case 'f': // frame
                if (i + 4 < argc)
                {
                    int thickness = std::atoi(argv[++i]);
                    int r = std::atoi(argv[++i]);
                    int g = std::atoi(argv[++i]);
                    int b = std::atoi(argv[++i]);
                    if (frame(img, thickness, r, g, b))
                    {
                        filterApplied = true;
                    }
                }
                else
                {
                    if (frame(img, 20, 0, 0, 0)) // default: 20px black frame
                    {
                        filterApplied = true;
                    }
                }
                break;

            case 'e': // edges
                edges(img);
                filterApplied = true;
                break;

            case 'B': // blur
                if (i + 1 < argc)
                {
                    int kernelSize = std::atoi(argv[++i]);
                    blur(img, kernelSize);
                    filterApplied = true;
                }
                else
                {
                    std::cerr << "Error: --blur requires a kernel size value\n";
                    return -1;
                }
                break;

            case 'm': // merge
                if (i + 3 < argc)
                {
                    std::string img2Path = argv[++i];
                    float alpha = std::atof(argv[++i]);
                    char mode = argv[++i][0];

                    Image img2(img2Path.c_str());
                    if (img2.imageData == nullptr)
                    {
                        std::cerr << "Error: Could not load second image '" << img2Path << "'" << std::endl;
                        return -2;
                    }

                    Image outputImg;
                    merge(img, img2, outputImg, alpha, mode);
                    img = outputImg;
                    filterApplied = true;
                }
                else
                {
                    std::cerr << "Error: --merge requires 3 values: image2_path alpha mode\n";
                    return -1;
                }
                break;

            case 'o': // output file
                if (i + 1 < argc)
                {
                    outputFile = argv[++i];
                }
                else
                {
                    std::cerr << "Error: -o requires an output filename\n";
                    return -1;
                }
                break;

            case 'z': // resize
                if (i + 2 < argc)
                {
                    int newWidth = std::atoi(argv[++i]);
                    int newHeight = std::atoi(argv[++i]);

                    if (!validateDimensions(newWidth, newHeight))
                    {
                        return -1;
                    }

                    img = resizeImageInMemory(img, newWidth, newHeight);
                    filterApplied = true;
                }
                else
                {
                    std::cerr << "Error: --resize requires 2 values: newWidth newHeight\n";
                    return -1;
                }
                break;

            case 'M': // morph
                if (i + 1 < argc)
                {
                    std::string targetPath = argv[++i];
                    pendingTargetImg = std::make_unique<Image>(targetPath.c_str());

                    if (pendingTargetImg->imageData == nullptr)
                    {
                        std::cerr << "Error: Could not load target image '" << targetPath << "'" << std::endl;
                        return -2;
                    }

                    // Check if weights image is provided (optional)
                    if (i + 1 < argc && argv[i + 1][0] != '-')
                    {
                        std::string weightsPath = argv[++i];
                        pendingWeightsImg = std::make_unique<Image>(weightsPath.c_str());

                        if (pendingWeightsImg->imageData == nullptr)
                        {
                            std::cerr << "Error: Could not load weights image '" << weightsPath << "'" << std::endl;
                            return -2;
                        }
                    }
                    else
                    {
                        // Create uniform white weights
                        pendingWeightsImg = std::make_unique<Image>(img.width, img.height);
                        for (int row = 0; row < pendingWeightsImg->height; row++)
                        {
                            for (int col = 0; col < pendingWeightsImg->width; col++)
                            {
                                (*pendingWeightsImg)(col, row, 0) = 255;
                                (*pendingWeightsImg)(col, row, 1) = 255;
                                (*pendingWeightsImg)(col, row, 2) = 255;
                            }
                        }
                        std::cout << "No weights image provided - using uniform weights\n";
                    }

                    morphPending = true;
                    filterApplied = true;
                }
                else
                {
                    std::cerr << "Error: --morph requires at least 1 value: target_image [weights_image]\n";
                    return -1;
                }
                break;

            case 'X': // blend factor for morph
                if (i + 1 < argc)
                {
                    morphBlendFactor = std::atof(argv[++i]);
                    if (morphBlendFactor < 0.0 || morphBlendFactor > 1.0)
                    {
                        std::cerr << "Warning: Blend factor should be between 0.0 and 1.0. Clamping value.\n";
                        morphBlendFactor = std::clamp(morphBlendFactor, 0.0, 1.0);
                    }
                    std::cout << "Morph blend factor set to: " << morphBlendFactor << "\n";
                }
                else
                {
                    std::cerr << "Error: --blend requires a value between 0.0 and 1.0\n";
                    return -1;
                }
                break;

            case 'A': // animate flag
                animateMode = true;
                // Check if frame count is provided (optional)
                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    animateFrames = std::atoi(argv[++i]);
                    if (animateFrames < 2)
                    {
                        std::cerr << "Warning: Frame count should be at least 2. Setting to 2.\n";
                        animateFrames = 2;
                    }
                    std::cout << "Animation mode enabled with " << animateFrames << " frames\n";
                }
                else
                {
                    std::cout << "Animation mode enabled with default " << animateFrames << " frames\n";
                }
                break;

            case 'h': // help
                printUsage(argv[0]);
                return 0;

            default: // unknown option
                std::cerr << "Error: Unknown option '" << arg << "'\n";
                printUsage(argv[0]);
                return -1;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error processing option '" << arg << "': " << e.what() << std::endl;
            return -1;
        }
    }

    if (!filterApplied)
    {
        std::cerr << "Warning: No filter applied. Use --help to see available options.\n";
    }

    // Execute morph if pending
    if (morphPending)
    {
        try
        {
            if (animateMode)
            {
                morphAnimated(img, *pendingTargetImg, *pendingWeightsImg, outputFile, animateFrames, morphBlendFactor);
                std::cout << "Animated GIF saved to: " << outputFile << std::endl;
                return 0;
            }
            else
            {
                morph(img, *pendingTargetImg, *pendingWeightsImg, morphBlendFactor);
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error during morphing: " << e.what() << std::endl;
            return -1;
        }
    }

    img.saveImage(outputFile.c_str());
    std::cout << "Image saved to: " << outputFile << std::endl;

    return 0;
}

void grayscale(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            // Use integer arithmetic for efficiency
            int sum = image(col, row, 0) + image(col, row, 1) + image(col, row, 2);
            int gray = (sum + 1) / 3; // Proper rounding
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
    // Determine output dimensions
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
        std::cerr << "Error: Invalid crop parameters (x=" << x << ", y=" << y
                  << ", w=" << width << ", h=" << height << ")\n";
        return false;
    }

    int startX = std::max(0, x);
    int startY = std::max(0, y);
    int cropWidth = std::min(width, image.width - startX);
    int cropHeight = std::min(height, image.height - startY);

    if (startX >= image.width || startY >= image.height || cropWidth <= 0 || cropHeight <= 0)
    {
        std::cerr << "Error: Crop area is outside image bounds\n";
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
        std::cerr << "Warning: Kernel size must be at least 1, using 1\n";
        kernelSize = 1;
    }

    if (kernelSize % 2 == 0)
    {
        kernelSize++;
        std::cout << "Note: Adjusted kernel size to " << kernelSize << " (must be odd)\n";
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
        std::cerr << "Error: Frame thickness must be positive\n";
        return false;
    }

    int maxThickness = std::min(image.width, image.height) / 2;
    if (thickness > maxThickness)
    {
        std::cerr << "Warning: Frame thickness too large, using max: " << maxThickness << "\n";
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

void resizeImage(Image &image, const std::string &imageName, int newWidth, int newHeight,
                 double scaleFactorX, double scaleFactorY)
{
    if (scaleFactorX > 0 && scaleFactorY > 0)
    {
        newWidth = static_cast<int>(scaleFactorX * image.width);
        newHeight = static_cast<int>(scaleFactorY * image.height);
    }
    else if (scaleFactorX > 0 && scaleFactorY == -1)
    {
        newWidth = static_cast<int>(scaleFactorX * image.width);
        newHeight = static_cast<int>((double)newWidth / image.width * image.height);
    }
    else if (scaleFactorX == -1 && scaleFactorY > 0)
    {
        newHeight = static_cast<int>(scaleFactorY * image.height);
        newWidth = static_cast<int>((double)newHeight / image.height * image.width);
    }
    else if (newWidth == -1 && scaleFactorX != -1)
    {
        newWidth = static_cast<int>(image.width * scaleFactorX);
    }

    if (newHeight == -1 && scaleFactorY != -1)
    {
        newHeight = static_cast<int>(image.height * scaleFactorY);
    }

    if (newWidth <= 0 || newHeight <= 0)
    {
        std::cerr << "Error: Invalid dimensions for resize\n";
        return;
    }

    image = resizeImageInMemory(image, newWidth, newHeight);
}

void morphOptimized(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor,
                    std::vector<int> &pixelMapping)
{
    int width = targetImage.width;
    int height = targetImage.height;
    int pixelCount = width * height;

    // Pre-calculate constants
    const double maxPosDistance = std::sqrt(static_cast<double>(width * width + height * height));
    const double posNormalizationFactor = 441.0 / maxPosDistance;

    // Create color buckets for faster search
    const int BUCKET_COUNT = Config::MORPH_BUCKET_SIZE * Config::MORPH_BUCKET_SIZE * Config::MORPH_BUCKET_SIZE;
    std::vector<std::vector<int>> colorBuckets(BUCKET_COUNT);

    // Populate buckets
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

    // Shuffle buckets for randomness
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    for (auto &bucket : colorBuckets)
    {
        std::shuffle(bucket.begin(), bucket.end(), rng);
    }

    std::vector<bool> pixelUsed(pixelCount, false);
    pixelMapping.resize(pixelCount);

    int pixelsProcessed = 0;

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int targetR = targetImage(col, row, 0);
            int targetG = targetImage(col, row, 1);
            int targetB = targetImage(col, row, 2);
            int weight = weightsImage(col, row, 0);
            int targetFlatIdx = row * width + col;

            // Adaptive search quality
            int maxChecks = Config::MORPH_BASE_CHECKS +
                            (weight * Config::MORPH_WEIGHT_CHECKS / 255);

            int targetBucketR = targetR / 16;
            int targetBucketG = targetG / 16;
            int targetBucketB = targetB / 16;

            double minDistance = std::numeric_limits<double>::max();
            int bestPixelIndex = -1;
            int checksPerformed = 0;
            bool foundMatch = false;

            // Search nearby color buckets
            for (int dr = -1; dr <= 1 && !foundMatch && checksPerformed < maxChecks; dr++)
            {
                for (int dg = -1; dg <= 1 && !foundMatch && checksPerformed < maxChecks; dg++)
                {
                    for (int db = -1; db <= 1 && !foundMatch && checksPerformed < maxChecks; db++)
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

                        for (size_t i = 0; i < colorBuckets[bucketIdx].size() &&
                                           !foundMatch && checksPerformed < maxChecks;
                             i++)
                        {
                            int srcFlatIdx = colorBuckets[bucketIdx][i];

                            if (!pixelUsed[srcFlatIdx])
                            {
                                checksPerformed++;

                                int srcRow = srcFlatIdx / width;
                                int srcCol = srcFlatIdx % width;

                                // Color distance
                                int r_diff = targetR - sourceImage(srcCol, srcRow, 0);
                                int g_diff = targetG - sourceImage(srcCol, srcRow, 1);
                                int b_diff = targetB - sourceImage(srcCol, srcRow, 2);
                                double colorDist = std::sqrt(static_cast<double>(
                                    r_diff * r_diff + g_diff * g_diff + b_diff * b_diff));

                                // Position distance
                                int pos_diff_col = col - srcCol;
                                int pos_diff_row = row - srcRow;
                                double positionDist = std::sqrt(static_cast<double>(
                                    pos_diff_col * pos_diff_col + pos_diff_row * pos_diff_row));
                                positionDist *= posNormalizationFactor;

                                // Combined distance with weight
                                double dist = (1.0 - blendFactor) * colorDist + blendFactor * positionDist;
                                dist *= (1.0 + static_cast<double>(weight) / 255.0);

                                if (dist < minDistance)
                                {
                                    minDistance = dist;
                                    bestPixelIndex = srcFlatIdx;

                                    if (dist < Config::MORPH_EARLY_STOP && weight < 128)
                                    {
                                        foundMatch = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Fallback: global search if no match found
            if (bestPixelIndex == -1)
            {
                int globalChecks = 0;

                for (size_t bucketIdx = 0; bucketIdx < colorBuckets.size() &&
                                           globalChecks < Config::MORPH_FALLBACK_CHECKS;
                     bucketIdx++)
                {
                    for (size_t i = 0; i < colorBuckets[bucketIdx].size() &&
                                       globalChecks < Config::MORPH_FALLBACK_CHECKS;
                         i++)
                    {
                        int srcFlatIdx = colorBuckets[bucketIdx][i];

                        if (!pixelUsed[srcFlatIdx])
                        {
                            globalChecks++;

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

void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage,
                   const std::string &outputPath, int frameCount, double blendFactor)
{
    std::cout << "Starting animated morph generation..." << std::endl;

    if (frameCount <= 1)
    {
        throw std::runtime_error("frameCount must be greater than 1 for animation");
    }

    // Resize images to match
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

    // Calculate pixel mapping
    std::vector<int> finalPixelMapping;
    morphOptimized(sourceImage, targetImage, weightsImage, blendFactor, finalPixelMapping);

    int totalFrames = Config::MORPH_HOLD_FRAMES + frameCount + Config::MORPH_HOLD_FRAMES;
    std::cout << "Generating " << totalFrames << " animation frames..." << std::endl;

    // Initialize GIF
    GifWriter g;
    GifBegin(&g, outputPath.c_str(), width, height, Config::MORPH_GIF_DELAY);

    // Pre-allocate frame buffers
    std::vector<uint8_t> initialFrameBuffer(width * height * 4);
    std::vector<uint8_t> finalFrameBuffer(width * height * 4);
    std::vector<uint8_t> currentFrameBuffer(width * height * 4);
    bool initialFrameSaved = false;
    bool finalFrameSaved = false;

    // Generate initial frame (pure source)
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
            double current_alpha = 1.0 * (1.0 - t) + blendFactor * t;

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

                    int warpedR = sourceImage(srcCol, srcRow, 0);
                    int warpedG = sourceImage(srcCol, srcRow, 1);
                    int warpedB = sourceImage(srcCol, srcRow, 2);

                    int targetR = targetImage(destCol, destRow, 0);
                    int targetG = targetImage(destCol, destRow, 1);
                    int targetB = targetImage(destCol, destRow, 2);

                    unsigned char R = static_cast<unsigned char>(std::clamp(
                        static_cast<int>(current_alpha * warpedR + (1.0 - current_alpha) * targetR), 0, 255));
                    unsigned char G = static_cast<unsigned char>(std::clamp(
                        static_cast<int>(current_alpha * warpedG + (1.0 - current_alpha) * targetG), 0, 255));
                    unsigned char B = static_cast<unsigned char>(std::clamp(
                        static_cast<int>(current_alpha * warpedB + (1.0 - current_alpha) * targetB), 0, 255));

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