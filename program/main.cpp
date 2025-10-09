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
    std::cout << "  " << programName << " source.png --morph target.png --animate 60 -o anim.gif\n";
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
            if (i + 1 < argc)
            {
                int degrees = std::atoi(argv[++i]);
                rotate(img, degrees);
                filterApplied = true;
            }
            else
            {
                rotate(img, 90); // default: 90 degrees
                filterApplied = true;
            }
            break;

        case 'l': // lighten
            if (i + 1 < argc)
            {
                int percent = std::atoi(argv[++i]);
                dnl(img, percent);
                filterApplied = true;
            }
            else
            {
                dnl(img, 20); // default: lighten by 20%
                filterApplied = true;
            }
            break;

        case 'd': // darken
            if (i + 1 < argc)
            {
                int percent = std::atoi(argv[++i]);
                dnl(img, -percent);
                filterApplied = true;
            }
            else
            {
                dnl(img, -20); // default: darken by 20%
                filterApplied = true;
            }
            break;

        case 'c': // crop
            if (i + 4 < argc)
            {
                int x = std::atoi(argv[++i]);
                int y = std::atoi(argv[++i]);
                int w = std::atoi(argv[++i]);
                int h = std::atoi(argv[++i]);
                crop(img, x, y, w, h);
                filterApplied = true;
            }
            else
            {
                // default: crop to center 50% of image
                int w = img.width / 2;
                int h = img.height / 2;
                int x = img.width / 4;
                int y = img.height / 4;
                crop(img, x, y, w, h);
                filterApplied = true;
            }
            break;

        case 'f': // frame
            if (i + 4 < argc)
            {
                int thickness = std::atoi(argv[++i]);
                int r = std::atoi(argv[++i]);
                int g = std::atoi(argv[++i]);
                int b = std::atoi(argv[++i]);
                frame(img, thickness, r, g, b);
                filterApplied = true;
            }
            else
            {
                frame(img, 20, 0, 0, 0); // default: 20px black frame
                filterApplied = true;
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
                morphBlendFactor = std::clamp(morphBlendFactor, 0.0, 1.0);
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
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                animateFrames = std::atoi(argv[++i]);
                animateFrames = std::max(2, animateFrames);
            }
            std::cout << "Animation mode enabled with " << animateFrames << " frames\n";
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

    if (!filterApplied)
    {
        std::cerr << "Warning: No filter applied. Use --help to see available options.\n";
    }

    // Execute morph if pending (after all flags are parsed)
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
            // Calculate average of RGB values
            int num = (image(col, row, 0) + image(col, row, 1) + image(col, row, 2)) / 3;

            // Apply threshold: >= 128 becomes white, < 128 becomes black
            if (num >= 128)
                num = 255; // White
            else
                num = 0; // Black

            // Set all three color channels to the same binary value
            image(col, row, 0) = image(col, row, 1) = image(col, row, 2) = num;
        }
    }
}

void invert(Image &image)
{
    // Iterate through each pixel in the image
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            // Invert each color channel: new_value = 255 - old_value
            image(col, row, 0) = 255 - image(col, row, 0); // Red
            image(col, row, 1) = 255 - image(col, row, 1); // Green
            image(col, row, 2) = 255 - image(col, row, 2); // Blue
        }
    }
}

void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode)
{
    // Determine output image dimensions based on merge mode
    if (mode == 'f')
    {
        // Use first image dimensions
        outputImage = Image(image1.width, image1.height);
    }
    else if (mode == 's')
    {
        // Use second image dimensions
        outputImage = Image(image2.width, image2.height);
    }
    else
    {
        // Use minimum dimensions of both images
        outputImage = Image(std::min(image1.width, image2.width),
                            std::min(image1.height, image2.height));
    }

    // Perform alpha blending for each pixel
    for (int row = 0; row < outputImage.height; row++)
    {
        for (int col = 0; col < outputImage.width; col++)
        {
            // Process each color channel (RGB)
            for (int c = 0; c < 3; c++)
            {
                // Get pixel values, using 0 if coordinates are out of bounds
                int val1 = (row < image1.height && col < image1.width) ? image1(col, row, c) : 0;
                int val2 = (row < image2.height && col < image2.width) ? image2(col, row, c) : 0;

                // Alpha blending formula: result = alpha * val1 + (1-alpha) * val2
                int mergedValue = static_cast<int>(alpha * val1 + (1 - alpha) * val2);

                // Clamp result to valid color range [0, 255]
                mergedValue = std::clamp(mergedValue, 0, 255);
                outputImage(col, row, c) = mergedValue;
            }
        }
    }
}

void reflect(Image &image)
{
    // Iterate through each row
    for (int row = 0; row < image.height; row++)
    {
        // Only process half the width to avoid double-swapping
        for (int col = 0; col < image.width / 2; col++)
        {
            // Swap all three color channels
            for (int k = 0; k < 3; k++)
            {
                // Temporarily store left pixel value
                unsigned int temp = image(col, row, k);

                // Copy right pixel to left position
                image(col, row, k) = image((image.width - 1 - col), row, k);

                // Copy stored left pixel to right position
                image((image.width - 1 - col), row, k) = temp;
            }
        }
    }
    return;
}

void rotate(Image &image, int degrees)
{
    // Normalize degrees to [0, 360) range
    degrees = degrees % 360;
    if (degrees < 0)
        degrees += 360;

    // Calculate number of 90-degree rotations needed
    int numRotations = degrees / 90;
    numRotations = numRotations % 4; // Normalize to [0, 3]

    // Perform rotation by applying 90-degree rotations sequentially
    for (int r = 0; r < numRotations; r++)
    {
        // Create new image with swapped dimensions (90-degree rotation effect)
        Image rotated(image.height, image.width);

        // Copy pixels with 90-degree clockwise rotation transformation
        for (int row = 0; row < image.height; row++)
        {
            for (int col = 0; col < image.width; col++)
            {
                // Rotation formula: (x,y) -> (height-1-y, x)
                rotated(image.height - 1 - row, col, 0) = image(col, row, 0); // Red
                rotated(image.height - 1 - row, col, 1) = image(col, row, 1); // Green
                rotated(image.height - 1 - row, col, 2) = image(col, row, 2); // Blue
            }
        }

        // Replace original image with rotated version
        image = rotated;
    }
}

void dnl(Image &image, int percent)
{
    // Process each pixel in the image
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            // Process each color channel
            for (int c = 0; c < 3; c++)
            {
                int value = image(col, row, c);

                // Apply percentage adjustment: new_value = value + (value * percent / 100)
                int newValue = value + (value * percent) / 100;

                // Clamp to valid color range [0, 255]
                if (newValue > 255)
                    newValue = 255;
                if (newValue < 0)
                    newValue = 0;

                image(col, row, c) = newValue;
            }
        }
    }
}

bool crop(Image &image, int x, int y, int width, int height)
{
    // Validate input parameters
    if (x < 0 || y < 0 || width <= 0 || height <= 0)
    {
        std::cerr << "Error: Invalid crop parameters. x, y must be non-negative, width and height must be positive." << std::endl;
        return;
    }

    // Adjust crop area to fit within image bounds
    int startX = std::max(0, x);
    int startY = std::max(0, y);
    int cropWidth = std::min(width, image.width - startX);
    int cropHeight = std::min(height, image.height - startY);

    if (startX >= image.width || startY >= image.height || cropWidth <= 0 || cropHeight <= 0)
    {
        std::cerr << "Error: Crop area is outside image bounds\n";
        return false;
    }

    // Create cropped image
    Image cropped(cropWidth, cropHeight);

    // Copy the specified region
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
    // Create a copy of the original image to read from during processing
    Image copy = image;

    // Sobel kernel for detecting vertical edges (Gx - horizontal gradients)
    int Gx[3][3] = {{-1, 0, 1},
                    {-2, 0, 2},
                    {-1, 0, 1}};

    // Sobel kernel for detecting horizontal edges (Gy - vertical gradients)
    int Gy[3][3] = {{-1, -2, -1},
                    {0, 0, 0},
                    {1, 2, 1}};

    // Process each pixel in the image
    for (int i = 0; i < image.height; i++)
    {
        for (int j = 0; j < image.width; j++)
        {
            // Initialize gradient accumulations for each color channel
            int Ix_red = 0, Iy_red = 0;
            int Ix_green = 0, Iy_green = 0;
            int Ix_blue = 0, Iy_blue = 0;

            // Apply 3x3 Sobel kernels around current pixel
            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    // Calculate neighbor pixel coordinates
                    int ni = i + di;
                    int nj = j + dj;

                    // Check if neighbor coordinates are within image bounds
                    if (ni >= 0 && ni < image.height && nj >= 0 && nj < image.width)
                    {
                        int gx_val = Gx[di + 1][dj + 1]; // Horizontal gradient kernel value
                        int gy_val = Gy[di + 1][dj + 1]; // Vertical gradient kernel value

                        // Accumulate gradients for red channel
                        Ix_red += copy(nj, ni, 0) * gx_val;
                        Iy_red += copy(nj, ni, 0) * gy_val;

                        // Accumulate gradients for green channel
                        Ix_green += copy(nj, ni, 1) * gx_val;
                        Iy_green += copy(nj, ni, 1) * gy_val;

                        // Accumulate gradients for blue channel
                        Ix_blue += copy(nj, ni, 2) * gx_val;
                        Iy_blue += copy(nj, ni, 2) * gy_val;
                    }
                }
            }

            // Calculate gradient magnitude for each color channel
            int mag_red = round(sqrt(Ix_red * Ix_red + Iy_red * Iy_red));
            int mag_green = round(sqrt(Ix_green * Ix_green + Iy_green * Iy_green));
            int mag_blue = round(sqrt(Ix_blue * Ix_blue + Iy_blue * Iy_blue));

            // Clamp magnitude values to valid color range [0, 255] and assign to output image
            image(j, i, 0) = (mag_red > 255) ? 255 : mag_red;
            image(j, i, 1) = (mag_green > 255) ? 255 : mag_green;
            image(j, i, 2) = (mag_blue > 255) ? 255 : mag_blue;
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

// frame function with RGB values

bool frame(Image &image, int thickness, int r, int g, int b, char style)
{
    // Validate frame thickness
    if (thickness <= 0)
    {
        std::cerr << "Error: Frame thickness must be positive\n";
        return false;
    }

    // Ensure thickness doesn't exceed half of either dimension
    int maxThickness = std::min(image.width, image.height) / 2;
    if (thickness > maxThickness)
    {
        std::cerr << "Warning: Frame thickness too large for image. Using maximum: " << maxThickness << std::endl;
        thickness = maxThickness;
    }

    // Clamp RGB values to valid range [0, 255]
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

// Helper function to resize an image in memory (reusable for any filter)
Image resizeImageInMemory(Image &image, int newWidth, int newHeight)
{
    double scaleFactorX = static_cast<double>(image.width) / newWidth;
    double scaleFactorY = static_cast<double>(image.height) / newHeight;
    Image resizedImage(newWidth, newHeight);

    for (int row = 0; row < newHeight; row++)
    {
        for (int col = 0; col < newWidth; col++)
        {
            for (int k = 0; k <= 2; k++)
            {
                const int oldX = static_cast<int>(round(col * scaleFactorX));
                const int oldY = static_cast<int>(round(row * scaleFactorY));
                if (oldX >= 0 && oldX < image.width && oldY >= 0 && oldY < image.height)
                {
                    resizedImage(col, row, k) = image(oldX, oldY, k);
                }
            }
        }
    }
    return resizedImage;
}

void resizeImage(Image &image, const std::string &imageName, int newWidth, int newHeight,
                 double scaleFactorX, double scaleFactorY)
{
    // Determine new dimensions if not explicitly provided
    if (newWidth == -1 && scaleFactorX != -1)
        newWidth = static_cast<int>(image.width * scaleFactorX);
    if (newHeight == -1 && scaleFactorY != -1)
        newHeight = static_cast<int>(image.height * scaleFactorY);

    // Handle scaling factor inversion for new logic (copied from user's provided logic)
    if (scaleFactorX > 0 && scaleFactorY > 0)
    {
        newWidth = static_cast<int>(scaleFactorX * image.width);
        newHeight = static_cast<int>(scaleFactorY * image.height);
    }
    else if (scaleFactorX > 0 && scaleFactorY == -1)
    {
        newWidth = static_cast<int>(scaleFactorX * image.width);
        newHeight = static_cast<int>((double)newWidth / image.width * image.height); // Maintain aspect ratio
    }
    else if (scaleFactorX == -1 && scaleFactorY > 0)
    {
        newHeight = static_cast<int>(scaleFactorY * image.height);
        newWidth = static_cast<int>((double)newHeight / image.height * image.width); // Maintain aspect ratio
    }
    else if (newWidth <= 0 || newHeight <= 0)
    {
        std::cerr << "Error: Invalid dimensions for resize." << std::endl;
        return;
    }

    image = resizeImageInMemory(image, newWidth, newHeight);
}

void morph(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor)
{
    // Step 1: Resize images to match target dimensions
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

    // Step 3: Apply pixel mapping (WARP) and then BLEND the warped source with the target image
    Image morphedImage(targetImage.width, targetImage.height);
    int width = targetImage.width;
    double alpha = blendFactor; // Blend factor: 1.0 = source, 0.0 = target

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

void morphOptimized(Image &sourceImage, Image &targetImage, Image &weightsImage, double blendFactor,
                    std::vector<int> &pixelMapping)
{
    int width = targetImage.width;
    int height = targetImage.height;
    int pixelCount = width * height;

    // Step 1: Create color buckets for faster search (quantize to 16x16x16 = 4096 buckets)
    const int BUCKET_SIZE = 16; // Divide each RGB channel into 16 levels
    const int BUCKET_COUNT = BUCKET_SIZE * BUCKET_SIZE * BUCKET_SIZE;

    std::vector<std::vector<int>> colorBuckets(BUCKET_COUNT);

    // Populate buckets with source pixel indices
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            int idx = row * width + col;
            int r = sourceImage(col, row, 0) / 16; // Quantize 0-255 to 0-15
            int g = sourceImage(col, row, 1) / 16;
            int b = sourceImage(col, row, 2) / 16;
            int bucketIdx = (r * BUCKET_SIZE * BUCKET_SIZE) + (g * BUCKET_SIZE) + b;
            colorBuckets[bucketIdx].push_back(idx);
        }
    }

    // Step 2: Shuffle buckets for randomness
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    for (auto &bucket : colorBuckets)
    {
        std::shuffle(bucket.begin(), bucket.end(), rng);
    }

    std::vector<bool> pixelUsed(pixelCount, false);
    pixelMapping.resize(pixelCount);

    // Step 3: Process each target pixel with optimized search
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

            // Determine search quality based on weight
            // High weight areas get more thorough search
            int maxChecks = 200 + (weight * 800 / 255); // 200-1000 checks based on weight

            // Calculate target color bucket
            int targetBucketR = targetR / 16;
            int targetBucketG = targetG / 16;
            int targetBucketB = targetB / 16;

            double minDistance = std::numeric_limits<double>::max();
            int bestPixelIndex = -1;
            int checksPerformed = 0;
            bool foundMatch = false;

            // Search strategy: Check nearby color buckets first
            for (int dr = -1; dr <= 1 && !foundMatch && checksPerformed < maxChecks; dr++)
            {
                for (int dg = -1; dg <= 1 && !foundMatch && checksPerformed < maxChecks; dg++)
                {
                    for (int db = -1; db <= 1 && !foundMatch && checksPerformed < maxChecks; db++)
                    {
                        int br = targetBucketR + dr;
                        int bg = targetBucketG + dg;
                        int bb = targetBucketB + db;

                        // Bounds checking
                        if (br < 0 || br >= BUCKET_SIZE || bg < 0 || bg >= BUCKET_SIZE ||
                            bb < 0 || bb >= BUCKET_SIZE)
                            continue;

                        int bucketIdx = (br * BUCKET_SIZE * BUCKET_SIZE) + (bg * BUCKET_SIZE) + bb;

                        // Search this bucket
                        for (size_t i = 0; i < colorBuckets[bucketIdx].size() && !foundMatch && checksPerformed < maxChecks; i++)
                        {
                            int srcFlatIdx = colorBuckets[bucketIdx][i];

                            if (!pixelUsed[srcFlatIdx])
                            {
                                checksPerformed++;

                                int srcRow = srcFlatIdx / width;
                                int srcCol = srcFlatIdx % width;

                                int sourceR = sourceImage(srcCol, srcRow, 0);
                                int sourceG = sourceImage(srcCol, srcRow, 1);
                                int sourceB = sourceImage(srcCol, srcRow, 2);

                                // Calculate distance
                                double r_diff = targetR - sourceR;
                                double g_diff = targetG - sourceG;
                                double b_diff = targetB - sourceB;
                                double colorDist = std::sqrt(r_diff * r_diff + g_diff * g_diff + b_diff * b_diff);

                                // Position distance calculation (Euclidean distance is better than flat index difference)
                                double pos_diff_col = col - srcCol;
                                double pos_diff_row = row - srcRow;
                                double positionDist = std::sqrt(pos_diff_col * pos_diff_col + pos_diff_row * pos_diff_row);

                                // Normalize position distance to similar scale as color (max color diff is ~441)
                                double max_pos_dist = std::sqrt(width * width + height * height);
                                positionDist = positionDist / max_pos_dist * 441.0;

                                double dist = (1.0 - blendFactor) * colorDist + blendFactor * positionDist;
                                dist = dist * (1.0 + (double)weight / 255.0);

                                if (dist < minDistance)
                                {
                                    minDistance = dist;
                                    bestPixelIndex = srcFlatIdx;

                                    // Early stopping: if distance is very good, accept it
                                    if (dist < 10.0 && weight < 128)
                                    {
                                        foundMatch = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // If no match found in nearby buckets, do a limited global search (Fallback logic)
            if (bestPixelIndex == -1)
            {
                int globalChecks = 0;
                int maxGlobalChecks = 500;

                for (size_t bucketIdx = 0; bucketIdx < colorBuckets.size() && globalChecks < maxGlobalChecks; bucketIdx++)
                {
                    for (size_t i = 0; i < colorBuckets[bucketIdx].size() && globalChecks < maxGlobalChecks; i++)
                    {
                        int srcFlatIdx = colorBuckets[bucketIdx][i];

                        if (!pixelUsed[srcFlatIdx])
                        {
                            globalChecks++;

                            int srcRow = srcFlatIdx / width;
                            int srcCol = srcFlatIdx % width;

                            int sourceR = sourceImage(srcCol, srcRow, 0);
                            int sourceG = sourceImage(srcCol, srcRow, 1);
                            int sourceB = sourceImage(srcCol, srcRow, 2);

                            double r_diff = targetR - sourceR;
                            double g_diff = targetG - sourceG;
                            double b_diff = targetB - sourceB;
                            double colorDist = std::sqrt(r_diff * r_diff + g_diff * g_diff + b_diff * b_diff);

                            double pos_diff_col = col - srcCol;
                            double pos_diff_row = row - srcRow;
                            double positionDist = std::sqrt(pos_diff_col * pos_diff_col + pos_diff_row * pos_diff_row);
                            double max_pos_dist = std::sqrt(width * width + height * height);
                            positionDist = positionDist / max_pos_dist * 441.0;

                            double dist = (1.0 - blendFactor) * colorDist + blendFactor * positionDist;
                            dist = dist * (1.0 + (double)weight / 255.0);

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
                pixelMapping[targetFlatIdx] = targetFlatIdx; // Fallback
            }

            // Progress indicator
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

// Animated morph function - creates a GIF showing pixel movement
void morphAnimated(Image &sourceImage, Image &targetImage, Image &weightsImage, const std::string &outputPath,
                   int frameCount, double blendFactor)
{
    std::cout << "Starting animated morph generation..." << std::endl;

    if (frameCount <= 1)
    {
        std::cerr << "Error: frameCount must be greater than 1 for an animation." << std::endl;
        return;
    }

    // Step 1: Resize images to match target dimensions
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

    // Step 2: Calculate the final pixel map (Inverse Map: target_index -> source_index)
    std::vector<int> finalPixelMapping;
    morphOptimized(sourceImage, targetImage, weightsImage, blendFactor, finalPixelMapping);

    // Number of frames to hold the initial and final results
    const int HOLD_FRAMES = 15; // Increased hold time at both ends

    // The actual morphing happens over 'frameCount' frames.
    // Total frames = (Start Hold) + (Morphing Frames) + (End Hold)
    int totalFrames = HOLD_FRAMES + frameCount + HOLD_FRAMES;

    std::cout << "Generating " << totalFrames << " animation frames..." << std::endl;

    // Step 3: Create GIF
    GifWriter g;
    // 💡 FIX for Quality: Increase delay to 5 centiseconds (50ms per frame, 20 FPS).
    // This often improves perceived quality and reduces the need for the GIF library to skip frames.
    int delay = 5;
    GifBegin(&g, outputPath.c_str(), width, height, delay);

    // Create a buffer to store the initial and final frames for re-use
    std::vector<uint8_t> initialFrameBuffer(width * height * 4);
    std::vector<uint8_t> finalFrameBuffer(width * height * 4);
    bool initialFrameSaved = false;

    // Generate frames
    for (int frame = 0; frame < totalFrames; frame++)
    {
        std::vector<uint8_t> currentFrameBuffer(width * height * 4);

        // Determine the progress factor 't' (0.0 to 1.0) for the actual morph
        float t;
        if (frame < HOLD_FRAMES)
        {
            t = 0.0f; // Start Hold: Keep the image at t=0
        }
        else if (frame >= HOLD_FRAMES + frameCount)
        {
            t = 1.0f; // End Hold: Keep the image at t=1
        }
        else
        {
            // Morphing in progress
            int morphFrame = frame - HOLD_FRAMES;
            t = (float)morphFrame / (frameCount - 1);
        }

        // Interpolate blend factor (starts at 1.0/pure source, ends at final blendFactor)
        double current_alpha = 1.0 * (1.0 - t) + blendFactor * t;

        // --- Use stored frame buffers for hold segments ---
        if (frame < HOLD_FRAMES)
        {
            // Write initial frame (Source Image)
            if (!initialFrameSaved)
            {
                // Generate and save the true initial frame (t=0)
                // This frame is pure Source Warped at t=0, blended with Target at 1.0 alpha

                // When t=0: sampleCol = destCol, sampleRow = destRow.
                // Warped Source Color = sourceImage(destCol, destRow).

                for (int destRow = 0; destRow < height; destRow++)
                {
                    for (int destCol = 0; destCol < width; destCol++)
                    {
                        int framePos = (destRow * width + destCol) * 4;

                        // When t=0, current_alpha=1.0. Blended R = 1.0 * sourceR + 0.0 * targetR = sourceR
                        // This ensures the frame is the original source image, which is the intention.
                        unsigned char R = sourceImage(destCol, destRow, 0);
                        unsigned char G = sourceImage(destCol, destRow, 1);
                        unsigned char B = sourceImage(destCol, destRow, 2);

                        initialFrameBuffer[framePos + 0] = R;
                        initialFrameBuffer[framePos + 1] = G;
                        initialFrameBuffer[framePos + 2] = B;
                        initialFrameBuffer[framePos + 3] = 255;
                    }
                }
                initialFrameSaved = true;
            }
            // Use the saved initial frame
            GifWriteFrame(&g, initialFrameBuffer.data(), width, height, delay);
        }
        else if (frame >= HOLD_FRAMES + frameCount)
        {
            // Write final frame (Morph Result) - the buffer for this was saved in the loop below
            GifWriteFrame(&g, finalFrameBuffer.data(), width, height, delay);
        }
        else
        {
            // --- GENERATE FRAME (Morphing in progress: HOLD_FRAMES <= frame < HOLD_FRAMES + frameCount) ---

            // Iterate over the destination pixel (target image coordinates)
            for (int destRow = 0; destRow < height; destRow++)
            {
                for (int destCol = 0; destCol < width; destCol++)
                {
                    int destIdx = destRow * width + destCol;
                    int finalSourceIdx = finalPixelMapping[destIdx];

                    int finalSrcRow = finalSourceIdx / width;
                    int finalSrcCol = finalSourceIdx % width;

                    // Interpolated sampling coordinates (Inverse Warp)
                    double sampleCol = destCol * (1.0 - t) + finalSrcCol * t;
                    double sampleRow = destRow * (1.0 - t) + finalSrcRow * t;

                    int srcCol = static_cast<int>(std::round(sampleCol));
                    int srcRow = static_cast<int>(std::round(sampleRow));

                    // Clamp to bounds
                    srcCol = std::clamp(srcCol, 0, width - 1);
                    srcRow = std::clamp(srcRow, 0, height - 1);

                    // Get Warped Source Color (sampled at interpolated position)
                    int warpedR = sourceImage(srcCol, srcRow, 0);
                    int warpedG = sourceImage(srcCol, srcRow, 1);
                    int warpedB = sourceImage(srcCol, srcRow, 2);

                    // Get Target Color (at the current pixel position)
                    int targetR = targetImage(destCol, destRow, 0);
                    int targetG = targetImage(destCol, destRow, 1);
                    int targetB = targetImage(destCol, destRow, 2);

                    // Color Blending
                    unsigned char R = static_cast<unsigned char>(
                        std::clamp(static_cast<int>(current_alpha * warpedR + (1.0 - current_alpha) * targetR), 0, 255));
                    unsigned char G = static_cast<unsigned char>(
                        std::clamp(static_cast<int>(current_alpha * warpedG + (1.0 - current_alpha) * targetG), 0, 255));
                    unsigned char B = static_cast<unsigned char>(
                        std::clamp(static_cast<int>(current_alpha * warpedB + (1.0 - current_alpha) * targetB), 0, 255));

                    // Write RGBA data to the buffer
                    int framePos = (destRow * width + destCol) * 4;

                    currentFrameBuffer[framePos + 0] = R;
                    currentFrameBuffer[framePos + 1] = G;
                    currentFrameBuffer[framePos + 2] = B;
                    currentFrameBuffer[framePos + 3] = 255;

                    // If this is the true final frame (t=1), save its data for the hold frames
                    if (frame == HOLD_FRAMES + frameCount - 1)
                    {
                        finalFrameBuffer[framePos + 0] = R;
                        finalFrameBuffer[framePos + 1] = G;
                        finalFrameBuffer[framePos + 2] = B;
                        finalFrameBuffer[framePos + 3] = 255;
                    }
                }
            }

            // Write generated frame to GIF
            GifWriteFrame(&g, currentFrameBuffer.data(), width, height, delay);
        }

        // Progress indicator
        if ((frame + 1) % 10 == 0 || frame == totalFrames - 1)
        {
            std::cout << "Progress: " << (frame + 1) << "/" << totalFrames << " frames\r" << std::flush;
        }
    }

    GifEnd(&g);
    std::cout << "\nAnimated GIF creation complete!" << std::endl;
}