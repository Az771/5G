// mib_common.h
#ifndef MIB_COMMON_H
#define MIB_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include "BCCH-BCH-Message.h"

int build_mib(BCCH_BCH_Message_t **out_msg);
int encode_mib_to_buffer(uint8_t *buf, size_t buf_size, size_t *out_len);

#endif
