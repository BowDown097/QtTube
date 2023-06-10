// Mongoose YouTube annotations proxy - used at https://sperg.rodeo/qttube_annotations
#include "qttubeannotations.h"
#include <curl/curl.h>
#include <mongoose.h>
#define ANNOTATIONS_ENDPOINT "https://storage.googleapis.com/biggest_bucket/annotations"

size_t curl_writefunc(void* ptr, size_t size, size_t nmemb, struct mg_connection* c)
{
    size_t real_size = size * nmemb;
    mg_http_printf_chunk(c, "%.*s", real_size, ptr);
    return real_size;
}

char* get_video_path(const char* video_id)
{
    char first[4];
    strncpy(first, video_id, 3);
    first[3] = '\0';

    char* path = malloc(256);
    if (video_id[0] != '-')
        snprintf(path, 256, "%s/%c/%s/%s.xml.gz", ANNOTATIONS_ENDPOINT, video_id[0], first, video_id);
    else
        snprintf(path, 256, "%s/-/ar-/%s/%s.xml.gz", ANNOTATIONS_ENDPOINT, first, video_id);

    return path;
}

void qttube_annotations_fn(struct mg_connection* c, struct mg_http_message* hm)
{
    char video_id[16];
    if (mg_http_get_var(&hm->query, "videoId", video_id, sizeof(video_id)) <= 0)
    {
        mg_http_reply(c, 400, NULL, "Failed: Missing videoId or invalid videoId given");
        return;
    }

    CURL* curl = curl_easy_init();
    if (curl == NULL)
    {
        mg_http_reply(c, 500, NULL, "Failed: Couldn't initialize CURL");
        return;
    }

    char* video_path = get_video_path(video_id);

    mg_printf(c, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/xml\r\n\r\n");

    char error_buf[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buf);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, video_path);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, c);

    if (curl_easy_perform(curl) != CURLE_OK)
    {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code == 404)
            mg_http_printf_chunk(c, "<error>No annotation data for this video</error>");
        else
            mg_http_printf_chunk(c, "<error>Failed: Couldn't get annotation data: %s</error>", error_buf);
    }

    mg_http_write_chunk(c, "", 0);
    curl_easy_cleanup(curl);
    free(video_path);
}
