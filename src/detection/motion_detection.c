/**
* @file motion_detection.c
* @brief Motion detection utilities for frame-to-frame analysis.
* 
* This module provides lightweight motion detection by comparing consecutive
* RGB frames.
*/

#include <stdio.h>
#include <stdbool.h>

#include "detection.h"
#include "image/image_encoder.h"

/**
* @brief Detect motion between two consecutive RGB frames.
*
* This function estimates whether significant motion has occurred between two
* consecutive RGB frames by comparing pixel values. It uses the Sum of Absolute
* Difference (SAD) method on a subsampled grid for efficiency.
*
* Motion detection is performed on a subsampled grid with a factor of 4, meaning
* only 1/16th of the pixeks are sampled to reduce computational load.
*
* Two motion detection approaches considered:
*
*   1. Total SAD thresholding (RAW)
*       - Accumulate the total sum of absolute differences (SAD) across sampled pixels.
*       - Motion is detected if the total difference exceeds a fixed threshold.
*       - Simple but dependent on image resolution and subsampling density.
*
*   2. Average SAD thresholding (Normalized) - Selected
*       - Compute the average SAD per sampled pixel.
*       - Motion is detected if the average difference exceeds a threshold.
*       - More robust to changes in resolution, subsampling or small noise.
*
* The typical range of the average SAD per sampled pixel is:
*   - 0     : no change between frames
*   - 5-10  : minor noise
*   - 20-40 : noticeable motion
*   - >50   : significant motion
*
* @param prev_frame   Pointer to the previous RGB frame
* @param curr_frame   Pointer to the current RGB frame
*
* @return true if motion is detected, false otherwise
*/
bool detect_motion(struct rgb_frame *prev_frame, struct rgb_frame *curr_frame) 
{
    if (!prev_frame || !curr_frame) {
        printf("detect_motion: Invalid arguments\n");
        return false;
    }

    // Frames must have the same dimensions for comparison
    if (prev_frame->width != curr_frame->width ||
        prev_frame->height != curr_frame->height ||
        prev_frame->stride != curr_frame->stride) {
        printf("detect_motion: Frame dimension mismatch\n");
        return false;
    }

    long diff_sum = 0;                          // Accumulated SAD
    int sampled_pixels = 0;                     // Number of sampled pixels

    const int width = curr_frame->width;
    const int height = curr_frame->height;
    const int stride = curr_frame->stride;

    // Iterate over current frame to compute difference
    for (int y=0; y < height; y+= SUBSAMPLE_FACTOR) 
    {
        // Pointers to the start of each row
        uint8_t *p_prev = prev_frame->data + y * stride;
        uint8_t *p_curr = curr_frame->data + y * stride;

        for (int x=0; x < width; x += SUBSAMPLE_FACTOR) 
        {
            int idx = x * 3;        // byte_index = pixel_index * 3

            // Compute absolute difference per color channel
            diff_sum += abs(p_curr[idx + 0] - p_prev[idx + 0]); // R
            diff_sum += abs(p_curr[idx + 1] - p_prev[idx + 1]); // G
            diff_sum += abs(p_curr[idx + 2] - p_prev[idx + 2]); // B

            sampled_pixels++;                 // Increment sampled pixel count
        }
    }

    // Average SAD per sampled pixel
    long avg_diff = diff_sum / sampled_pixels;

    return (avg_diff >= MOTION_THRESHOLD);
}