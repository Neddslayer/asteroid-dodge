from PIL import Image
import numpy as np
import sys

def convert_image(input_path, output_path, replace_color=(249, 240, 246)):
    # Open the image
    img = Image.open(input_path).convert("RGBA")

    write = "const uint8_t image_data[] = {\n"

    w, h = img.size
    for i in range(h):
        for j in range(w):
            pixel = img.getpixel((j, i))
            if (pixel[3] == 0):
                write += f"0x{replace_color[0]:02X}, 0x{replace_color[1]:02X}, 0x{replace_color[2]:02X}, 0xff, "
            else:
                write += f"0x{pixel[0]:02X}, 0x{pixel[1]:02X}, 0x{pixel[2]:02X}, 0x{pixel[3]:02X}, "
        write += "\n"
    write += "}"

    # Write the C array to the output file
    with open(output_path, 'wb') as file:
        file.write(write.encode('utf-8'))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py input_image.png output_array.c")
    else:
        convert_image(sys.argv[1], sys.argv[2])
