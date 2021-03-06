#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "kr_message.h"
#include "kr_net.h"


int kr_message_parse(T_KRMessage *krmsg)
{
    /* parse message header */
    sscanf(krmsg->msgbuf, KR_MSGHEADER_FMT, \
        &krmsg->msgtype, krmsg->msgid, krmsg->serverid, krmsg->clientid, \
        &krmsg->datasrc, krmsg->objectkey, &krmsg->msglen);

    if (krmsg->msgid[0] == '-') krmsg->msgid[0] = '\0';
    if (krmsg->serverid[0] == '-') krmsg->serverid[0] = '\0';
    if (krmsg->clientid[0] == '-') krmsg->clientid[0] = '\0';
    if (krmsg->objectkey[0] == '-') krmsg->objectkey[0] = '\0';
        
    /* check message len */
    if (krmsg->msglen > KR_MSGBODY_MAXLEN) {
        fprintf(stderr, "message too large[%d]!", krmsg->msglen);
        return -1;
    }

    return 0;
}


int kr_message_dump(T_KRMessage *krmsg, char *buf, size_t len)
{
    if (krmsg->msgid[0] == '\0') krmsg->msgid[0] = '-';
    if (krmsg->serverid[0] == '\0') krmsg->serverid[0] = '-';
    if (krmsg->clientid[0] == '\0') krmsg->clientid[0] = '-';
    if (krmsg->objectkey[0] == '\0') krmsg->objectkey[0] = '-';
    
    krmsg->msglen = KR_MSGHEADER_LEN + len;
        
    /* check message len */
    if (krmsg->msglen > KR_MSGBODY_MAXLEN) {
        fprintf(stderr, "message too large[%d]!", krmsg->msglen);
        return -1;
    }

    /* dump message header */
    sprintf(krmsg->msgbuf, KR_MSGHEADER_FMT, \
        krmsg->msgtype, krmsg->msgid, krmsg->serverid, krmsg->clientid, \
        krmsg->datasrc, krmsg->objectkey, krmsg->msglen);

    memcpy(krmsg->msgbuf+KR_MSGHEADER_LEN, buf, len);

    return 0;
}


static void kr_message_set_error(char *err, const char *fmt, ...)
{
    va_list ap;

    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, KR_NET_ERR_LEN, fmt, ap);
    va_end(ap);
}


int kr_message_read(char *err, int fd, T_KRMessage *krmsg)
{
    int readLen = 0;
    char msghead[KR_MSGHEADER_LEN+1] = {0};

    /* read message header */
    readLen = kr_net_read(fd, msghead, KR_MSGHEADER_LEN);
    if( readLen < 0) {
        kr_message_set_error(err, "read head error[%s]!", strerror(errno));
        return -1;
    } else if( readLen == 0) {
        kr_message_set_error(err, "Connection Closed!");
        return 0;
    }
    
    krmsg->fd = fd;
    sscanf(msghead, KR_MSGHEADER_FMT, \
        &krmsg->msgtype, krmsg->msgid, krmsg->serverid, krmsg->clientid, \
        &krmsg->datasrc, krmsg->objectkey, &krmsg->msglen);

    if (krmsg->msgid[0] == '-') krmsg->msgid[0] = '\0';
    if (krmsg->serverid[0] == '-') krmsg->serverid[0] = '\0';
    if (krmsg->clientid[0] == '-') krmsg->clientid[0] = '\0';
    if (krmsg->objectkey[0] == '-') krmsg->objectkey[0] = '\0';
        
    /* check message len */
    if (krmsg->msglen > KR_MSGBODY_MAXLEN) {
        kr_message_set_error(err, "message too large[%d]!", krmsg->msglen);
        return -1;
    }

    /* read message body */
    if (readLen == KR_MSGHEADER_LEN && krmsg->msglen > 0) {
        readLen = kr_net_read(fd, krmsg->msgbuf, krmsg->msglen);
        if( readLen != krmsg->msglen) {
            kr_message_set_error(err, "read body error[%s]!", strerror(errno));
            return readLen;
        }
    }

    return readLen;
}


int kr_message_write(char *err, int fd, T_KRMessage *krmsg)
{
    int writeLen = 0;
    char msghead[KR_MSGHEADER_LEN+1] = {0};

    if (krmsg->msgid[0] == '\0') krmsg->msgid[0] = '-';
    if (krmsg->serverid[0] == '\0') krmsg->serverid[0] = '-';
    if (krmsg->clientid[0] == '\0') krmsg->clientid[0] = '-';
    if (krmsg->objectkey[0] == '\0') krmsg->objectkey[0] = '-';
    
    snprintf(msghead, sizeof(msghead), KR_MSGHEADER_FMT, \
        krmsg->msgtype, krmsg->msgid, krmsg->serverid, krmsg->clientid, \
        krmsg->datasrc, krmsg->objectkey, krmsg->msglen);
        
    /* check message len */
    if (krmsg->msglen > KR_MSGBODY_MAXLEN) {
        kr_message_set_error(err, "message too large[%d]!", krmsg->msglen);
        return -1;
    }

    /* write message header */
    writeLen = kr_net_write(fd, msghead, KR_MSGHEADER_LEN);
    if( writeLen < 0) {
        kr_message_set_error(err, "write head error[%s]!", strerror(errno));
        return -1;
    } else if( writeLen == 0) {
        kr_message_set_error(err, "Connection Closed!");
        return 0;
    }

    if (writeLen == KR_MSGHEADER_LEN && krmsg->msglen > 0) {
        writeLen = kr_net_write(fd, krmsg->msgbuf, krmsg->msglen);
        if( writeLen != krmsg->msglen) {
            kr_message_set_error(err, "write body error[%s]!", strerror(errno));
            return writeLen;
        }
    }

    return writeLen;
}

