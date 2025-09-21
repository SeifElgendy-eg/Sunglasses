#include "helpers.h"
#include <math.h>
// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int num =
                round((image[i][j].rgbtBlue + image[i][j].rgbtRed + image[i][j].rgbtGreen) / 3.0);

            image[i][j].rgbtRed = image[i][j].rgbtGreen = image[i][j].rgbtBlue = num;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            RGBTRIPLE temp = image[i][j];
            image[i][j] = image[i][width - 1 - j];
            image[i][width - 1 - j] = temp;
        }
    }
    return;
}

// blur images
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            copy[i][j] = image[i][j];
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int red = 0, green = 0, blue = 0;
            int count = 0;

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    // Ensure we stay within image bounds
                    if (ni >= 0 && ni < height && nj >= 0 && nj < width)
                    {
                        red += copy[ni][nj].rgbtRed;
                        green += copy[ni][nj].rgbtGreen;
                        blue += copy[ni][nj].rgbtBlue;
                        count++;
                    }
                }
            }

            image[i][j].rgbtRed = round((float) red / count);
            image[i][j].rgbtGreen = round((float) green / count);
            image[i][j].rgbtBlue = round((float) blue / count);
        }
    }
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    RGBTRIPLE copy[height][width];
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            copy[i][j] = image[i][j];
        }
    }

    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int Ix_red = 0, Iy_red = 0;
            int Ix_green = 0, Iy_green = 0;
            int Ix_blue = 0, Iy_blue = 0;

            for (int di = -1; di <= 1; di++)
            {
                for (int dj = -1; dj <= 1; dj++)
                {
                    int ni = i + di;
                    int nj = j + dj;

                    if (ni >= 0 && ni < height && nj >= 0 && nj < width)
                    {
                        int gx_val = Gx[di + 1][dj + 1];
                        int gy_val = Gy[di + 1][dj + 1];

                        Ix_red += copy[ni][nj].rgbtRed * gx_val;
                        Iy_red += copy[ni][nj].rgbtRed * gy_val;

                        Ix_green += copy[ni][nj].rgbtGreen * gx_val;
                        Iy_green += copy[ni][nj].rgbtGreen * gy_val;

                        Ix_blue += copy[ni][nj].rgbtBlue * gx_val;
                        Iy_blue += copy[ni][nj].rgbtBlue * gy_val;
                    }
                }
            }

            int mag_red = round(sqrt(Ix_red * Ix_red + Iy_red * Iy_red));
            int mag_green = round(sqrt(Ix_green * Ix_green + Iy_green * Iy_green));
            int mag_blue = round(sqrt(Ix_blue * Ix_blue + Iy_blue * Iy_blue));

            image[i][j].rgbtRed = (mag_red > 255) ? 255 : mag_red;
            image[i][j].rgbtGreen = (mag_green > 255) ? 255 : mag_green;
            image[i][j].rgbtBlue = (mag_blue > 255) ? 255 : mag_blue;
        }
    }
    return;
}
