#pragma once

/**
 * Read a Porable Grey Map file and store in 2D array of size widthxheigth
 * @return the parsed image, NULL on error
 * */
int** pgm_read(const char* filepath, int* width, int* height);

/**
 * Writes image to the specified file.
 * @return 0 on success, -1 on failure
 * */
int pgm_write(const char* filepath, int** image, int width, int height);
