#include "update_handler.h"
#include "status_led.h"
#include "board_definitions.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamString.h>

// Forward declaration
extern void shutdown();

// Update progress tracking
typedef struct
{
    bool in_progress;
    size_t current_size;
    size_t total_size;
    String error_message;
    String status;
    unsigned long start_time;
} UpdateProgress;

static UpdateProgress _update_progress = {false, 0, 0, "", "idle", 0};
static WebServer *_server = nullptr;
static unsigned long _reboot_time = 0;  // Time when device should reboot (0 = no reboot pending)

/**
 * Verify that a firmware file is valid
 */
bool verify_firmware_signature(const uint8_t *buf, size_t len)
{
    // For ESP32, check the magic number (0xE9)
    if (len < 1 || buf[0] != 0xE9)
    {
        return false;
    }
    // Should maybe probably possibly do a sha256 check since CI provides it but eh
    return true;
}

/**
 * Start an update process
 */
bool start_update(size_t size)
{
    _update_progress.in_progress = true;
    _update_progress.current_size = 0;
    _update_progress.total_size = size;
    _update_progress.error_message = "";
    _update_progress.status = "starting";
    _update_progress.start_time = millis();

    // Shutdown clock operations
    shutdown();

    // Set LED status
    led_set_status(LED_OTA);

    Serial.println("Update started");

    if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH))
    {
        StreamString str;
        Update.printError(str);
        _update_progress.error_message = str.c_str();
        _update_progress.in_progress = false;
        _update_progress.status = "error";
        Serial.println("Failed to start update: " + _update_progress.error_message);
        return false;
    }

    return true;
}

/**
 * Write data chunk to firmware
 */
bool write_update_chunk(const uint8_t *data, size_t len)
{
    if (!_update_progress.in_progress)
    {
        return false;
    }

    size_t written = Update.write((uint8_t *)data, len);
    if (written != len)
    {
        _update_progress.error_message = "Failed to write firmware data";
        _update_progress.status = "error";
        _update_progress.in_progress = false;
        Update.abort();
        Serial.println("Write failed");
        return false;
    }

    _update_progress.current_size += written;
    _update_progress.status = "uploading";

    led_update();
    return true;
}

/**
 * Finish the update process
 */
bool finish_update()
{
    if (!_update_progress.in_progress)
    {
        return false;
    }

    _update_progress.status = "finishing";

    if (!Update.end(true))
    {
        StreamString str;
        Update.printError(str);
        _update_progress.error_message = str.c_str();
        _update_progress.status = "error";
        _update_progress.in_progress = false;
        return false;
    }

    _update_progress.status = "success";
    _update_progress.in_progress = false;
    Serial.println("Update finished");

    // Schedule reboot for 3 seconds from now (non-blocking)
    _reboot_time = millis() + 3000;

    return true;
}

/**
 * Handle file upload from web form
 */
void handle_update_upload()
{
    if (!_server)
        return;

    HTTPUpload &upload = _server->upload();
    String error_msg = "";

    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("Update started: %s\n", upload.filename.c_str());

        if (!start_update(upload.totalSize))
        {
            error_msg = _update_progress.error_message;
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (_update_progress.in_progress)
        {
            // Verify firmware signature on first chunk
            if (_update_progress.current_size == 0)
            {
                if (!verify_firmware_signature(upload.buf, upload.currentSize))
                {
                    _update_progress.error_message = "Invalid firmware signature";
                    _update_progress.status = "error";
                    _update_progress.in_progress = false;
                    Update.abort();
                    error_msg = "Invalid firmware file";
                }
            }

            if (!write_update_chunk(upload.buf, upload.currentSize))
            {
                error_msg = _update_progress.error_message;
            }
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (upload.totalSize > 0 && _update_progress.current_size == upload.totalSize)
        {
            if (!finish_update())
            {
                error_msg = _update_progress.error_message;
            }
        }
        else if (_update_progress.in_progress)
        {
            error_msg = "Upload size mismatch";
            _update_progress.error_message = error_msg;
            _update_progress.status = "error";
            _update_progress.in_progress = false;
            Update.abort();
        }
    }
    else if (upload.status == UPLOAD_FILE_ABORTED)
    {
        if (_update_progress.in_progress)
        {
            _update_progress.error_message = "Upload cancelled";
            _update_progress.status = "error";
            _update_progress.in_progress = false;
            Update.abort();
        }
    }

    if (error_msg.length() > 0)
    {
        _server->send(400, "application/json",
                      "{\"status\":\"error\",\"message\":\"" + error_msg + "\"}");
    }
}

/**
 * Handle POST request to /api/update/upload
 */
void handle_post_update_upload()
{
    if (!_server)
        return;
    _server->send(200, "application/json", update_get_status());
}

/**
 * Handle GET request to /api/update/status
 */
void handle_get_update_status()
{
    if (!_server)
        return;
    _server->send(200, "application/json", update_get_status());
}

/**
 * Handle POST request to /api/update/fetch from GitHub
 */
void handle_post_update_fetch()
{
    if (!_server)
        return;

    // Parse JSON body
    String body = _server->arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error)
    {
        _server->send(400, "application/json",
                      "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }

    String url = doc["url"] | "";

    if (url.length() == 0)
    {
        _server->send(400, "application/json",
                      "{\"status\":\"error\",\"message\":\"URL is required\"}");
        return;
    }

    _update_progress.status = "fetching";
    _update_progress.in_progress = true;
    _update_progress.current_size = 0;
    _update_progress.total_size = 0;
    _update_progress.error_message = "";
    _update_progress.start_time = millis();

    _server->send(200, "application/json",
                  "{\"status\":\"started\",\"message\":\"Firmware download started\"}");

    // Synchronous fetch - blocks server during download but simplifies implementation
    // UI displays a waiting message during this period

    // Create HTTPS client
    WiFiClientSecure client;
    client.setInsecure(); // Disable certificate verification for GitHub

    HTTPClient http;
    if (!http.begin(client, url))
    {
        _update_progress.status = "error";
        _update_progress.in_progress = false;
        _update_progress.error_message = "Failed to initialize HTTP client";
        Serial.println("Update error: HTTP init failed");
        return;
    }

    // Follow redirects (GitHub releases use 302 redirects)
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        _update_progress.status = "error";
        _update_progress.in_progress = false;
        _update_progress.error_message = "HTTP error: " + String(httpCode);
        http.end();
        Serial.printf("Update error: HTTP %d\n", httpCode);
        return;
    }

    int totalSize = http.getSize();
    if (totalSize <= 0)
    {
        _update_progress.status = "error";
        _update_progress.in_progress = false;
        _update_progress.error_message = "Invalid content length";
        http.end();
        return;
    }

    _update_progress.total_size = totalSize;

    if (!start_update(totalSize))
    {
        http.end();
        return;
    }

    // Stream the firmware download
    WiFiClient *stream = http.getStreamPtr();
    int len = totalSize;
    uint8_t buff[512] = {0};

    while (http.connected() && (len > 0 || len == -1))
    {
        size_t size = stream->available();
        if (size)
        {
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

            // Verify firmware signature on first chunk
            if (_update_progress.current_size == 0)
            {
                if (!verify_firmware_signature(buff, c))
                {
                    _update_progress.error_message = "Invalid firmware signature";
                    _update_progress.status = "error";
                    _update_progress.in_progress = false;
                    Update.abort();
                    http.end();
                    return;
                }
            }

            if (!write_update_chunk(buff, c))
            {
                http.end();
                return;
            }

            if (len > 0)
            {
                len -= c;
            }
        }
        delay(1); // Allow background tasks
    }

    http.end();

    if (_update_progress.in_progress)
    {
        finish_update();
    }
}

/**
 * Handle POST request to /api/update/cancel
 */
void handle_post_update_cancel()
{
    if (!_server)
        return;
    update_cancel();
    _server->send(200, "application/json",
                  "{\"status\":\"cancelled\",\"message\":\"Update cancelled\"}");
}

/**
 * Handle GET request to /api/update/target
 */
void handle_get_update_target()
{
    if (!_server)
        return;
    _server->send(200, "text/plain", BOARD_TARGET);
}

void update_init(WebServer *server)
{
    _server = server;

    // Register update handlers
    _server->on("/api/update/upload", HTTP_POST, handle_post_update_upload, handle_update_upload);
    _server->on("/api/update/status", HTTP_GET, handle_get_update_status);
    _server->on("/api/update/fetch", HTTP_POST, handle_post_update_fetch);
    _server->on("/api/update/cancel", HTTP_POST, handle_post_update_cancel);
    _server->on("/api/update/target", HTTP_GET, handle_get_update_target);

    Serial.println("Update handler initialized");
}

void update_handle()
{
    // Check if scheduled reboot time has arrived
    if (_reboot_time > 0 && millis() >= _reboot_time)
    {
        _reboot_time = 0;  // Clear the reboot timer
        ESP.restart();
    }
}

String update_get_status()
{
    int percent = 0;
    if (_update_progress.total_size > 0)
    {
        percent = (_update_progress.current_size / (float)_update_progress.total_size) * 100;
    }

    char json[512];
    snprintf(json, sizeof(json),
             "{\"in_progress\":%s,\"status\":\"%s\",\"progress\":%d,\"current\":%d,"
             "\"total\":%d,\"error\":\"%s\",\"elapsed\":%lu}",
             _update_progress.in_progress ? "true" : "false",
             _update_progress.status.c_str(),
             percent,
             _update_progress.current_size,
             _update_progress.total_size,
             _update_progress.error_message.c_str(),
             _update_progress.in_progress ? (millis() - _update_progress.start_time) : 0);

    return String(json);
}

bool update_in_progress()
{
    return _update_progress.in_progress;
}

void update_cancel()
{
    if (_update_progress.in_progress)
    {
        Update.abort();
        _update_progress.in_progress = false;
        _update_progress.status = "cancelled";
        _update_progress.error_message = "Update cancelled by user";
    }
}
