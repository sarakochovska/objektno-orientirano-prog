#include <cmath>
#include <iostream>
#include <cstdint>
#include <fstream>
using namespace std;

const int size = 1024;

struct RGB {
    uint8_t r, g, b;
};

struct imageRGB {
    RGB data[size][size];
};

void saveToPPM(const imageRGB* pic) {
    ofstream file("C:/Users/Anton/Desktop/output.ppm");

    if (!file.is_open()) {
        cout << "Failed to open output.ppm for writing!" << endl;
        return;
    }

    file << "P3\n" << size << " " << size << "\n255\n";

    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < size; ++y) {
            file << static_cast<int>(pic->data[x][y].r) << " "
                 << static_cast<int>(pic->data[x][y].g) << " "
                 << static_cast<int>(pic->data[x][y].b) << "\n";
        }
    }

    file.close();
    cout << "output.ppm saved to Desktop!" << endl;
}

int main() {
    //Allocate the image on the heap to avoid stack overflow
    imageRGB* img = new imageRGB;

    int center = size / 2;
    float maxD = sqrt(2 * pow(center, 2));

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            float dx = center - row;
            float dy = center - col;
            float d = sqrt(dx * dx + dy * dy);
            float gradient = pow(d / maxD, 0.38f);

            uint8_t red   = static_cast<uint8_t>((1.0f - gradient) * 120);
            uint8_t green = static_cast<uint8_t>((1.0f - gradient) * 40);
            uint8_t blue  = static_cast<uint8_t>((1.0f - gradient) * 255);

            img->data[row][col] = { red, green, blue };
        }
    }

    saveToPPM(img);


    delete img;

    return 0;
}
