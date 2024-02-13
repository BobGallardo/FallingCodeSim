# FallingCodeSim

## A simulation of falling code for a led matrix panel

[![View the example here](https://img.youtube.com/vi/EgAk3i24URg/0.jpg)](https://www.youtube.com/watch?v=EgAk3i24URg)

Usage (run with sudo for necessary privileges):

    sudo ./falling-code [options]

Options:

    --width [value]                 Set the width of the LED matrix (default: 64)
    --height [value]                Set the height of the LED matrix (default: 64)
    --chained-matrix [value]        Set the number of chained matrix panels (default: 2)
    --max-chars [value]             Set the maximum number of characters (1-1000, default: 100)
    --speed [value]                 Set the base speed for falling characters (1-100, default: 3)
    --speed-variance [value]        Set the speed variance (0-100, default: 25)
    --speed-effect [value]          Set the percentage of characters affected by speed variance (0-100, default: 100)
    --tail-length [value]           Set the default length of the trails (1-50, default: 25)
    --tail-length-variance [value]  Set the tail length variance as a percentage (0-100, default: 25)
    --tail-length-effect [value]    Set the percentage of characters affected by tail length variance (0-100, default: 100)
    --help                          Display this help and exit

Examples:

    sudo ./falling-code --speed 20 --tail-length 15 --speed-variance 30
    sudo ./falling-code --max-chars 100 --speed-effect 50 --tail-length-effect 75
    sudo ./falling-code --width 128 --height 64 --chained-matrix 1 --max-chars 100 --speed 3 --speed-variance 25 --speed-effect 100 --tail-length 25 --tail-length-variance 25 --tail-length-effect 100

Prerequisites:

* Raspberry Pi
* Adafruit RGB Matrix Bonnet for Raspberry Pi [Adafruit](https://www.adafruit.com/product/3211)
* HUB75 type RGB matrices [Example devices](https://www.aliexpress.com/item/3256805021298783.html)

Includes: [led-matrix-c.h](https://github.com/hzeller/rpi-rgb-led-matrix)
