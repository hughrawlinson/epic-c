#define CURL_STATICLIB
#define _GNU_SOURCE
// #define DRAW_GUI
#include "curl/curl.h"
#include "gui.c"
#include <dirent.h>
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

typedef struct image_meta {
  char *url[85];
  char *id[22];
} IMAGE_META;

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

struct response get_epic_image_manifest() {
  struct response chunk;

  CURL *curl;
  CURLcode res;
  curl = curl_easy_init();
  curl_easy_setopt(curl, CURLOPT_URL, EPIC_API_ENDPOINT);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);

  init_response(&chunk);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  if (!curl) {
    fprintf(stderr, "Failed to allocate curl\n");
    exit(1);
  }

  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "Failed to load epic images: %s\n",
            curl_easy_strerror(res));
    exit(1);
  }

  curl_easy_cleanup(curl);
  return chunk;
}

static const char *datafolder = "/home/hugh/epic-images/";

void start_epic_image_download(IMAGE_META *image_metas, size_t count,
                               char ***filepaths) {
  CURL *handles[count];
  CURLM *multi_handle = curl_multi_init();
  CURLMsg *msg;  /* for picking up messages with the transfer status */
  int msgs_left; /* how many messages are left */
  FILE *of;
  int still_running = 1;
  char *output_filepath;

  for (size_t i = 0; i < count; i++) {
    handles[i] = curl_easy_init();
    output_filepath =
        malloc(strlen(datafolder) + strlen(image_metas[i].id) + 5);
    strcpy(output_filepath, datafolder);
    strcat(output_filepath, image_metas[i].id);
    strcat(output_filepath, ".png\0");
    asprintf(&(*filepaths)[i], "%s", output_filepath);
    of = fopen(output_filepath, "wb");
    curl_easy_setopt(handles[i], CURLOPT_URL, image_metas[i].url);
    curl_easy_setopt(handles[i], CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(handles[i], CURLOPT_WRITEDATA, of);
    curl_multi_add_handle(multi_handle, handles[i]);
    printf("%s\n", output_filepath);
    free(output_filepath);
  }

  while (still_running) {
    CURLMcode mc = curl_multi_perform(multi_handle, &still_running);

    if (still_running) {
      printf("Still running: %d\n", still_running);
      mc = curl_multi_poll(multi_handle, NULL, 0, 1000, NULL);
    }

    if (mc) {
      fprintf(stderr, "CURL Multi Poll response: %s", curl_multi_strerror(mc));
      break;
    }
    usleep(100000);
  }

  while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      printf("HTTP transfer completed with status %s\n",
             curl_easy_strerror(msg->data.result));
    }

    for (size_t i = 0; i < count; i++) {
      curl_multi_remove_handle(multi_handle, handles[i]);
      curl_easy_cleanup(handles[i]);
    }
    curl_multi_cleanup(multi_handle);
  }
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

void get_image_metas_from_manifest(IMAGE_META *image_metas, size_t image_count,
                                   struct json_object *body) {
  char year[5];
  char month[3];
  char day[3];

  for (size_t i = 0; i < image_count; i++) {
    json_object *photo = json_object_array_get_idx(body, i);
    json_object *image_name_object = json_object_object_get(photo, "image");
    const char *image_name = json_object_get_string(image_name_object);

    strncpy(year, image_name + 8, 4);
    year[4] = '\0';
    strncpy(month, image_name + 12, 2);
    month[2] = '\0';
    strncpy(day, image_name + 14, 2);
    day[2] = '\0';

    snprintf(image_metas[i].url, 85,
             "https://epic.gsfc.nasa.gov/archive/natural/%s/%s/%s/png/%s.png",
             year, month, day, image_name);
    strcpy(image_metas[i].id, image_name);
  }
}

int file_select(const struct dirent *entry) {
  char test_string[9];
  char test_string_postfix[5];

  strncpy(test_string, entry->d_name, 8);
  test_string[8] = NULL;
  strncpy(test_string_postfix, entry->d_name + 22, 5);

  return !strcmp(test_string, "epic_1b_") &
         !strcmp(test_string_postfix, ".png");
}

size_t get_list_of_downloaded_image_paths(char ***filepaths) {
  struct dirent **namelist;
  size_t length;
  length = scandir(datafolder, &namelist, &file_select, alphasort);
  if (length == -1) {
    perror("scandir");
    exit(EXIT_FAILURE);
  }

  *filepaths = malloc(length * sizeof(char *));

  for (size_t i = 0; i < length; i++) {
    asprintf(&(*filepaths)[i], "%s%s", datafolder, namelist[i]->d_name);
    free(namelist[i]);
  }

  free(namelist);
  return length;
}

size_t download_images(char ***filepaths) {
  struct response r;
  json_object *body;
  size_t image_count;
  IMAGE_META *image_metas;

  r = get_epic_image_manifest();
  body = parse_response_body(&r);
  image_count = json_object_array_length(body);
  image_metas = malloc(sizeof(IMAGE_META) * image_count);
  *filepaths = malloc(image_count * sizeof(char *));
  get_image_metas_from_manifest(image_metas, image_count, body);
  start_epic_image_download(image_metas, image_count, filepaths);

  free(body);
  free(r.ptr);
  free(image_metas);

  return image_count;
}

int main(void) {
  char **filepaths;
  size_t filecount;
  int draw_result;

  curl_global_init(CURL_GLOBAL_ALL);

  if (secure_getenv("SKIP_DOWNLOAD")) {
    filecount = get_list_of_downloaded_image_paths(&filepaths);
  } else {
    filecount = download_images(&filepaths);
  }

  for (size_t i = 0; i < filecount; i++) {
    printf("%s\n", filepaths[i]);
  }

  draw_result = draw();

  curl_global_cleanup();
  return draw_result;
}
