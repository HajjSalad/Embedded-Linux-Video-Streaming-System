#ifndef IMAGE_ENCODER_H
#define IMAGE_ENCODER_H

/**
* @file image_encoder.h
* @brief Public API for image format conversion and JPEG compression
*/

#include <stddef.h>

/**
* @brief Container for a raw YUYV422 camera frame
*
* Represents a single frame captured from the camera in YUYV422 pixel format.
*/
struct yuyv_frame {
    unsigned char *data;    /**< Pointer to raw YUYV422 frame data */
    unsigned int width;     /**< Frame width in pixels */
    unsigned int height;    /**< Frame height in pixels */
    unsigned long size;     /**< Size in bytes (width * height * 2) */
};

/**
* @brief Container for an RGB24 image frame
*
* Holds an RGB image converted from a YUYV422 source frame
*
* width:  Number of pixels per row
* stride: Number of bytes between the start of row n and row n+1
* therefore, for RGB24:
*   logical row size = width * 3 bytes
*   stride           = width * 3 bytes + padding (if any)
*
* Padding can exist because of:
*   - DMA alignment (very common)
*   - SIMD alignment (16 / 32 / 64 bytes)
*   - Hardware camera requirements
*   - GPU-friendly row alignment
*
* size = height * stride : Calculated at the point of use because stride may vary.
*/
struct rgb_frame {
    unsigned char *data;    /**< Pointer to raw RGB24 frame data */
    unsigned int width;     /**< Frame width in pixels */
    unsigned int height;    /**< Frame height in pixels */
    unsigned int stride;    /**< Number of bytes per row */
};

/**
* @brief Container for a JPEG-compressed image frame
*/
struct jpeg_frame {
    unsigned char* data;    /**< Pointer to JPEG-compressed image data */
    unsigned long size;     /**< Size of the JPEG data in bytes */
};

/** Function Prototypes */
int convert_yuyv_to_rgb(const struct yuyv_frame *in,
                         struct rgb_frame *out);
int convert_rgb_to_jpeg(const struct rgb_frame *in,
                         struct jpeg_frame *out);
int convert_yuyv_to_jpeg(unsigned char *yuyv_data,
                         int width,
                         int height,
                         struct jpeg_frame *frame);

#endif  /* JPEG_ENCODER_H */