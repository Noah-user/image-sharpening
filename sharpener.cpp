#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

// global constants
const char EXT[] = ".ppm";
const char IMG_DIR[] = "imageFiles/";

// global variables
int exitState = EXIT_SUCCESS;

// data type alias
using data_t = unsigned short;

// pixel structure
// pixel_t contains fields for the red, green, and blue color values.
// pixel_t has a static const data_t member called MAX_VAL.
// This member is set to a value of 255, which is the maximum allowed
// value for R, G, or B by definition.
// This member is marked as static, which means that it is shared by all
// instances of the pixel_t struct and can be accessed without creating an
// instance of the struct, which can be accessed using pixel_t::MAX_VAL
struct pixel_t {
    // constant
    static const data_t MAX_VAL = 255;
    // instance variables
    data_t red;
    data_t green;
    data_t blue;
};

// image structure
// image_t contains two fields: width and height, which specify the dimensions
// of the image, which is a 2-D array of pixel_t structures that represents
// the image data.
// The image_t struct also has a static const data_t member called
// MAX_DIMENSION.
// This member is set to a value of 1024, which is the maximum allowed
// dimension of an image (to ensure the image fits into memory).
// This member is marked as static, which means that it is shared by all
// instances of the image_t struct and can be accessed without creating an
// instance of the struct.
struct image_t {
    static const data_t MAX_DIMENSION = 600;
    data_t width = 0;  // initialize to 0
    data_t height = 0;  // initialize to 0
    pixel_t data[MAX_DIMENSION][MAX_DIMENSION];
};

// function prototypes
void populateInputFileNames(string names[], const string name);
void openInputFiles(ifstream fin[], const string names[]);
bool readHeader(ifstream& infile, image_t& image);
bool validateFiles(ifstream fin[], image_t& image);
void readPixel(ifstream& infile, pixel_t& pixel);
void calcPixelAverage(ifstream fin[], pixel_t& pixel);
void processImageFiles(ifstream fin[], image_t& image);
void writeHeader(ofstream& outfile, const image_t& image);
void writePixel(ofstream& outfile, const pixel_t& pixel);
void writePixels(ofstream& outfile, const image_t& image);
void writeImage(const string& fnOut, const image_t& image);
void closeInputFiles(ifstream fin[]);

int main(int argc, const char* argv[]) {
    string name;
    string outputFileName;
    cout << "Processing images ..." << endl;

    if (argc != 2) {
        exitState = EXIT_FAILURE;
    } else {
        name = argv[1];    // get image name
        outputFileName = IMG_DIR + name + EXT;
        string names[10];
        ifstream fin[10];
        image_t image;

        populateInputFileNames(names, name);
        openInputFiles(fin, names);  // good here
        for (int i = 0; i < 10; ++i) {
            if (!fin[i].is_open()) {
                cout << "File " << i << " not open!" << endl;
            }
            readHeader(fin[i], image);
        }
//        cout << "read headers initiated" << endl;
//        for (int i = 0; i < 10; ++i) {
//            readHeader(fin[i], image);
//        }
        if (!validateFiles(fin, image)) {
            cout << "Error validate files returned false." << endl;
            closeInputFiles(fin);
            exitState = EXIT_FAILURE;
        }  // good here
        if (exitState == EXIT_SUCCESS) {
            processImageFiles(fin, image);
            writeImage(outputFileName, image);
            cout << endl;
            cout << "Done processing... " << endl;
            cout << "    Use the Linux display command to view: " <<
            outputFileName << endl;
            cout << endl;
            closeInputFiles(fin);
        }
    }
    return exitState;
}

/// function declarations
/// @brief This function populates the array holding the input filenamess
/// with the correct filenames strings
/// @param [out] names an array containing the names of the files
/// @param name the user supplied name of the image
void populateInputFileNames(string names[], const string name) {
    for (int i = 0; i < 10; ++i) {
        stringstream ss;
        ss << IMG_DIR << name << "/" << name << "_" << setw(3) << setfill('0')
        << to_string(i + 1) << EXT;
        names[i] = ss.str();
        cout << names[i] << endl;  // debug
    }
}

/// @brief This function opens the input files
/// @param [in,out] fin an array of input file handlers
/// @param [in] names an array of input filenames
void openInputFiles(ifstream fin[], const string names[]) {
    for (int i = 0; i < 10; ++i) {
        fin[i].open(names[i]);
        if (!fin[i]) {
            cout << "Failed to open file " << names[i] << endl;
            exitState = EXIT_FAILURE;
        }
    }
}

/// @brief Reads header information from the input file stream of an opened
/// ppm file and verifies the magic number, and color max value.
/// @param [in,out] infile The input file stream handler
/// @param [out] image the data structure for size and pixel data
/// @return true if file passes the magic number and vax val test,
/// otherwise false
bool readHeader(ifstream& infile, image_t& image) {
    bool isValid = true;
    string magic;
    int maxVal;
    infile >> magic >> image.width >> image.height >> maxVal;
    if (magic != "P3" || maxVal != pixel_t::MAX_VAL ||
    image.width > image_t::MAX_DIMENSION ||
    image.height > image_t::MAX_DIMENSION) {
        cout << "Invalid header" << endl;
        isValid = false;  // false for bad max val
    }
    return isValid;
}

/// @brief Validate that all the files have the same properties and set
/// the width and height of the final image
/// @pre files must be open
/// @post file handler is now set to read the first pixal
/// @param [in,out] fin an array of file handlers for the images
/// @param [out] image image structure containing the width and height
/// @return true if all the files pass the test, otherwise false
bool validateFiles(ifstream fin[], image_t& image) {
    bool isValid = true;
    image_t tempImage;
    for (int i = 0; i < 10; ++i) {
        fin[i].clear();
        fin[i].seekg(0, ios::beg);
        if (!fin[i]) {
            cout << "File " << i << " is invalid" << endl;
            isValid = false;
        } else {
            if (!readHeader(fin[i], tempImage)) {
                cout << "Error: invalid header file" << i + 1 << endl;
                isValid = false;
            } else {
                if (i == 0) {
                    image.width = tempImage.width;
                    image.height = tempImage.height;
                } else {
                    if (image.width != tempImage.width || image.height !=
                    tempImage.height) {
                        cout << "Error: dimension mismatch" << endl;
                        isValid = false;
                    }
                }
            }
        }
    }
    return isValid;
}

/// @brief Reads a single pixel from the input file stream of an opened ppm
/// file into a pixel_t struct.
/// @param [in,out] infile The input file stream handler
/// @param [out] pixel The pixel_t struct to hold the read pixel
void readPixel(ifstream& infile, pixel_t& pixel) {
    infile >> pixel.red >> pixel.green >> pixel.blue;
}

/// @brief This function calculates the average color values (RGB) for
/// IMAGE_COUNT files
/// @pre the same pixel in each file is ready to be read
/// @param [in] fin an array of input file handlers for the input image files
/// @param pixel The pixel structure that holds the pixel data
void calcPixelAverage(ifstream fin[], pixel_t& pixel) {
    int totalRed = 0;
    int totalGreen = 0;
    int totalBlue = 0;
    pixel_t temp;
    for (int i = 0; i < 10; ++i) {
        readPixel(fin[i], temp);
        totalRed += temp.red;
        totalGreen += temp.green;
        totalBlue += temp.blue;
    }
    pixel.red = static_cast<data_t>(totalRed / 10);
    pixel.green = static_cast<data_t>(totalGreen / 10);
    pixel.blue = static_cast<data_t>(totalBlue / 10);
}

/// @brief This is the key function that reads the pixel data from the
/// image files and stacks the pixels into a clean image.
/// @param [in] fin an array of file handlers for images that need to be
/// processed for clarity
/// @param [out] image holds the pixels of the final calculated image
void processImageFiles(ifstream fin[], image_t& image) {
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            calcPixelAverage(fin, image.data[y][x]);
        }
    }
}

/// @brief Writes header information to the out file stream of
/// an opened ppm file
/// @param [in,out] outfile The input file stream handler
/// @param [out] image pixel data struct with the width and height
void writeHeader(ofstream& outfile, const image_t& image) {
    outfile << "P3\n" << image.width << " " << image.height << "\n" <<
    pixel_t::MAX_VAL << "\n";
}

/// @brief Writes a single pixel's data to an output file stream.
/// @param [in,out] outfile The output file stream handler
/// @param pixel The rgb pixel to be written
void writePixel(ofstream& outfile, const pixel_t& pixel) {
    outfile << pixel.red << " " << pixel.green << " " << pixel.blue << "\n";
}

/// @brief Writes an image's pixel data to an output file stream.
/// @param [in,out] outfile The output file stream handler
/// @param [in] image The struct containing the image data to be written.
void writePixels(ofstream& outfile, const image_t& image) {
    for (int y =0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            writePixel(outfile, image.data[y][x]);
        }
    }
}

/// @brief This function accepts a filename, opens a file for output and
/// write an image file to the hard drive
/// @param fnOut name of the image file, ie orion.ppm
/// @param image the instantiated structure holding the image
void writeImage(const string& fnOut, const image_t& image) {
    ofstream outfile(fnOut);
    if (!outfile) {
        cout << "error opening output file!" << endl;
        exitState = EXIT_FAILURE;
    } else {
        writeHeader(outfile, image);
        writePixels(outfile, image);
        outfile.close();
    }
}

/// @brief This function closes the input files
/// @param [in,out] fin an array of input image files
void closeInputFiles(ifstream fin[]) {
    for (int i = 0; i < 10; ++i) {
        fin[i].close();
    }
}