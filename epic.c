#define CURL_STATICLIB
// #define DRAW_GUI
#include "curl/curl.h"
#include "gui.c"
#include <json-c/json_tokener.h>
#include <json-c/json_util.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *EPIC_API_ENDPOINT = "https://epic.gsfc.nasa.gov/api/natural";

const short check_delay = 120;
const short rotate_delay = 20;

struct response {
  char *ptr;
  size_t len;
};

void init_response(struct response *s) {
  s->len = 0;
  s->ptr = malloc(s->len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, struct response *s) {
  size_t new_len = s->len + size * nmemb;
  s->ptr = realloc(s->ptr, new_len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size * nmemb;
}

struct response get_epic_images() {
  CURL *curl;
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, EPIC_API_ENDPOINT);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);

  struct response chunk;
  init_response(&chunk);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  if (!curl) {
    fprintf(stderr, "Failed to allocate curl\n");
    exit(1);
  }

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "Failed to load epic images: %s\n",
            curl_easy_strerror(res));
    exit(1);
  }

  curl_easy_cleanup(curl);
  return chunk;
}

json_object *parse_response_body(struct response *r) {
  json_tokener *tok = json_tokener_new();

  json_object *jobj = NULL;
  enum json_tokener_error jerr;

  do {
    jobj = json_tokener_parse_ex(tok, r->ptr, r->len);
  } while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue);

  if (jerr != json_tokener_success) {
    printf("JSON tokenizing failed: %s", json_tokener_error_desc(jerr));
    exit(EXIT_FAILURE);
  }

  json_tokener_free(tok);

  return jobj;
}

int main(void) {
  curl_global_init(CURL_GLOBAL_ALL);

  struct response r = get_epic_images();
  json_object *body = parse_response_body(&r);

  size_t arr = json_object_array_length(body);
  for (size_t i = 0; i < arr; i++) {
    json_object *photo = json_object_array_get_idx(body, i);
    json_object *image_name_object = json_object_object_get(photo, "image");
    const char *image_name = json_object_get_string(image_name_object);

    char year[5];
    strncpy(year, image_name + 8, 4);
    year[4] = '\0';
    char month[3];
    strncpy(month, image_name + 12, 2);
    month[2] = '\0';
    char day[3];
    strncpy(day, image_name + 14, 2);
    day[2] = '\0';

    printf("https://epic.gsfc.nasa.gov/archive/natural/%s/%s/%s/png/%s.png\n",
           year, month, day, image_name);
  }

  free(body);
  free(r.ptr);

  int draw_result = draw();

  curl_global_cleanup();
  return draw_result;
}
