/**
* @file detector.h
* @brief Public interface for the detection module.
*/

#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdint.h>

/** @brief */
#define MAX_DETECTIONS  5

/** @brief Context structure for the object detection module */
struct detector_ctx {
    void *model;                /**< tflite::FlatBufferModel* */
    void *interpreter;          /**< tflite::Interpreter* */
};

/**
 * @struct box
 * @brief Represents a single detected bounding box.
 *
 * Coordinates are normalized values in the range [0.0, 1.0], relative to
 * the frame dimensions.
 */
struct box {
    float xmin;  /**< Left boundary of the bounding box */
    float ymin;  /**< Top boundary of the bounding box */
    float xmax;  /**< Right boundary of the bounding box */
    float ymax;  /**< Bottom boundary of the bounding box */
};

/**
 * @struct detection_result
 * @brief Stores all object detection outputs for a single frame.
 *
 * This struct holds the bounding boxes, class IDs, and confidence scores
 * for all objects detected in a frame.
 */
struct detection_result {
    int num_detections;                     /**< Number of objects detected */
    struct box boxes[MAX_DETECTIONS];       /**< Array of detected bounding boxes */
    int class_ids[MAX_DETECTIONS];          /**< Array of class IDs corresponding to each detection */
    float scores[MAX_DETECTIONS];           /**< Array of confidence scores [0.0, 1.0] for each detection */
};

/** Function Prototypes */
int detector_init(struct detector_ctx *dctx);
void run_object_detection(struct detector_ctx *dctx,
                          struct rgb_frame *rgb,
                          struct detection_result *result);
void draw_detections(struct rgb_frame *rgb,
                     struct detection_result *result);

#endif  // DETECTOR_H