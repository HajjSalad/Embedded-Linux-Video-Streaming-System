#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

/**
* @file  request_manager.h
* @brief 
*/

// Forward declare the context structures
struct camera_ctx;
struct stream_ctx;
struct jpeg_frame;
struct pipeline_ctx;

/** Function prototypes */
int read_http_request(int client_fd, char* request_buffer, size_t buffer_size);
void handle_http_request(struct stream_ctx *sctx, struct pipeline_ctx *pipeline, const char *request_buffer);

#endif  // REQUEST_MANAGER_H