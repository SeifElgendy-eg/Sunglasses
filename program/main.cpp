#include <Image_Class.h>
#include <iostream>

using namespace std;
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

    img.saveImage("output.png");

    return 0;
}