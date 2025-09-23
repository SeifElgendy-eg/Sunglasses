#include <Image_Class.h>
#include <iostream>
#include <cmath>

using namespace std;
void grayscale(Image &image);

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

    grayscale(img);
    img.saveImage("output.png");

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
