#include "escapars.h"

#define isdgnum(k) ((k >= '0') && (k <= '9'))
#define isnotdgnum(k) ((k < '0') || (k > '9'))
#define isdghex(k) (((k >= '0') && (k <= '9')) || ((k >= 'a') && (k <= 'f')))
#define isnotdghex(k) (((k < '0') || (k > '9')) && ((k < 'a') || (k > 'f')))
#define hextonum(k) (k <= '9' ? k - '0' : k - 'a' + 10)
// char(k) in above definition must have already been lowercased

#define isCCTL(k) ((k >= '@') && (k <= '_'))
#define isnotCCTL(k) ((k < '@') || (k > '_'))

#define TRUE 1
#define FALSE 0
#define WORD unsigned short
#define DWORD unsigned int
#define MINBUFSIZE 4

int putnumber(char *buf, unsigned n) {
	// freestyle little-endian format
	char*bob = buf;
	*buf++ = n & 0xff;
	if (n >= 1<<8 ) *buf++ = (n>>8 ) & 0xff;
	if (n >= 1<<16) *buf++ = (n>>16) & 0xff;
	if (n >= 1<<24) *buf++ = (n>>24) & 0xff;
	return buf-bob;
}

int putnumhex(char *buf, unsigned n, int is32bit) {
	// *((DWORD*)buf) = n; // destructive
	if (is32bit) *((DWORD*)buf) = n;
	else *((WORD*)buf) = n;
	return is32bit? 4 : 2;
}
/*
TEST
	a\ "   -abc\t\x430\D5465433\X066\Y12335
	\c430\D5465433\X066\ //
	\c430\D25631287820\03465433\Y00000000663433\
	a\ \1"\c430\D25631287820\03\r\n46\cG5433\Y00000000663433\\
	a\ \1"\c430\D25631287820\03\r\n46\cG5433\Y00000000663433\\\x23\X45\

*/

int escaparse(const char *source, char *buf, int bufsize, int *outpos) {
	int i, n, x, result;
	char k, cc, c;
	const char *base = source;
	char *first = buf, *last = buf + bufsize - MINBUFSIZE; // dword
	int broken, atEOF, numscan, instate = FALSE;
	if (!source || (bufsize < MINBUFSIZE)) return 0;
	if (outpos) *outpos = 0;
	while ((c = *source++) && (buf <= last)) {
		atEOF = !*source;
		if(c == '\\') {
			if (instate = ~instate) {
				// FIRST dig into state machine
				// should the last \ be thrown away?
				// comment the ifs below to purge the last orphaned "\"
				if (atEOF) *buf++ = c;
				continue;
			}
			//if (!instate) { *buf++ = c; continue; }
		}
		if (!instate) { *buf++ = c; continue; }
		instate = FALSE; broken = FALSE;

		switch(c) {
			// check escapes that needs a valid arg
			// immediate next-char's check only
			case 'c': k = *source; broken = atEOF || isnotCCTL(k); break;
			case 'd': case 'D': k = *source; broken = atEOF || isnotdgnum(k); break;
			case 'x': case 'y': case 'X': case 'Y': k = *source|0x20; broken = atEOF || isnotdghex(k); break;
		}

		if (broken) { *buf++ = c; continue; }

		n = 0; numscan = FALSE;
		switch (c) {
			case '0': *buf++ = '\0'; break;
			case 'a': *buf++ = '\a'; break;
			case 'b': *buf++ = '\b'; break;

			// 'c' has already been checked for validity
			case 'c': *buf++ = *source++ & 0x1f; break; // don't forget to inc pointer
			case 'd': case 'D': x = c=='d' ? 0xff+1 : (unsigned)-1/10;
				while(c = *source++) {
					if isnotdgnum(c) {
						if (numscan) buf += putnumber(buf, n);
						if (c == '\\') source--; // revert back
						else *buf++ = c;
						numscan = FALSE; break;
					} else numscan = TRUE;
					k = c - '0';
					if((unsigned)n < (unsigned)x) n = n * 10 + k;
					else {
						buf += putnumber(buf, n);
						*buf++ = c;
						numscan = FALSE; break;
					}
				}
				// only happened if c == NULL
				if (numscan) buf += putnumber(buf, n);
				if(!c) source--; // revert back
			break;

			case 'f': *buf++ = '\f'; break;
			case 'n': *buf++ = '\n'; break;
			case 'r': *buf++ = '\r'; break;
			case 't': *buf++ = '\t'; break;
			case 'u': *buf++ = '\u'; break;
			case 'v': *buf++ = '\v'; break;

			case 'x': // 2digits hex = byte
				k = *source | 0x20; // actually, already assigned above;
				n = hextonum(k);
				c = *++source;
				k = c | 0x20;
				if isnotdghex(k) *buf++ = n;
				else { *buf++ = n<<4 | hextonum(k); source++; }  // don't forget to inc pointer
			break;

			case 'y': // 4digits hex = WORD
			case 'X': // 8digits hex = DWORD
				cc = c;
				k = i = cc == 'y' ? 4 : 8; k >>= 1;
				while((c = *source++) && (i--)) {
					k = c | 0x20;
					if isnotdghex(k) {
						if (numscan) buf += putnumhex(buf, n, cc == 'X');
						if (c == '\\') source--; // revert back
						else *buf++ = c;
						numscan = FALSE; break;
					} else numscan = TRUE;
					if((unsigned)n < (unsigned)(1<<28))
						n = (n<<4) | hextonum(k);
					else {
						*((DWORD*)buf) = n;
						buf += k;
						*buf++ = c;
						numscan = FALSE; break;
					}
				}
				// only happened if c == NULL, or i == 0
				if (numscan) {
					buf += putnumhex(buf, n, cc = 'X');
					if (c) *buf++ = c;
				}
				if(!c) source--; // revert back
			break;

			case 'Y': // free-style hex number
				while(c = *source++) {
					k = (c | 0x20);
					if (!isdghex(k)) {
						if (numscan) buf += putnumber(buf, n);
						if (c == '\\') source--; // revert back
						else *buf++ = c;
						numscan = FALSE; break;
					} else numscan = TRUE;
					if((unsigned)n < (unsigned)1<<28)
						n = ((unsigned)n<<4) | hextonum(k);
					else {
						buf += putnumber(buf, n);
						*buf++ = c;
						numscan = FALSE; break;
					}
				}
				// only happened if c == NULL
				if (numscan) buf += putnumber(buf, n);
				if(!c) source--; // revert back
			break;
			default: *buf++ = c;
		}
	}
	result = buf - first;
	if (outpos) *outpos = source - base -1;
	if (c) return -result; // not enough buffersize
	return result;
}
