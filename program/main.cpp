#include <Image_Class.h>
#include <iostream>
#include <cmath>

void grayscale(Image &image);
void invert(Image &image);
void reflect(Image &image);

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

    // invert(img);
    // img.saveImage("inverted_output.png");
    reflect(img);
    img.saveImage("reflected_output.png");

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
