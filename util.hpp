#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <stdint.h>
#include <openssl/md5.h>
#include <string.h>
#include <time.h>

typedef struct {
	uint32_t key_len;
	uint32_t body_len;
	char content[0];
} request;

typedef struct {
	uint32_t interval_in_seconds;
	char key[0];
} raw_key;

typedef struct {
	char body[0];
} raw_body;

static uint64_t get_key(uint8_t* key, size_t key_len, uint8_t* section_id) {
	uint8_t buf[MD5_DIGEST_LENGTH];
	uint64_t ret;
	MD5((const unsigned char*)key, key_len, buf);
	memcpy(&ret, buf, sizeof(ret));
	*section_id = buf[0];
	return ret;
}

static uint64_t get_raw_key(char* buf, uint8_t* section_id) {
	request* req = (request*)buf;
	_INFO("key_len=%d,body_len=%d", req->key_len, req->body_len);
	raw_key* rkey = (raw_key*)req->content;
	uint8_t* key_buf = (uint8_t*)malloc(req->key_len);
	memcpy(key_buf, rkey->key, req->key_len - sizeof(uint32_t));
	uint32_t time_slot = 0;
	time_t cur_seconds = time(NULL);
	time_slot = cur_seconds / rkey->interval_in_seconds; 
	memcpy(key_buf, &time_slot, req->key_len - sizeof(uint32_t));
	uint64_t ret = get_key(key_buf, req->key_len, section_id);
	free(key_buf);
	return ret;
}

#endif

