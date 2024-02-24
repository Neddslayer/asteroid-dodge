# Asteroid Dodge
An example game made using [my VEX TinyGL fork](https://github.com/Neddslayer/TinyGL).
Demonstrates basic 2D sprite rendering and management.

## Textures
In order to create textures on the VEX brain without using an SD card, you can use the `convert.py` script.
Usage is `python convert.py "path/to/image/file" "path/to/output/c/file"`
It's similar to the LVGL color converter, however it outputs in the proper format and converts transparent pixels into the "chroma key" color.
Since TinyGL doesn't natively support transparency, I added a chroma key color to it, that being hex `f9f0f6`. Any pixel that matches that color will not be rendered. This isn't a perfect solution, but it works fine for 2D rendering.