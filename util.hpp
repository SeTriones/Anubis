#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include <stdint.h>
#include <openssl/md5.h>
#include <string.h>

uint64_t get_key(uint8_t* key, size_t key_len, uint8_t* section_id) {
	uint8_t buf[MD5_DIGEST_LENGTH];
	uint64_t ret;
	MD5((const unsigned char*)key, key_len, buf);
	memcpy(&ret, buf, sizeof(ret));
	*section_id = buf[0];
	return ret;
}

#endif
