#include <Image_Class.h>
#include <iostream>
#include <cmath>

void grayscale(Image &image);
void bnw(Image &image);
void invert(Image &image);
void reflect(Image &image);
void rotate(Image &image, int degrees);
void dnl(Image &image, int percent);
void blur(Image &image, int kernelSize);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <image_filename>" << std::endl;
        return -1;
    }

    Image img(argv[1]);
    if (img.imageData == nullptr) // Check loading of the image.
        return -2;

    // grayscale(img);
    // img.saveImage("grayscale_output.png");

    // bnw(img);
    // img.saveImage("bnw_output.png");

    // invert(img);
    // img.saveImage("inverted_output.png");

    // reflect(img);
    // img.saveImage("reflected_output.png");

    // rotate(img, 90);
    // img.saveImage("rotated_output.png");

    dnl(img, 20); // Lighten by 20%
    img.saveImage("lightened_output.png");
    dnl(img, -40); // Lighten by 20%
    img.saveImage("darkened_output.png");

    // blur(img, 10);
    // img.saveImage("blurred_output.png");

    return 0;
}

void grayscale(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            int num = round((image(col, row, 0) + image(col, row, 1) + image(col, row, 2)) / 3.0);
            image(col, row, 0) = image(col, row, 1) = image(col, row, 2) = num;
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
            if (num >= 128)
                num = 255;
            else
                num = 0;
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

void reflect(Image &image)
{
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width / 2; col++)
        {
            for (int k = 0; k < 3; k++)
            {
                unsigned int temp = image(col, row, k);
                image(col, row, k) = image((image.width - 1 - col), row, k);
                image((image.width - 1 - col), row, k) = temp;
            }
        }
    }
    return;
}

void rotate(Image &image, int degrees)
{
    degrees = degrees % 360;
    if (degrees < 0)
        degrees += 360;

    int numRotations = degrees / 90;
    numRotations = numRotations % 4; // Normalize to [0, 3]

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

// darken and lighten image
void dnl(Image &image, int percent)
{
    // percent: positive to lighten, negative to darken
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            for (int c = 0; c < 3; c++)
            {
                int value = image(col, row, c);
                int newValue = value + (value * percent) / 100;
                if (newValue > 255)
                    newValue = 255;
                if (newValue < 0)
                    newValue = 0;
                image(col, row, c) = newValue;
            }
        }
    }
}

void blur(Image &image, int kernelSize)
{
    if (kernelSize < 1)
        kernelSize = 1;
    if (kernelSize % 2 == 0)
        kernelSize++; // Ensure odd size for symmetry
    int radius = kernelSize / 2;
    Image copy = image;
    for (int row = 0; row < image.height; row++)
    {
        for (int col = 0; col < image.width; col++)
        {
            int red = 0, green = 0, blue = 0, count = 0;
            for (int dr = -radius; dr <= radius; dr++)
            {
                for (int dc = -radius; dc <= radius; dc++)
                {
                    int nr = row + dr;
                    int nc = col + dc;
                    if (nr >= 0 && nr < image.height && nc >= 0 && nc < image.width)
                    {
                        red += copy(nc, nr, 0);
                        green += copy(nc, nr, 1);
                        blue += copy(nc, nr, 2);
                        count++;
                    }
                }
            }
            image(col, row, 0) = round((float)red / count);
            image(col, row, 1) = round((float)green / count);
            image(col, row, 2) = round((float)blue / count);
        }
    }
}
