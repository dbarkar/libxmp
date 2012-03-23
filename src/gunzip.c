#include <stdio.h>
#include "common.h"
#include "inflate.h"

/* See RFC1952 for further information */

/* The flag byte is divided into individual bits as follows:

   bit 0   FTEXT
   bit 1   FHCRC
   bit 2   FEXTRA
   bit 3   FNAME
   bit 4   FCOMMENT
   bit 5   reserved
   bit 6   reserved
   bit 7   reserved
*/

#define FLAG_FTEXT	(1 << 0)
#define FLAG_FHCRC	(1 << 1)
#define FLAG_FEXTRA	(1 << 2)
#define FLAG_FNAME	(1 << 3)
#define FLAG_FCOMMENT	(1 << 4)

struct member {
	uint8 id1;
	uint8 id2;
	uint8 cm;
	uint8 flg;
	uint32 mtime;
	uint8 xfl;
	uint8 os;
};

int decrunch_gzip(FILE *in, FILE *out)
{
	struct member member;
	int val, c;
	uint32 crc;

	val = fread(&member, 1, sizeof (struct member), in);
	if (val != sizeof (struct member)) {
		return -1;
	}

	if (member.cm != 0x08) {
		return -1;
	}

	if (member.flg & FLAG_FEXTRA) {
		int xlen = read16l(in);
		fseek(in, xlen, SEEK_CUR);
	}

	if (member.flg & FLAG_FNAME) {
		do {
			c = read8(in);
		} while (c != 0);
	}

	if (member.flg & FLAG_FCOMMENT) {
		do {
			c = read8(in);
		} while (c != 0);
	}

	if (member.flg & FLAG_FHCRC) {
		read16l(in);
	}
	
	val = inflate(in, out, &crc, 1);
	if (val != 0) {
		return -1;
	}

	/* Check CRC32 */
	val = read32l(in);
	if (val != crc) {
		return -1;
	}

	/* Check file size */
	val = read32l(in);
	if (val != ftell(out)) {
		return -1;
	}

	return 0;
}

