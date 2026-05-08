/**
* @file  image_processor.c
* @brief Image processing stage of the camera streaming pipeline.
*
* This modules implements the core image processing pipeline used by the procuder thread.
*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

#include "image_encoder.h"
#include "camera/camera.h"
#include "image_processor.h"
#include "http/mjpeg_stream.h"
#include "cb/circular_buffer.h"
#include "detection/detection.h"

/**
* @brief Process a captured camera frame and enqueue it for streaming.
*
* This function implements the main frame processing pipeline for a single 
* camera frame. The stages include:
*
*   1. Convert the raw YUYV frame to RGB format.
*   2. Detect motion by comparing the current RGB frame with the previous one.
*       - Object detection is only performed if motion is detected to save
*         computational resources.
*   3. If motion is detected:
*       -  Run object detection on a copy of the current RGB frame resized to
*          the model's expected input (ex. 300x300).
*       - Store detection results in a `detection_result` struct.
*       - Draw bounding boxes and labels on the original RGB frame.
*           - Done on the original 640x480 frame to preserve quality.
*           - Drawing is unaffected by image size, so the original frame is used.
*   4. Encode the annotated RGB frame into JPEG format.
*   5. Push the JPEG frame into the shared circular buffer for the streaming thread.
*   6. Signal frame availability to the consumer thread via a semaphore.
*
* @note This function represents the producer stage of the producer-consumer streaming pipeline.
*
* @param yuyv   Pointer to the captured YUYV frame from the camera
* @param cctx   Pointer to the camera context structure that holds all session state
* @param sctx   Pointer to the stream structure context that holds stream sessions
* @param pipe   Pointer to the pipeline context containing thread and synchronization primitives
* @param dctx   Pointer to the detector context used for object detection
*
* @return 0 on success, -1 on failure
*/
int image_processor(struct yuyv_frame *yuyv, 
                    struct camera_ctx *cctx, 
                    struct stream_ctx *sctx,
                    struct pipeline_ctx *pipe,
                    struct detector_ctx *dctx)
{
    // Validate input pointers
    if (!yuyv || !cctx || !sctx || !pipe || !dctx) {
        printf("image_processor: Invalid arguments\n");
        return -1;
    }

    struct rgb_frame rgb = {0};                       // Original RGB frame (640x480)
    struct rgb_frame rgb_for_detection = {0};         // Copy of RGB frame for object detection
    struct detection_result result = {0};             // Detection results
                    
    struct jpeg_frame *jpeg = calloc(1, sizeof(*jpeg));    // Heap-allocated JPEG frame
    if (!jpeg) {
        printf("image_processor: Failed to allocate JPEG frame\n");
        return -1;
    }

    // 1. Convert raw YUYV frame to RGB
    if (convert_yuyv_to_rgb(yuyv, &rgb) != 0) {
        printf("image_processor: Error converting YUYV to RGB");
        goto cleanup;
    }

    // Make a copy of the original RGB frame for detection
    if (copy_rgb_frame(&rgb, &rgb_for_detection) != 0) {
        printf("image_processor: Failed to copy RGB frame for detection\n");
        goto cleanup;
    }

    // 2. Detect motion between current and previous frames
    bool motion_detected = detect_motion(&rgb);

    if (motion_detected) 
    {
        printf("image_processor: Motion detected, running object detection\n");
        
        // 3. Run object detection on the copied RGB frame
        int nd = run_object_detection(dctx, &rgb_for_detection, &result);
        if (nd < 0) {
            printf("image_processor: Object detection failed\n");
            // Proceed without annotations
        } else if (nd > 0) {
            // 4. Draw bounding boxes and labels on the original RGB frame
            draw_detection(&rgb, &result);
        }
    }

    // 5. Encode the annotated RGB frame into JPEG
    if (convert_rgb_to_jpeg(&rgb, jpeg) != 0) {
        printf("image_processor: Error converting RGB to JPEG");
        goto cleanup;
    }

    // 6. Push JPEG into circular buffer
    pthread_mutex_lock(pipe->mutex);
    cb_write(pipe->cb, jpeg);                        // overwrite allowed
    pthread_mutex_unlock(pipe->mutex);

    sem_post(pipe->sem);                             // Signal frame availability

    // Cleanup
    free(rgb.data);
    free(rgb_for_detection.data);
    return 0;

cleanup:
    // Cleanup on error
    free(rgb.data);
    free(rgb_for_detection.data);
    free(jpeg);
    return -1;
}