
#ifndef MSG_H
#define MSG_H

// server to client
enum svc_ops_e
{
    svc_bad,
    svc_nop,
    svc_gamestate,
    svc_configstring,  // [short] [string] only in gamestate messages
    svc_baseline,      // only in gamestate messages
    svc_serverCommand, // [string] to be executed by client game module
    svc_download,      // [short] size [size bytes]
    svc_snapshot,
    svc_EOF
};

// client to server
enum clc_ops_e
{
    clc_bad,
    clc_nop,
    clc_move,          // [[usercmd_t]
    clc_moveNoDelta,   // [[usercmd_t]
    clc_clientCommand, // [string] message
    clc_EOF
};

int msg_read_byte(msg_t *msg);

char *msg_read_string_line(msg_t *msg);

#endif