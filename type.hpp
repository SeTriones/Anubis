#ifndef _TYPE_HPP_
#define _TYPE_HPP_

#include <stdint.h>

/* request type:
 * uint16_t header_len;
 * uint16_t body_len;
 * char[header_len] header;
 * char[body_len] body_len;
 *
 * response:
 * uint16_t count;
 */

typedef struct {
	uint64_t key;
	int* ptr;
} KeyUnit; 

typedef struct {
	int last_visit;
	int pos;
} VisitUint;

#endif
