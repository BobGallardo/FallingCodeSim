#include "led-matrix-c.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <ctime>
#include <random>
#include <thread>

using namespace std;

struct Character {
    int x, y;
    int tailLength;
    float fallSpeed;
};

// Global parameters with default values and their ranges
int matrixWidth = 64;
int matrixHeight = 64;
int chainedMatrix = 1;
int maxCharacters = 100;
float fallSpeed = 3;
int fallSpeedVariance = 25;
int fallSpeedEffect = 100;
int tailLength = 25;
int tailLengthVariance = 25;
int tailLengthEffect = 100;

vector<Character> characters;

void printHelp() {
    cout << "Falling Code Simulation\n"
         << "Usage (run with sudo for necessary privileges):\n"
         << "  sudo ./falling-code [options]\n"
         << "Options:\n"
         << "  --width [value]                 Set the width of the LED matrix (default: 64)\n"
         << "  --height [value]                Set the height of the LED matrix (default: 64)\n"
         << "  --chained-matrix [value]        Set the number of chained matrix panels (default: 2)\n"
         << "  --max-chars [value]             Set the maximum number of characters (1-1000, default: 100)\n"
         << "  --speed [value]                 Set the base speed for falling characters (1-100, default: 3)\n"
         << "  --speed-variance [value]        Set the speed variance (0-100, default: 25)\n"
         << "  --speed-effect [value]          Set the percentage of characters affected by speed variance (0-100, default: 100)\n"
         << "  --tail-length [value]           Set the default length of the trails (1-50, default: 25)\n"
         << "  --tail-length-variance [value]  Set the tail length variance as a percentage (0-100, default: 25)\n"
         << "  --tail-length-effect [value]    Set the percentage of characters affected by tail length variance (0-100, default: 100)\n"
         << "  --help                          Display this help and exit\n"
         << "Examples:\n"
         << "  sudo ./falling-code --speed 20 --tail-length 15 --speed-variance 30\n"
         << "  sudo ./falling-code --max-chars 100 --speed-effect 50 --tail-length-effect 75\n"
         << "  sudo ./falling-code --width 128 --height 64 --chained-matrix 1 --max-chars 100 --speed 3 --speed-variance 25 --speed-effect 100 --tail-length 25 --tail-length-variance 25 --tail-length-effect 100\n";
}

void parseArguments(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--help") {
            printHelp();
            exit(0);
        } else if (arg == "--width" && i + 1 < argc) matrixWidth = stoi(argv[++i]);
        else if (arg == "--height" && i + 1 < argc) matrixHeight = stoi(argv[++i]);
        else if (arg == "--max-chars" && i + 1 < argc) maxCharacters = stoi(argv[++i]);
        else if (arg == "--speed" && i + 1 < argc) fallSpeed = stof(argv[++i]);
        else if (arg == "--tail-length" && i + 1 < argc) tailLength = stoi(argv[++i]);
        else if (arg == "--tail-length-variance" && i + 1 < argc) tailLengthVariance = stoi(argv[++i]);
        else if (arg == "--tail-length-effect" && i + 1 < argc) tailLengthEffect = stoi(argv[++i]);
        else if (arg == "--speed-variance" && i + 1 < argc) fallSpeedVariance = stoi(argv[++i]);
        else if (arg == "--speed-effect" && i + 1 < argc) fallSpeedEffect = stoi(argv[++i]);
        else if (arg == "--chained-matrix" && i + 1 < argc) chainedMatrix = stoi(argv[++i]);
        else cerr << "Unknown option or missing value for: " << argv[i] << endl;
    }
}

void initCharacters() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> disWidth(0, matrixWidth - 1);
    uniform_int_distribution<> disHeight(0, matrixHeight - 1);
    uniform_real_distribution<> disSpeedVariance(1.0f - fallSpeedVariance / 100.0f, 1.0f + fallSpeedVariance / 100.0f);
    uniform_real_distribution<> disTailVariance(1.0f - tailLengthVariance / 100.0f, 1.0f + tailLengthVariance / 100.0f);
    uniform_int_distribution<> disEffect(0, 100);

    characters.resize(maxCharacters);
    for (auto& character : characters) {
        character.x = disWidth(gen);
        character.y = -disHeight(gen);
        character.tailLength = static_cast<int>(tailLength * disTailVariance(gen));
        if (disEffect(gen) < fallSpeedEffect) {
            character.fallSpeed = fallSpeed * disSpeedVariance(gen);
        } else {
            character.fallSpeed = fallSpeed;
        }
    }
}

void updateAndDrawCharacters(LedCanvas* offscreen_canvas) {
    for (auto& character : characters) {
        character.y += character.fallSpeed;
        if (character.y - character.tailLength > matrixHeight) {
            character.y = -character.tailLength;
            character.x = rand() % matrixWidth;
        }

        for (int j = 0; j < character.tailLength; ++j) {
            int tailY = character.y - j;
            if (tailY < 0 || tailY >= matrixHeight) continue;
            int intensity = (255 * (character.tailLength - j)) / character.tailLength;
            led_canvas_set_pixel(offscreen_canvas, character.x, tailY, 0, intensity, 0);
        }
    }
}

int main(int argc, char** argv) {
    RGBLedMatrixOptions options;
    RGBLedMatrix* matrix;
    LedCanvas* offscreen_canvas;

    memset(&options, 0, sizeof(options));
    options.rows = matrixHeight;
    options.cols = matrixWidth;
    options.chain_length = chainedMatrix;

    parseArguments(argc, argv);

    matrix = led_matrix_create_from_options(&options, &argc, &argv);
    if (matrix == nullptr) {
        cerr << "Could not initialize matrix. Make sure you're running with appropriate permissions.\n";
        return 1;
    }

    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);
    initCharacters();

    while (true) {
        led_canvas_clear(offscreen_canvas);
        updateAndDrawCharacters(offscreen_canvas);
        offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
        this_thread::sleep_for(chrono::milliseconds(50));
    }

    led_matrix_delete(matrix);
    return 0;
}
