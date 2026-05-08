/**
* @file  request_manager.c
* @brief 
*
* 
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "request_manager.h"

/**
 * @brief Read an HTTP request from a client socket.
 *
 * Reads the HTTP request line and headers from the client socket into a buffer.
 *
 * @param client_fd File descriptor of the connected client socket
 * @param request_buffer Buffer to store the HTTP request
 * @param buffer_size Size of the request buffer
 *
 * @return Number of bytes read on success, -1 on error
*/
int read_http_request(int client_fd, char* request_buffer, size_t buffer_size)
{
    memset(request_buffer, 0, buffer_size);  // Clear buffer

    ssize_t bytes_read = read(client_fd, request_buffer, buffer_size - 1);

    // Validate the read bytes size
    if (bytes_read < 0) {
        perror("read_http_request: Failed to read from client socket");
        return -1;
    } else if (bytes_read == 0) {
        printf("read_http_request: Client closed connection before sending request.\n");
        return -1;
    }
    
    // Ensure null-termination
    request_buffer[bytes_read] = '\0';
    
    printf("read_http_request: Received request (%zd bytes):\n%s\n", bytes_read, request_buffer);
    
    return bytes_read;
}

/**
 * @brief Route HTTP request to the appropriate handler.
 *
 * Parses the HTTP request line to determine the requested endpoint
 * and dispatches to the corresponding handler function.
 *
 * Supported endpoints:
 *   GET /start-recording   - Start saving frames to disk
 *   GET /start-live        - Live MJPEG stream
 *   GET /playback          - Stream from saved recording
 *
 * @param sctx Stream context containing server and client file descriptors
 * @param pipeline Pipeline context containing camera, buffer, and detector state
 * @param request_buffer Buffer containing the HTTP request
*/
void handle_http_request(struct stream_ctx *sctx, 
                         struct pipeline_ctx *pipeline,
                         const char *request_buffer)
{
    // Parse request line
    if (strstr(request_buffer, "GET /start-recording")) 
    {
        printf("request_manager: start-recording command received\n");
    }
    else if (strstr(request_buffer, "GET /start-live"))
    {
        printf("request_manager: start-live command received\n");
    }
    else if (strstr(request_buffer, "GET /playback"))
    {
        printf("request_manager: playback command received\n");
    } 
    else if (strstr(request_buffer, "GET /"))
    {
        printf("request_manager: default command received\n");
    } 
    else {
        // Unknown request
        printf("request_manager: Unsupported request\n");
    }
}