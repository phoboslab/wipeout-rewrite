
#include "network_types.h"

#include <stdbool.h>

unsigned short msg_short_flip_endian(unsigned short value)
{
    return (value >> 8) | (value << 8);
}

unsigned int msg_int_flip_endian(unsigned int value)
{
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

int msg_read_bits(msg_t *msg, int bits)
{
    int value = 0;
    int get = 0;
    bool sgn = false;
    int i;
    int nbits = 0;

    if (bits < 0)
    {
        bits = -bits;
        sgn = true;
    }
    else
    {
        sgn = false;
    }

    if (msg->oob)
    {
        if (bits == 8)
        {
            value = msg->data[msg->readcount];
            msg->readcount += 1;
            msg->bit += 8;
        }
        else if (bits == 16)
        {
            unsigned short *sp = (unsigned short *)&msg->data[msg->readcount];
            value = msg_short_flip_endian(*sp);
            msg->readcount += 2;
            msg->bit += 16;
        }
        else if (bits == 32)
        {
            unsigned int *ip = (unsigned int *)&msg->data[msg->readcount];
            value = msg_int_flip_endian(*ip);
            msg->readcount += 4;
            msg->bit += 32;
        }
        else
        {
            //Com_Error(ERR_DROP, "can't read %d bits\n", bits);
        }
    }
    else
    {
        nbits = 0;
        if (bits & 7)
        {
            nbits = bits & 7;
            for (i = 0; i < nbits; i++)
            {
                //value |= (Huff_getBit(msg->data, &msg->bit) << i);
            }
            bits = bits - nbits;
        }
        if (bits)
        {
            for (i = 0; i < bits; i += 8)
            {
                //Huff_offsetReceive(msgHuff.decompressor.tree, &get, msg->data, &msg->bit);
                value |= (get << (i + nbits));
            }
        }
        msg->readcount = (msg->bit >> 3) + 1;
    }
    if (sgn)
    {
        if (value & (1 << (bits - 1)))
        {
            value |= -1 ^ ((1 << bits) - 1);
        }
    }

    return value;
}


int msg_read_byte(msg_t *msg)
{
    int c = (unsigned char)msg_read_bits(msg, 8);
    if (msg->readcount > msg->cursize)
    {
        c = -1;
    }
    return c;
}

char *msg_read_string_line(msg_t *msg)
{
    static char string[MAX_STRING_CHARS];
    unsigned long l = 0;
    int c = 0;

    do
    {
        c = msg_read_byte(msg); // use ReadByte so -1 is out of bounds
        if (c == -1 || c == 0)
        {
            break;
        }
        // translate all fmt spec to avoid crash bugs
        if (c == '%')
        {
            c = '.';
        }
        // don't allow higher ascii values
        if (c > 127)
        {
            c = '.';
        }

        string[l] = c;
        l++;
    } while (l < sizeof(string) - 1);

    string[l] = 0;

    return string;
}

void msg_bitstream(msg_t* buf) {
    buf->oob = true;
}
