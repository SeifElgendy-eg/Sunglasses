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
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>

void grayscale(Image &image);
void bnw(Image &image);
void invert(Image &image);
void merge(Image &image1, Image &image2, Image &outputImage, float alpha, char mode);
void reflect(Image &image);
void rotate(Image &image, int degrees);
void dnl(Image &image, int percent);
void crop(Image &image, int x, int y, int width, int height);
void frame(Image &image, int thickness, int r, int g, int b, char style = 's');
void edges(Image &image);
void blur(Image &image, int kernelSize);
void resizeImage(Image &image, const std::string &imageName, int newWidth = -1, int newHeight = -1,
                 double scaleFactorX = -1, double scaleFactorY = -1);

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
    std::cout << "  --resize [w h] [sx sy]   Resize image by width/height or scale factors\n";
    std::cout << "  -o <output_file>         Specify output filename (default: output.png)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << " input.png --grayscale -o gray.png\n";
    std::cout << "  " << programName << " input.png --rotate 90 -o rotated.png\n";
    std::cout << "  " << programName << " input.png --rotate -o rotated.png  # uses default 90\n";
    std::cout << "  " << programName << " input.png --blur 5 -o blurred.png\n";
    std::cout << "  " << programName << " input.png --frame -o framed.png  # default black frame\n";
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

    std::string outputFile = "output.png";
    bool filterApplied = false;

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

                double scaleFactorX = static_cast<double>(img.width) / newWidth;
                double scaleFactorY = static_cast<double>(img.height) / newHeight;

                Image resizedImage(newWidth, newHeight);

                for (int row = 0; row < newHeight; row++)
                {
                    for (int col = 0; col < newWidth; col++)
                    {
                        for (int k = 0; k <= 2; k++)
                        {
                            const int oldX = static_cast<int>(round(col * scaleFactorX));
                            const int oldY = static_cast<int>(round(row * scaleFactorY));

                            if (oldX >= 0 && oldX < img.width && oldY >= 0 && oldY < img.height)
                            {
                                resizedImage(col, row, k) = img(oldX, oldY, k);
                            }
                        }
                    }
                }
                img = resizedImage;
                filterApplied = true;
            }
            else
            {
                std::cerr << "Error: --resize requires 2 values: newWidth newHeight\n";
                return -1;
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

    if (!filterApplied)
    {
        std::cerr << "Warning: No filter applied. Use --help to see available options.\n";
    }

    img.saveImage(outputFile.c_str());
    std::cout << "Image saved to: " << outputFile << std::endl;

    return 0;
}

void grayscale(Image &image)
{
    // Iterate through each pixel in the image
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            // Calculate average of RGB values and round to nearest integer
            int num = round((image(col, row, 0) + image(col, row, 1) + image(col, row, 2)) / 3.0);

            // Set all three color channels to the same grayscale value
            image(col, row, 0) = image(col, row, 1) = image(col, row, 2) = num;
        }
    }
}

void bnw(Image &image)
{
    // Iterate through each pixel in the image
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            // Calculate average of RGB values (integer division for speed)
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

void crop(Image &image, int x, int y, int width, int height)
{
    // Validate input parameters
    if (x < 0 || y < 0 || width <= 0 || height <= 0)
    {
        std::cerr << "Error: Invalid crop parameters. x, y must be non-negative, width and height must be positive." << std::endl;
        return;
    }

    if (x >= image.width || y >= image.height)
    {
        std::cerr << "Error: Starting coordinates (" << x << ", " << y << ") are outside image bounds ("
                  << image.width << " x " << image.height << ")." << std::endl;
        return;
    }

    if (x + width > image.width || y + height > image.height)
    {
        std::cerr << "Error: Crop area extends beyond image bounds. Max crop size from ("
                  << x << ", " << y << ") is " << (image.width - x) << " x " << (image.height - y) << "." << std::endl;
        return;
    }

    if (width > image.width || height > image.height)
    {
        std::cerr << "Error: Crop dimensions (" << width << " x " << height
                  << ") exceed original image dimensions (" << image.width << " x " << image.height << ")." << std::endl;
        return;
    }

    // Create cropped image
    Image cropped(width, height);

    // Copy the specified region
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            // Copy RGB channels
            cropped(col, row, 0) = image(x + col, y + row, 0); // Red
            cropped(col, row, 1) = image(x + col, y + row, 1); // Green
            cropped(col, row, 2) = image(x + col, y + row, 2); // Blue
        }
    }
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

void blur(Image &image, int kernelSize)
{
    // Ensure minimum kernel size
    if (kernelSize < 1)
        kernelSize = 1;

    // Ensure odd kernel size for symmetry around center pixel
    if (kernelSize % 2 == 0)
        kernelSize++;

    int radius = kernelSize / 2; // Distance from center to edge of kernel

    // Create a copy of the original image to read from during processing
    Image copy = image;

    // Process each pixel in the image
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            int red = 0, green = 0, blue = 0, count = 0;

            // Iterate through kernel area around current pixel
            for (int dr = -radius; dr <= radius; dr++)
            {
                for (int dc = -radius; dc <= radius; dc++)
                {
                    int nr = row + dr; // Neighbor row
                    int nc = col + dc; // Neighbor column

                    // Check if neighbor coordinates are within image bounds
                    if (nr >= 0 && nr < image.height && nc >= 0 && nc < image.width)
                    {
                        // Accumulate color values from neighboring pixels
                        red += copy(nc, nr, 0);
                        green += copy(nc, nr, 1);
                        blue += copy(nc, nr, 2);
                        count++; // Count valid neighbors for averaging
                    }
                }
            }

            // Set pixel to average of all valid neighbors
            image(col, row, 0) = round((float)red / count);   // Red
            image(col, row, 1) = round((float)green / count); // Green
            image(col, row, 2) = round((float)blue / count);  // Blue
        }
    }
}

// frame function with RGB values
void frame(Image &image, int thickness, int r, int g, int b, char style)
{
    // Validate frame thickness
    if (thickness <= 0)
    {
        std::cerr << "Error: Frame thickness must be positive." << std::endl;
        return;
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

    // Process each pixel in the image
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            bool inFrame = false;

            if (style == 's') // Simple/solid frame
            {
                // Check if pixel is within thickness distance from any edge
                inFrame = (row < thickness || row >= image.height - thickness || col < thickness || col >= image.width - thickness);
            }

            if (inFrame)
            {
                image(col, row, 0) = r;
                image(col, row, 1) = g;
                image(col, row, 2) = b;
            }
        }
    }
}

void resizeImage(Image &image, const std::string &imageName, int newWidth, int newHeight,
                 double scaleFactorX, double scaleFactorY)
{
    image.loadNewImage(imageName + ".jpg");

    /// Getting the scale factors
    if (scaleFactorX > 0 && scaleFactorY > 0)
    {
        newWidth = static_cast<int>(scaleFactorX * image.width);
        newHeight = static_cast<int>(scaleFactorY * image.height);

        /// The scale Factors must be inverted to compute old x,y correclty
        scaleFactorX = 1 / scaleFactorX;
        scaleFactorY = 1 / scaleFactorY;

        /// Kepping the aspect ratio when only one scaling factor is given
    }
    else if (scaleFactorX > 0 && scaleFactorY == -1)
    {
        newWidth = static_cast<int>(scaleFactorX * image.width);
        scaleFactorX = 1 / scaleFactorX;
        scaleFactorY = scaleFactorX;
        newHeight = static_cast<int>(1 / scaleFactorY * image.height);
    }
    else if (scaleFactorX == -1 && scaleFactorY > 0)
    {
        newHeight = static_cast<int>(scaleFactorY * image.height);
        scaleFactorY = 1 / scaleFactorY;
        scaleFactorX = scaleFactorY;
        newWidth = static_cast<int>(1 / scaleFactorX * image.width);
    }
    else if (newWidth > 0 && newHeight > 0)
    {
        scaleFactorX = static_cast<double>(image.width) / newWidth;
        scaleFactorY = static_cast<double>(image.height) / newHeight;
    }
    else
    {
        throw std::invalid_argument("You must provide Either new width and height or x,y scaling factors");
    }

    Image resizedImage(newWidth, newHeight);

    for (int i = 0; i < resizedImage.width; i++)
    {
        for (int j = 0; j < resizedImage.height; j++)
        {
            for (int k = 0; k <= 2; k++)
            {
                /// Locating old pixels to be copied
                const int oldX = static_cast<int>(round(i * scaleFactorX));
                const int oldY = static_cast<int>(round(j * scaleFactorY));

                /// Nearest Neighbor Interpolation Applied
                resizedImage(i, j, k) = image(oldX, oldY, k);
            }
        }
    }
    resizedImage.saveImage(imageName + "_output_resized.jpg");
}
