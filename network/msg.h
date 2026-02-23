
#pragma once

#include "network_types.h"

int msg_read_byte(msg_t *msg);

char *msg_read_string_line(msg_t *msg);
