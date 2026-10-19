#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "board.h"
#if !USE_LWIP
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
#else
#include "ethernet.h"
#include "tcpip.h"
#include "lwip/apps/lwiperf.h"
#include "dhcp.h"
#include "lwip/sockets.h"
#endif

#include "mmcsd_core.h"
#include "mfcapi.h"
#include "lcd.h"
#include "timer.h"
#if !USE_LWIP
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_DHCP_Server.h"
#endif
#include "carlink_video.h"
#include "carlink_ey_audio.h"
#include "carlink_ey.h"
#include "brtbt_interface.h"
#include "iot_wifi.h"
#include "carlink_utils.h"
#include "carlink_common.h"
#include "time.h"
#include "cJSON.h"
#include "FreeRTOS_Sockets.h"

// Configuration parameters
#define H264DEC_INBUF_SIZE (LCD_WIDTH * LCD_HEIGHT * 2)
#define MAX_PAYLOAD_SIZE 1024 * 384 // Maximum payload length
#define SOCKET_TIMEOUT_MS 100      // Non-blocking timeout in milliseconds
#define TASK_STACK_SIZE 2048       // Task stack size
#define TASK_PRIORITY 5            // Task priority

// -------------------------- Simplified locked logging macros --------------------------
// 1. Log level definitions (only INFO and ERROR retained)
#define LOG_LEVEL_INFO 2    // Information level (critical processes, such as connections, startups)
#define LOG_LEVEL_ERROR 1   // Error level (exception information, such as socket failures, parsing errors)
#define LOG_LEVEL_DISABLE 0 // Disable all logs

// Current log level (switch as needed for debug/release versions; release versions are recommended to be set to LOG_LEVEL_ERROR)
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO

#define __filename__(x)                                                       \
    ({                                                                        \
        const char *slash = strrchr(x, '/');                                  \
        const char *backslash = strrchr(x, '\\');                             \
        const char *last_separator = (slash > backslash) ? slash : backslash; \
        last_separator ? last_separator + 1 : x;                              \
    })

// 2. Log global mutex (ensure multi-threaded printing does not conflict)
static SemaphoreHandle_t g_log_mutex = NULL;

// 3. Log lock initialization (must be called before task creation, placed at the beginning of ey_servers_init)
static void log_mutex_init(void)
{
    // if (g_log_mutex == NULL)
    // {
    //     g_log_mutex = xSemaphoreCreateMutex();
    //     // When lock creation fails, use raw printf for emergency dnotification (only once to avoid repeated printing)
    //     if (g_log_mutex == NULL)
    //     {
    //         printf("[LOG_EMERG] Log mutex creation failed! May cause log confusion!\r\n");
    //     }
    // }
}

// 4. INFO level logs (including filename and line number, used for critical process notifications)

#define LOG_INFO(fmt, ...)                                                                     \
    do                                                                                         \
    {                                                                                          \
        /* Format: [INFO] filename:line number - content */                                    \
        printf("[INFO] %s:%d - " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)

// 5. ERROR level logs (including filename and line number, used for exception error location)
#define LOG_ERROR(fmt, ...)                                                                     \
    do                                                                                          \
    {                                                                                           \
        printf("[ERROR] %s:%d - " fmt "\r\n", __func__, __LINE__, ##__VA_ARGS__); \
    } while (0)
// ------------------------------------------------------------------------

// 2-byte endian conversion for unsigned char
#define sw16(x)                                             \
    ((uint16_t)((((uint16_t)(x) & (uint16_t)0x00ff) << 8) | \
                (((uint16_t)(x) & (uint16_t)0xff00) >> 8)))

// 4-byte endian conversion for int32_t
#define sw32(x)                             \
    ((((uint32_t)(x) & 0xff000000) >> 24) | \
     (((uint32_t)(x) & 0x00ff0000) >> 8) |  \
     (((uint32_t)(x) & 0x0000ff00) << 8) |  \
     (((uint32_t)(x) & 0x000000ff) << 24))

// Port 11111 message header definition (disable alignment)
#pragma pack(push, 1)
typedef struct
{
    uint8_t sync_word[3];  // Sync word (e.g., 0xAA,0xBB,0xCC)
    uint8_t type;          // Message type
    uint32_t dummy;        // Reserved field
    uint32_t payload_size; // Payload length
} msg_header_11111;
#pragma pack(pop)

// Port 11113 message header definition (disable alignment)
#pragma pack(push, 1)
typedef struct
{
    uint8_t magic[2];       // Magic word "EY"
    uint32_t json_length;   // JSON part length
    uint32_t binary_length; // Binary part length
} msg_header_11113;
#pragma pack(pop)

// Client connection status
typedef struct
{
    int socket;              // Client socket
    struct sockaddr_in addr; // Client address
    SemaphoreHandle_t mutex; // Synchronization lock
} ClientInfo;

// Global client status (shared by both ports, ensuring only one client at a time)
static ClientInfo g_client_11111 = {-1, {0}, NULL};
static ClientInfo g_client_11113 = {-1, {0}, NULL};
static SemaphoreHandle_t g_global_mutex; // Global client management lock

static struct timespec timeRecvLast = {0, 0};
static int heartbeat_flag = 0;
static struct timespec timelast = {0, 0};
static int process_flag_11111 = 0;
static int process_flag_11113 = 0;
static uint16_t gH264Index = 0;
static int gH264Frames = 0;
static int gCRC16Log = 0;
static int gNeedJumpFlag = 0;

extern void ey_server_connected(int is_connected);
extern void ey_server_h264_recv(uint8_t *data,size_t data_len);
extern void ey_server_11113_recv(uint8_t *json,size_t json_len, uint8_t *binary,size_t binary_len);
extern void er_server_end();

static unsigned int  read2BytesToInt2( unsigned char *buffer)
{
    return ntohs(*( unsigned short *)(buffer));
}

void writeIntToBuf4Bytes2(unsigned char *buffer,  unsigned int  value)
{
    uint32_t packetlen = htonl((uint32_t)(value));
    memcpy(buffer, &packetlen, sizeof(packetlen));
}
// #define CRC16_ENABLE 1
#ifdef CRC16_ENABLE


// 利用32位累加器和循环展开优化的16位校验和
static uint16_t calculate_checksum16_fast(const unsigned char *data, size_t length) {
    uint32_t sum = 0;
    size_t i = 0;
    
    // 按4字节块处理（精确匹配Java的LITTLE_ENDIAN解析）
    for (; i + 3 < length; i += 4) {
        // 小端序解析：低地址字节对应整数低位
        uint32_t val = 
            ((uint32_t)data[i] & 0xFF) |
            (((uint32_t)data[i+1] & 0xFF) << 8) |
            (((uint32_t)data[i+2] & 0xFF) << 16) |
            (((uint32_t)data[i+3] & 0xFF) << 24);
        sum += val;
    }
    
    // 处理剩余字节（与Java保持一致的累加方式）
    for (; i < length; i++) {
        sum += (uint32_t)(data[i] & 0xFF);
    }
    
    // 折叠为16位（最多两次运算）
    sum = (sum & 0xFFFF) + (sum >> 16);
    if (sum > 0xFFFF) {
        sum -= 0x10000;  // 替代第二次加法，结果相同但更直观
    }
    
    return (uint16_t)sum;
}

#endif

// Utility function: Set socket to non-blocking mode
static int set_nonblocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

// Utility function: Set socket timeout
static int set_socket_timeout(int sockfd, int timeout_ms)
{
    struct timeval timeout = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        return -1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
        return -1;
    return 0;
}

static int g_client_11111_status = 0;
static void tcp_server_11111_connected(int status)
{
    if (g_client_11111_status != status)
    {
        g_client_11111_status = status;
        LOG_INFO("tcp_server_11111_connected [%s]", g_client_11111_status ? "true" : "false");
        if (status == 1)
        {
            heartbeat_flag = 1;
            ey_server_connected(1);
        }
        else
        {
            ey_server_connected(0);
            timeRecvLast.tv_nsec = 0;
            heartbeat_flag = 0;
            gCRC16Log = 0;
            gH264Index = 0;
            gNeedJumpFlag = 0;
        }
    }
}

// Close client connection
static void close_client(ClientInfo *client)
{

    LOG_INFO("close_client socketfd = %d", client->socket);

    if (client->socket != -1)
    {
        xSemaphoreTake(client->mutex, portMAX_DELAY);
        shutdown(client->socket, SHUT_RDWR);
        close(client->socket);
        client->socket = -1;
        memset(&client->addr, 0, sizeof(client->addr));
        xSemaphoreGive(client->mutex);
    }
}

// Kick all clients (close both ports simultaneously)
void kick_all_clients()
{
    LOG_INFO("kick_all_clients");
    xSemaphoreTake(g_global_mutex, portMAX_DELAY);
    close_client(&g_client_11111);
    close_client(&g_client_11113);
    xSemaphoreGive(g_global_mutex);

    tcp_server_11111_connected(0);
}
// Add heartbeat detection task
static void clockTime(struct timespec *time_t)
{
    clock_gettime(CLOCK_MONOTONIC, time_t);
}

static double clockDiff(struct timespec t0, struct timespec t1, bool isAbs)
{
    if ((t0.tv_sec == 0 && t0.tv_nsec == 0) || (t1.tv_sec == 0 && t1.tv_nsec == 0))
    {

        return 0.0;
    }
    double time0, time1;

    time0 = t0.tv_sec + (t0.tv_nsec / 1000000000.0);
    time1 = t1.tv_sec + (t1.tv_nsec / 1000000000.0);

    time1 = time0 - time1;

    if (time1 < 0 && isAbs)
    {
        time1 = -time1;
    }

    return time1;
}

static int tcp_send(int socketFd, unsigned char *data, int len)
{
    int sent_bytes = 0;   // Number of bytes sent
    int bytes_left = len; // Remaining bytes to send
    int n;

    if (socketFd == -1)
    {
        return -1;
    }

    while (bytes_left > 0)
    {
        // Attempt to send remaining data
        n = send(socketFd, data + sent_bytes, bytes_left, 0);

        // Check return value of send function
        if (n == -1)
        { // Send failed
            if (errno == EINTR)
            { // If interrupted by signal, continue sending
                continue;
            }
            else
            { // Other errors, return error code
                LOG_ERROR("send err %d", errno);
                return -1;
            }
        }
        else if (n == 0)
        {
            // Normally send does not return 0 unless the peer closes the connection
            LOG_ERROR("Connection closed by peer.");
            return -1;
        }
        else
        {
            // Send successful, update sent and remaining bytes
            sent_bytes += n;
            bytes_left -= n;
        }
    }
    return 0;
}

// Create TCP server and bind to port
static int create_tcp_server(uint16_t port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        LOG_ERROR("Failed to create socket: %d", errno);
        return -1;
    }

    // Set address reuse (avoid port occupation)
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind port
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY};
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        LOG_ERROR("Failed to bind to port %d: %d", port, errno);
        close(sockfd);
        return -1;
    }

    // Start listening
    if (listen(sockfd, 1) < 0)
    {
        LOG_ERROR("Failed to listen on port %d: %d", port, errno);
        close(sockfd);
        return -1;
    }

    // Set listening socket to non-blocking
    set_nonblocking(sockfd);
    LOG_INFO("TCP server started, port: %d", port);
    return sockfd;
}


// Port 11111: Data processing task (non-blocking + sticky packet handling)
static void tcp_server_11111_task(void *arg)
{
    
#define TEST_CRC16
#ifdef TEST_CRC16
    unsigned char CRC16Test[] = {
        0x00, 0x00, 0x00, 0x01, 0x27, 0x64, 0x00, 0x16, 0xAC, 0x2D, 0x90, 0x0C, 0x83, 0xDB, 0x01, 0x10,
        0x00, 0x00, 0x03, 0x00, 0xA0, 0x00, 0x00, 0x0C, 0x89, 0xDA, 0x1C, 0x2D, 0x92, 0x00, 0x00, 0x01,
        0x28, 0xEE, 0x3C, 0xB0, 0x00, 0x00, 0x01, 0x06, 0x00, 0x01, 0xC0, 0x01, 0x01, 0x04, 0x80, 0x00,
        0x00, 0x01, 0x25, 0x88, 0x80, 0x40, 0x3F, 0xDE, 0x08, 0xE5, 0xF1, 0x00, 0x23, 0x09, 0xFF, 0x05,
        0xE3, 0x1E, 0xD3, 0xED, 0x9D, 0xDA, 0x3B, 0xE2, 0x45, 0xD8, 0x63, 0xB4, 0xBB, 0x19, 0x2F, 0x29,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x02, 0xA4, 0xE1, 0xAB,
        0xC2, 0xF3, 0xC0, 0x00, 0x0F, 0x80, 0x0D, 0xF0, 0x1E, 0x20, 0x47, 0xC0, 0xB4, 0x02, 0x78, 0x0C,
        0x00, 0x2D, 0xC0, 0xF0, 0x04, 0x88, 0x18, 0x80, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x08, 0x08
    };
    uint16_t crc = calculate_checksum16_fast(CRC16Test,sizeof(CRC16Test));
    printf("test crc:0x%x OK:0x7f9d\r\n",crc);

#endif

    int server_sock = create_tcp_server(11111);
    if (server_sock < 0)
    {
        return;
    }

    size_t recv_default_size = H264DEC_INBUF_SIZE + sizeof(msg_header_11111);
    uint8_t *recv_buf = pvPortMalloc(recv_default_size);
    
    size_t buf_len = 0;
    uint8_t *header_ptr;

    while (1)
    {
        process_flag_11111 = 1;
        // Wait for new client connection (non-blocking)
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock >= 0)
        {
            // New client connected, kick old connection
            if(g_client_11111.socket != -1){
                tcp_server_11111_connected(0); // Disconnect old client notify
            }
            close_client(&g_client_11111);
            

            // Configure new client socket
            set_nonblocking(client_sock);
            set_socket_timeout(client_sock, SOCKET_TIMEOUT_MS);

            // Update client information
            xSemaphoreTake(g_global_mutex, portMAX_DELAY);
            g_client_11111.socket = client_sock;
            g_client_11111.addr = client_addr;
            xSemaphoreGive(g_global_mutex);

            LOG_INFO("New connection on port 11111: %s:%d socketfd %d",
                     inet_ntoa(client_addr.sin_addr),
                     ntohs(client_addr.sin_port),client_sock);

           buf_len = 0;
        }

        // Process data from connected client
        xSemaphoreTake(g_client_11111.mutex, portMAX_DELAY);
        int current_sock = g_client_11111.socket;
        xSemaphoreGive(g_client_11111.mutex);

        process_flag_11111 = 2;

        if (current_sock != -1)
        {
            tcp_server_11111_connected(1);

            process_flag_11111 = 3;
            // Non-blocking read data into buffer
            ssize_t recv_len = recv(current_sock, recv_buf + buf_len,
                                    recv_default_size - buf_len, 0);
            process_flag_11111 = 4;                        
            if (recv_len > 0)
            {
                buf_len += recv_len;
            }
            else if (recv_len == 0)
            {
                // Client actively closed
                LOG_ERROR("Client on port 11111 disconnected");
                // tcp_server_11111_connected(0);
                kick_all_clients();
                buf_len = 0;
                continue;
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                // Error occurred
                LOG_ERROR("Receive error on port 11111: %d", errno);
                // tcp_server_11111_connected(0);
                kick_all_clients();
                buf_len = 0;
                continue;
            }

            // Sticky packet handling: parse complete message
            while (buf_len >= sizeof(msg_header_11111) && g_client_11111.socket != -1)
            {
                msg_header_11111 *header = (msg_header_11111 *)recv_buf;

                // Verify sync word (assumed to be 0xAA,0xBB,0xCC)
                if (header->sync_word[0] != 0xAA ||
                    header->sync_word[1] != 0xBB ||
                    header->sync_word[2] != 0xCC)
                {
                    // Sync word error, shift to re-search
                    LOG_ERROR("Sync word error on port 11111");
                    memmove(recv_buf, recv_buf + 1, buf_len - 1);
                    buf_len--;
                    continue;
                }

                // Check payload length validity
                if (header->payload_size > H264DEC_INBUF_SIZE)
                {
                    LOG_ERROR("Payload on port 11111 too large: %u > %u", header->payload_size, H264DEC_INBUF_SIZE);
                    // tcp_server_11111_connected(0);
                    kick_all_clients();
                    buf_len = 0;
                    break;
                }

                // Check if complete message is available
                size_t total_len = sizeof(msg_header_11111) + header->payload_size;
                if (buf_len < total_len)
                {
                    break; // Incomplete data, wait for more
                }
                clockTime(&timeRecvLast);
                // Process complete message (example printing only)
                // LOG_INFO("Message received on port 11111: type=%hhu, payload_size=%u",
                //          header->type, header->payload_size);
                if (0x0 == header->type)
                {
#ifdef CRC16_ENABLE
                    uint16_t index, crc16;
                    header_ptr = recv_buf;

                    index = read2BytesToInt2(header_ptr + 4);
                    crc16 = read2BytesToInt2(header_ptr + 6);
                    index = sw16(index);
                    crc16 = sw16(crc16);

                    //printf(" ---- index:%d crc16:%04x ---- [%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x]\r\n", index, crc16,header_ptr[0], header_ptr[1], header_ptr[2], header_ptr[3], header_ptr[4], header_ptr[5], header_ptr[6], header_ptr[7], header_ptr[8], header_ptr[9], header_ptr[10], header_ptr[11]);

                    if (crc16 > 0)
                    {
                        gCRC16Log = 1;
                    }
                    if (gCRC16Log)
                    {
                        if (index != gH264Index)
                        {
                            LOG_ERROR("index error index:%d - %d crc16:%04x header[%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x]", index, gH264Index, crc16,
                                   header_ptr[0], header_ptr[1], header_ptr[2], header_ptr[3], header_ptr[4], header_ptr[5], header_ptr[6], header_ptr[7], header_ptr[8], header_ptr[9], header_ptr[10], header_ptr[11]);
                            gNeedJumpFlag = 1;
                        }
                        gH264Index = index + 1;
                    }
#endif

#ifdef CRC16_ENABLE
                if (gCRC16Log)
                {
                    uint16_t h264_crc16 = calculate_checksum16_fast(recv_buf + sizeof(msg_header_11111)+4,  header->payload_size-4);
                    // printf("CRC index:%d crc16:%04x - %04x header[%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x] len:%d \r\n", index, crc16,h264_crc16,
                    //         header_ptr[0],header_ptr[1],header_ptr[2],header_ptr[3],header_ptr[4],header_ptr[5],header_ptr[6],header_ptr[7],header_ptr[8],header_ptr[9],header_ptr[10],header_ptr[11],h264SrcSize);

                    if (h264_crc16 != crc16)
                    {
                        LOG_ERROR("CRC16 error index:%d crc16:%04x - %04x header[%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x] len:%d \r\n", index, crc16, h264_crc16,
                               header_ptr[0], header_ptr[1], header_ptr[2], header_ptr[3], header_ptr[4], header_ptr[5], header_ptr[6], header_ptr[7], header_ptr[8], header_ptr[9], header_ptr[10], header_ptr[11], header->payload_size-4);
                        // if (header->payload_size-4 < 33)
                        // {
                        //     printf("h264 data len:%d [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ]\r\n", header->payload_size-4, h264SrcBufPtr[0], h264SrcBufPtr[1], h264SrcBufPtr[2], h264SrcBufPtr[3], h264SrcBufPtr[4], h264SrcBufPtr[5], h264SrcBufPtr[6], h264SrcBufPtr[7], h264SrcBufPtr[8], h264SrcBufPtr[9], h264SrcBufPtr[10], h264SrcBufPtr[11], h264SrcBufPtr[12], h264SrcBufPtr[13], h264SrcBufPtr[14], h264SrcBufPtr[15], h264SrcBufPtr[16], h264SrcBufPtr[17], h264SrcBufPtr[18], h264SrcBufPtr[19], h264SrcBufPtr[20], h264SrcBufPtr[21], h264SrcBufPtr[22], h264SrcBufPtr[23], h264SrcBufPtr[24], h264SrcBufPtr[25], h264SrcBufPtr[26], h264SrcBufPtr[27], h264SrcBufPtr[28], h264SrcBufPtr[29], h264SrcBufPtr[30], h264SrcBufPtr[31]);
                        // }
                        gNeedJumpFlag = 1;

                        // Remove processed data
                        memmove(recv_buf, recv_buf + total_len, buf_len - total_len);
                        buf_len -= total_len;
                        continue;
                    }
                }
#endif
                    gH264Frames++;

                    if(1 == gNeedJumpFlag){
                        static int header_time_len = sizeof(msg_header_11111) + 4;

                        if(recv_buf[header_time_len] == 0x00 && 
                            recv_buf[header_time_len + 1] == 0x00  && 
                            recv_buf[header_time_len + 2] == 0x00  &&
                            recv_buf[header_time_len + 3] == 0x01 &&
                            (recv_buf[header_time_len + 4] == 0x65 || recv_buf[header_time_len + 4] == 0x25))
                            {
                                gNeedJumpFlag = 0;
                                LOG_INFO("Jumped to key frame");
                            }
                            else{
                                LOG_ERROR("Waiting key frame [%02x %02x %02x %02x %02x]",recv_buf[header_time_len],recv_buf[header_time_len + 1],recv_buf[header_time_len + 2],recv_buf[header_time_len + 3],recv_buf[header_time_len + 4]);
                                
                                // Remove processed data
                                memmove(recv_buf, recv_buf + total_len, buf_len - total_len);
                                buf_len -= total_len;
                                continue;
                            }
                    }
                    ey_server_h264_recv(recv_buf + sizeof(msg_header_11111)+4, header->payload_size-4);
                }
                else if (0x4 == header->type)
                {
                    uint8_t reply[13] = {0};

                    reply[0] = 'O';
                    reply[1] = 'K';
                    WRITE_LE16(reply + 2, get_carlink_video_width());
                    WRITE_LE16(reply + 4, get_carlink_video_height());
                    WRITE_LE16(reply + 6, get_carlink_video_width());
                    WRITE_LE16(reply + 8, get_carlink_video_height());
                    reply[10] = get_carlink_video_fps();
                    WRITE_LE16(reply + 11, 2000);

                    tcp_send(current_sock, reply, sizeof(reply));

                    LOG_INFO("Send OK reply width:%d height:%d fps:%d", get_carlink_video_width(), get_carlink_video_height(), get_carlink_video_fps());
                }
                else if (0x6 == header->type)
                {
                    LOG_INFO("Receive heartbeat message");
                }
                // TODO: Actually process payload (recv_buf + sizeof(msg_header_11111))

                // Remove processed data
                memmove(recv_buf, recv_buf + total_len, buf_len - total_len);
                buf_len -= total_len;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Yield CPU
    }

    close(server_sock);  // 关闭监听套接字
    if(recv_buf){
        vPortFree(recv_buf);
    }
    er_server_end();
    vTaskDelete(NULL);
}


// Port 11113: Data processing task (non-blocking + sticky packet handling)
static void tcp_server_11113_task(void *arg)
{
    int server_sock = create_tcp_server(11113);
    if (server_sock < 0)
    {
        return;
    }

    size_t recv_default_size = MAX_PAYLOAD_SIZE + sizeof(msg_header_11113);
    uint8_t *recv_buf = pvPortMalloc(recv_default_size);

    size_t buf_len = 0;

    while (1)
    {
        // Wait for new client connection (non-blocking)
        process_flag_11113 = 1;
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock >= 0)
        {
            // New client connected, kick old connection
            close_client(&g_client_11113);

            // Configure new client socket
            set_nonblocking(client_sock);
            set_socket_timeout(client_sock, SOCKET_TIMEOUT_MS);

            // Update client information
            xSemaphoreTake(g_global_mutex, portMAX_DELAY);
            g_client_11113.socket = client_sock;
            g_client_11113.addr = client_addr;
            xSemaphoreGive(g_global_mutex);

            LOG_INFO("New connection on port 11113: %s:%d socketfd %d",
                     inet_ntoa(client_addr.sin_addr),
                     ntohs(client_addr.sin_port),client_sock);

            buf_len = 0;

            //打开功能开关
            static char *json_str = "{"
                                "\"fun\":\"funSwitch\","
                                "\"data\":[\"tbt\",\"cross\",\"lane\"]"
                                "}";

            static char buffer[256] = {0};
            buffer[0] = 'E';
            buffer[1] = 'Y';
            writeIntToBuf4Bytes2(buffer + 2, strlen(json_str));
            memcpy(buffer + 10, json_str, strlen(json_str));
            tcp_send(client_sock,buffer, 10 + strlen(json_str));
        }

        // Process data from connected client
        xSemaphoreTake(g_client_11113.mutex, portMAX_DELAY);
        int current_sock = g_client_11113.socket;
        xSemaphoreGive(g_client_11113.mutex);
        process_flag_11113 = 2;
        if (current_sock != -1)
        {

            // Non-blocking read data into buffer
            process_flag_11113 = 3;
            // printf("process_flag_11113 = 3 \r\n");
            ssize_t recv_len = recv(current_sock, recv_buf + buf_len,
                                    recv_default_size - buf_len, 0);
            // printf("process_flag_11113 = 4 \r\n");
            process_flag_11113 = 4;
            if (recv_len > 0)
            {
                buf_len += recv_len;
            }
            else if (recv_len == 0)
            {
                // Client actively closed
                LOG_ERROR("Client on port 11113 disconnected");
                kick_all_clients();
                buf_len = 0;
                continue;
            }
            else if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                // Error occurred
                LOG_ERROR("Receive error on port 11113: %d", errno);
                kick_all_clients();
                buf_len = 0;
                continue;
            }

            // Sticky packet handling: parse complete message
            while (buf_len >= sizeof(msg_header_11113) && g_client_11113.socket != -1)
            {
                msg_header_11113 *header = (msg_header_11113 *)recv_buf;

                // LOG_INFO("Message received on port 11113: header[%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]", recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3], recv_buf[4], recv_buf[5], recv_buf[6], recv_buf[7], recv_buf[8], recv_buf[9]);

                // Verify magic word "EY"
                if (header->magic[0] != 'E' || header->magic[1] != 'Y')
                {
                    // Magic word error, shift to re-search
                    LOG_ERROR("Invalid magic word on port 11113");
                    memmove(recv_buf, recv_buf + 1, buf_len - 1);
                    buf_len--;
                    continue;
                }
                uint32_t json_length = ntohl(header->json_length);
                uint32_t binary_length = ntohl(header->binary_length);

                // LOG_INFO("11113 --> header len:%d json_length:%d binary_length:%d",sizeof(header), json_length,binary_length);

                // Calculate total length and check validity
                size_t payload_total = json_length + binary_length;
                if (payload_total > MAX_PAYLOAD_SIZE)
                {
                    LOG_ERROR("Payload on port 11113 too large: %u", (unsigned int)payload_total);
                    kick_all_clients();
                    buf_len = 0;
                    break;
                }

                // Check if complete message is available
                size_t total_len = sizeof(msg_header_11113) + payload_total;
                if (buf_len < total_len)
                {
                    break; // Incomplete data, wait for more
                }
                ey_server_11113_recv(recv_buf + sizeof(msg_header_11113), json_length, recv_buf + sizeof(msg_header_11113) + json_length, binary_length);
                // Process complete message (example printing only)
                // LOG_ERROR("Message received on port 11113: json=%s, binary=%u",
                //           recv_buf + sizeof(msg_header_11113), header->binary_length);
                // TODO: Actually process JSON (recv_buf + sizeof(header)) and binary data

                // Remove processed data
                memmove(recv_buf, recv_buf + total_len, buf_len - total_len);
                buf_len -= total_len;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10)); // Yield CPU
    }

    close(server_sock);  // 关闭监听套接字
    if(recv_buf){
        vPortFree(recv_buf);
    }
    vTaskDelete(NULL);
}

static void heartbeat_task_proc(void *arg)
{
    LOG_INFO("ey_heartbeat_task_proc start");

    struct timespec time;
    struct timespec timeSend;
    clockTime(&timeSend);

    while (1) 
    {
        // printf("ey_hea rtbeat_task_proc run heartbeat_flag:%d tv_nsec:%ld\r\n",heartbeat_flag,timeRecvLast.tv_nsec );
        
        clockTime(&time);

        if (heartbeat_flag)
        {

            if (timeRecvLast.tv_nsec != 0 && clockDiff(time, timeRecvLast, true) > 5.0)
            {
                LOG_ERROR("ey_heartbeat_task_proc timeout!!!! 5s -> %.2f",clockDiff(time, timeRecvLast, true));
                kick_all_clients();

                timeRecvLast.tv_nsec = 0;
            }

            if (clockDiff(time, timeSend, true) > 2.0)
            {
                clockTime(&timeSend);

                static uint8_t heartbeatback[] = {
                    0xAA, 0xBB, 0xCC, 0x0F, 0x00, 0x00, 0x00, 0x00,
                    0x0D, 0x00, 0x00, 0x00, // 13 after sw32 conversion (fixed value)
                    '{', '\"', 'v', 'e', 'r', '\"', ':', '\"', '2', '.', '0', '\"', '}'};

                if (g_client_11111.socket != -1)
                {
                    LOG_INFO("Send heartbeat to 11111");
                    tcp_send(g_client_11111.socket, heartbeatback, sizeof(heartbeatback));
                }
            }
        }

        if (timelast.tv_nsec == 0 || clockDiff(time, timelast, true) > 5.0)
        {
            clockTime(&timelast);
            LOG_INFO("ey ==> process:[%d %d] frame rate:%df/s  CRC Show:%s", process_flag_11111, process_flag_11113, gH264Frames / 5, gCRC16Log ? "true" : "false");
            gH264Frames = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(100));; // 100ms
    }

    LOG_INFO("ey_heartbeat_task_proc end");

    vTaskDelete(NULL);
}

#define EY_VERSION "20260121.01"

// Initialization function: Create two server tasks  非堵塞+粘包处理
void ey_servers_init()
{
    LOG_INFO("ey_servers_init ver:%s", EY_VERSION);
    // Step 1: First initialize the log lock (must be before creating tasks to avoid unready locks)
    log_mutex_init();

    // Initialize mutexes
    g_global_mutex = xSemaphoreCreateMutex();
    g_client_11111.mutex = xSemaphoreCreateMutex();
    g_client_11113.mutex = xSemaphoreCreateMutex();

    xTaskCreate(tcp_server_11111_task, "tcp11111", TASK_STACK_SIZE,
                NULL, configMAX_PRIORITIES - 3, NULL);

    xTaskCreate(tcp_server_11113_task, "tcp11113", TASK_STACK_SIZE,
                NULL, TASK_PRIORITY, NULL);

    xTaskCreate(heartbeat_task_proc, "heartbeat", TASK_STACK_SIZE,
                NULL, TASK_PRIORITY, NULL);
}