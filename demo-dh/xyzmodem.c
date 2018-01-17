#include <string.h>
#include <timer.h>
#include <xyzModem.h>

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define BSP 0x08
#define NAK 0x15
#define CAN 0x18
#define EOF 0x1A                /* ^Z for DOS officionados */

/* to re-use pdl buffer with the xyz pkt[] */
extern uint8_t pdl_cmd_buf[];

/* Data & state local to the protocol */
static struct
{
	uint8_t *pkt, *bufp;
	uint8_t blk, cblk, crc1, crc2;
	uint8_t next_blk;        /* Expected block */
	int len;
	bool at_eof, tx_ack;
	bool tty_initialized;
	struct tty *tty;
} xyz;

#define POLL_INTERVAL                    8/* @921kbps: chars arrive every 9us */

#define xyzModem_CHAR_TIMEOUT            (2000000/POLL_INTERVAL) /* 2 seconds */
#define xyzModem_EXIT_TIMEOUT            (250000)             /* 0.25 seconds */

#define xyzModem_MAX_RETRIES             200
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3 /* Wait for 3 CAN before quitting */

#define xyzModem_tstc xyz.tty->tstc
#define xyzModem_putc xyz.tty->putc
#define xyzModem_getc xyz.tty->getc

/*
 RDA Bootrom - Adaptation layer
 */
#define xyz_udelay(t) udelay(t)

static int xyz_getc_timeout (char *c)
{
	unsigned counter = 0;
	while (!xyzModem_tstc () && (counter < xyzModem_CHAR_TIMEOUT)) {
		xyz_udelay (POLL_INTERVAL);
		counter++;
	}
	if (xyzModem_tstc ()) {
		*c = xyzModem_getc ();
		return 1;
	}
	return 0;
}

static void xyz_flush(void)
{
	while (xyzModem_tstc()) {
		(void)xyzModem_getc();
	}
}

static void xyz_putc(char c)
{
	xyzModem_putc(c);
}

// The non-lookup-table based CRC16 below uses as few
// instructions as the table based version, but 
// avoid expensive memory reads.
static uint16_t
rda_crc16(uint8_t *buf, int len)
{
    int i;
    uint16_t cksum;
    uint8_t  s, t;
    uint32_t r;

    cksum = 0;
    for (i = 0;  i < len;  i++) {
        s = *buf++ ^ (cksum >> 8);
        t = s ^ (s >> 4);
        r = (cksum << 8) ^
            t       ^
            (t << 5) ^
            (t << 12);
        cksum = r;
    }
    return cksum;
}

/*
 X-MODEM code follows
 */
 
/* Wait for the line to go idle */
static void xyzModem_flush(void)
{
	xyz_flush();
}

static int xyzModem_get_hdr (void)
{
  char c;
  int res;
  bool hdr_found = false;
  int i, can_total, hdr_chars;
  uint16_t cksum;

  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  if (xyz.tx_ack)
    {
      xyz_putc (ACK);
      xyz.tx_ack = false;
    }
  while (!hdr_found)
    {
      res = xyz_getc_timeout (&c);
      if (res)
        {
          hdr_chars++;
          switch (c)
            {
            case SOH:
            case STX:
              hdr_found = true;
              break;
            case CAN:
              if (++can_total == xyzModem_CAN_COUNT)
                {
                  return xyzModem_cancel;
                }
              else
                {
                  /* Wait for multiple CAN to avoid early quits */
                  break;
                }
            case EOT:
              /* EOT only supported if no noise */
              if (hdr_chars == 1)
                {
                  xyz_putc (ACK);
                  return xyzModem_eof;
                }
            default:
              /* Ignore, waiting for start of header */
              ;
            }
        }
      else
        {
          /* Data stream timed out */
          xyzModem_flush ();        /* Toss any current input */
          xyz_udelay (xyzModem_EXIT_TIMEOUT);
          return xyzModem_timeout;
        }
    }

  /* Header found, now read the data */
  res = xyz_getc_timeout ((char *) &xyz.blk);
  if (!res)
    {
      return xyzModem_timeout;
    }
  res = xyz_getc_timeout ((char *) &xyz.cblk);
  if (!res)
    {
      return xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  for (i = 0; i < xyz.len; i++)
    {
      res = xyz_getc_timeout (&c);
      if (res)
        {
          xyz.pkt[i] = c;
        }
      else
        {
          return xyzModem_timeout;
        }
    }
  res = xyz_getc_timeout ((char *) &xyz.crc1);
  if (!res)
    {
      return xyzModem_timeout;
    }
  res = xyz_getc_timeout ((char *) &xyz.crc2);
  if (!res)
  {
    return xyzModem_timeout;
  }

  /* Validate the message */
  if ((xyz.blk ^ xyz.cblk) != (uint8_t) 0xFF)
    {
      xyzModem_flush ();
      return xyzModem_frame;
    }
  /* Verify CRC */
  cksum = rda_crc16 (xyz.pkt, xyz.len);
  if (cksum != ((xyz.crc1 << 8) | xyz.crc2))
    {
      return xyzModem_cksum;
    }

  /* If we get here, the message passes [structural] muster */
  return 0;
}


void xyzModem_init(struct tty *ptty)
{
	xyz.tty = ptty;
	xyz.tty_initialized = 1;
}

int xyzModem_stream_open(int *err)
{
	if (xyz.tty_initialized == 0) {
		*err = xyzModem_access;
		return -1;
	}

	/* re-use pdl buffer as xyz.pkt */
	xyz.pkt = pdl_cmd_buf;
	xyz.len = 0;
	xyz.at_eof = false;
	xyz.tx_ack = false;

	/* Select CRC checksum */
	xyz_putc ('C');
	xyz.next_blk = 1;

	return 0;
}

int xyzModem_stream_read(char *buf, int size, int *err)
{
  int stat, total, len;
  int retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  while (!xyz.at_eof && (size > 0))
    {
      if (xyz.len == 0)
        {
          retries = xyzModem_MAX_RETRIES;
          while (retries-- > 0)
            {
              stat = xyzModem_get_hdr ();
              if (stat == 0)
                {
                  if (xyz.blk == xyz.next_blk)
                    {
                      xyz.tx_ack = true;
                      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

                      /* Data blocks can be padded with ^Z (EOF) characters */
                      /* This code tries to detect and remove them */
                      if ((xyz.bufp[xyz.len - 1] == EOF) &&
                          (xyz.bufp[xyz.len - 2] == EOF) &&
                          (xyz.bufp[xyz.len - 3] == EOF))
                        {
                          while (xyz.len
                                 && (xyz.bufp[xyz.len - 1] == EOF))
                            {
                              xyz.len--;
                            }
                        }
                      break;
                    }
                  else if (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
                    {
                      /* Just re-ACK this so sender will get on with it */
                      xyz_putc (ACK);
                      continue;        /* Need new header */
                    }
                  else
                    {
                      stat = xyzModem_sequence;
                    }
                }
              if (stat == xyzModem_cancel)
                {
                  break;
                }
              if (stat == xyzModem_eof)
                {
                  xyz_putc (ACK);
                  xyz.at_eof = true;
                  break;
                }
              xyz_putc ('C');
            }
          if (stat < 0)
            {
              *err = stat;
              xyz.len = -1;
              return total;
            }
        }
      /* Don't "read" data from the EOF protocol package */
      if (!xyz.at_eof)
        {
          len = xyz.len;
          if (size < len)
            len = size;
          memcpy ((uint8_t *)buf, xyz.bufp, len);
          size -= len;
          buf += len;
          total += len;
          xyz.len -= len;
          xyz.bufp += len;
        }
    }
  return total;
}

void xyzModem_stream_close(int *err)
{
}

void xyzModem_stream_terminate(int abort)
{
  if (abort)
    {
      /* The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal */
      /* number of Backspaces is a friendly way to get the other end to abort. */
      xyz_putc (CAN);
      xyz_putc (CAN);
      xyz_putc (CAN);
      xyz_putc (CAN);
      xyz_putc (BSP);
      xyz_putc (BSP);
      xyz_putc (BSP);
      xyz_putc (BSP);
      /* Now consume the rest of what's waiting on the line. */
      xyzModem_flush ();
      xyz.at_eof = true;
    }
  else
    {
      /* Now consume the rest of what's waiting on the line. */
      xyzModem_flush ();

      /* Make a small delay to give terminal programs like minicom
       * time to get control again after their file transfer program
       * exits.
       */
      xyz_udelay (xyzModem_EXIT_TIMEOUT);
    }
}

char *xyzModem_error(int err)
{
	char *perr;

	switch (err) {
	case xyzModem_access:
		return "Can't access file",
	case xyzModem_noZmodem:
		return "Sorry, zModem not available yet",
	case xyzModem_timeout:
		return "Timed out",
	case xyzModem_eof:
		return "End of file",
	case xyzModem_cancel:
		return "Cancelled",
	case xyzModem_frame:
		return "Invalid framing",
	case xyzModem_cksum:
		return "CRC/checksum error",
	case xyzModem_sequence:
		return "Block sequence error",
	default:
		return "Unknown error",
	}
}
