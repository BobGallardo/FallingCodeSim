#include "led-matrix-c.h"
#include <cstdlib> // For atoi, atof, and rand
#include <cstring> // For strcmp
#include <unistd.h> // For usleep
#include <vector>
#include <iostream>
#include <time.h>   // For srand(time(NULL))

// Global parameters with default values and their ranges
int matrixWidth = 64;                          // Width of one LED matrix panel
int matrixHeight = 64;                         // Height of one LED matrix panel
int chainedMatrix = 1;                         // Number of chained matrix panels
int maxCharacters = 100;                       // Maximum number of characters to display
float fallSpeed = 3;                           // Base speed for falling characters (min: 1, max: 100)
int fallSpeedVariance = 25;                    // Speed variance (min: 0, max: 100)
int fallSpeedEffect = 100;                     // Percentage of characters affected by speed variance (min: 0, max: 100)
int tailLength = 25;                           // Default length of the trails behind characters (min: 1, max: 50)
int tailLengthVariance = 25;                   // Tail length variance as a percentage (min: 0, max: 100)
int tailLengthEffect = 100;                    // Percentage of characters affected by tail length variance

struct Character {
    int x, y;                                   // Position of the character
    int tailLength;                             // Length of the trailing effect
    float fallSpeed;                            // Speed at which the character falls
};

std::vector<Character> characters;              // Vector to hold all characters

void printHelp() {
    std::cout << "Falling Code Simulation\n"
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

// Function to parse command-line arguments
void parseArguments(int argc, char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            printHelp();
            exit(0);
        } else if (strcmp(argv[i], "--width") == 0 && i + 1 < argc) matrixWidth = atoi(argv[++i]);
        else if (strcmp(argv[i], "--height") == 0 && i + 1 < argc) matrixHeight = atoi(argv[++i]);
        else if (strcmp(argv[i], "--max-chars") == 0 && i + 1 < argc) maxCharacters = atoi(argv[++i]);
        else if (strcmp(argv[i], "--speed") == 0 && i + 1 < argc) fallSpeed = atof(argv[++i]);
        else if (strcmp(argv[i], "--tail-length") == 0 && i + 1 < argc) tailLength = atoi(argv[++i]);
        else if (strcmp(argv[i], "--tail-length-variance") == 0 && i + 1 < argc) tailLengthVariance = atoi(argv[++i]);
        else if (strcmp(argv[i], "--tail-length-effect") == 0 && i + 1 < argc) tailLengthEffect = atoi(argv[++i]);
        else if (strcmp(argv[i], "--speed-variance") == 0 && i + 1 < argc) fallSpeedVariance = atoi(argv[++i]);
        else if (strcmp(argv[i], "--speed-effect") == 0 && i + 1 < argc) fallSpeedEffect = atoi(argv[++i]);
        else if (strcmp(argv[i], "--chained-matrix") == 0 && i + 1 < argc) chainedMatrix = atoi(argv[++i]);
        else std::cerr << "Unknown option or missing value for: " << argv[i] << std::endl;
    }
}

// Initializes the characters with random positions, tail lengths, and fall speeds
void initCharacters() {
    srand(time(NULL)); // Seed for random number generation
    characters.resize(maxCharacters);
    for (auto &character : characters) {
        character.x = rand() % matrixWidth;
        character.y = -(rand() % matrixHeight);
        float tailVariance = 1.0f + (rand() % (2 * tailLengthVariance + 1) - tailLengthVariance) / 100.0f;
        character.tailLength = static_cast<int>(tailLength * tailVariance);

        // Apply speed variance based on fallSpeedEffect
        if (rand() % 100 < fallSpeedEffect) {
            float speedAdjustment = 1.0f + (rand() % (2 * fallSpeedVariance + 1) - fallSpeedVariance) / 100.0f;
            character.fallSpeed = fallSpeed * speedAdjustment;
        } else {
            character.fallSpeed = fallSpeed;
        }
    }
}

// Updates and draws each character on the matrix
void updateAndDrawCharacters(struct LedCanvas *offscreen_canvas) {
    for (auto &character : characters) {
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

// Main function
int main(int argc, char **argv) {
    struct RGBLedMatrixOptions options;
    struct RGBLedMatrix *matrix;
    struct LedCanvas *offscreen_canvas;
    memset(&options, 0, sizeof(options));
    options.rows = matrixHeight;
    options.cols = matrixWidth;
    options.chain_length = chainedMatrix; // Use the chainedMatrix variable for matrix setup

    parseArguments(argc, argv); // Parse command-line arguments

    matrix = led_matrix_create_from_options(&options, &argc, &argv);
    if (matrix == NULL) {
        std::cerr << "Could not initialize matrix. Make sure you're running with appropriate permissions.\n";
        return 1;
    }

    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);
    initCharacters(); // Initialize characters

    while (true) {
        led_canvas_clear(offscreen_canvas);
        updateAndDrawCharacters(offscreen_canvas); // Update and draw characters
        offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas); // Swap the offscreen canvas to display the updated frame
        usleep(50000); // Sleep to control the update rate
    }

    led_matrix_delete(matrix); // Clean up
    return 0;
}
