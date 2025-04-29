
#include "client.h"

#include <msg.h>
#include <network.h>
#include <stdio.h>

// concerning syncing of clients to the game state

typedef enum
{
	DISCONNECTED,
	CONNECTED
} connect_state_t;

typedef struct
{
	const char *name;
	connect_state_t state;
} client_t;

void server_connect_client(netadr_t from)
{

	// client_t* client;

	// // see https://github.com/id-Software/Quake-III-Arena/blob/dbe4ddb10315479fc00086f08e25d968b4b43c49/code/server/sv_client.c#L230

	// // (skipped a whole bunch of checks to avoid connecting, like invalid client versions and the nature of the connection)

	// // build a new connection
	// // accept the new client
	// // this is the only place a client_t is ever initialized

	// // send the connect packet to the client
	// network_out_of_band_print(SERVER, from, "connnectRepsonse");

	// printf(" %s has connected...\n", client->name );

	// client->state = CONNECTED;
}

void server_parse_msg(msg_t *msg)
{
	// if (cl_shownet->integer == 1)
	// {
	// 	Com_Printf("%i ", msg->cursize);
	// }
	// else if (cl_shownet->integer >= 2)
	// {
	// 	Com_Printf("------------------\n");
	// }

	msg_bitstream(msg);

	// // get the reliable sequence acknowledge number
	// clc.reliableAcknowledge = MSG_ReadLong(msg);
	// //
	// if (clc.reliableAcknowledge < clc.reliableSequence - MAX_RELIABLE_COMMANDS)
	// {
	// 	clc.reliableAcknowledge = clc.reliableSequence;
	// }

	//
	// parse the message
	//
	while (true)
	{
		if (msg->readcount > msg->cursize)
		{
			printf("reading past the end of message...\n");
			break;
		}

		int cmd = msg_read_byte(msg);

		if (cmd == clc_EOF)
		{
			printf("end of message.\n");
			break;
		}

		// if (cl_shownet->integer >= 2)
		// {
		// 	if (!svc_strings[cmd])
		// 	{
		// 		Com_Printf("%3i:BAD CMD %i\n", msg->readcount - 1, cmd);
		// 	}
		// 	else
		// 	{
		// 		SHOWNET(msg, svc_strings[cmd]);
		// 	}
		// }

		// other commands
		switch (cmd)
		{
		default:
			char* s = msg_read_string_line(msg);
			printf("message command not understood...\n");
			printf("message was: '%'\n", s);
			break;
		case clc_clientCommand:
			//CL_ParseCommandString(msg);
			break;
		}
	}
}
