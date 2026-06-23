/**
* @file motion_detection.c
* @brief Motion detection utilities for frame-to-frame analysis.
* 
* This module provides lightweight motion detection by comparing consecutive
* RGB frames.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "detection.h"
#include "image/image_encoder.h"

/**
* @brief
*
*
*
*
*
*/
bool detect_motion(struct rgb_frame *curr_frame) 
{
    static struct rgb_frame prev_frame = {0};   // persistent previous frame

    // First frame: initialize previous frame and return "no motion"
    if (!prev_frame.data) {
        copy_rgb_frame(curr_frame, &prev_frame);
        return false;
    }

    bool motion = detect_motion_sad(&prev_frame, curr_frame);

    // Update previous frame for next iteration
    copy_rgb_frame(curr_frame, &prev_frame);

    return motion;
}

/**
* @brief Create a deep copy of an RGB frame.
*
* Allocates a new pixel buffer for the destination frame and copies
* all metadata and pixel data from the source frame.
*
* @param src   Pointer to the source RGB frame to copy from
* @param dst   Pointer to the destination RGB frame to copy to
*
* @return 0 on success, -1 on failure
*/
int copy_rgb_frame(struct rgb_frame *src, struct rgb_frame *dst)
{
    if (!src || !src->data || !dst) {
        return -1;
    }

    // Total size in bytes of the RGB frame
    const size_t size = src->stride * src->height;

    // Free existing destination data if any
    if (dst->data) {
        free(dst->data);
        dst->data = NULL;
    }

    dst->width = src->width;
    dst->height = src->height;
    dst->stride = src->stride;

    // Allocate destination pixel buffer
    dst->data = malloc(size);
    if (!dst->data) {
        return -1;  
    }

    // Copy pixel data from source to destination
    memcpy(dst->data, src->data, size);

    return 0;
}

/**
* @brief Detect motion between two consecutive RGB frames.
*
* This function estimates whether significant motion has occurred between two
* consecutive RGB frames by comparing pixel values. It uses the Sum of Absolute
* Difference (SAD) method on a subsampled grid for efficiency.
*
* Motion detection is performed on a subsampled grid with a factor of 4, meaning
* only 1/16th of the pixels are sampled to reduce computational load.
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
* @param curr_frame   Pointer to the current RGB frame
*
* @return true if motion is detected, false otherwise
*/
bool detect_motion_sad(struct rgb_frame *prev_frame, struct rgb_frame *curr_frame) 
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