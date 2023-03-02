#ifndef HAVE_ESCAPARS_H
#define HAVE_ESCAPARS_H

int escaparse(const char *source, char *buf, int bufsize, int *outpos);

#endif /* #ifndef HAVE_ESCAPARS_H */

/*
	Returns the length of the interpreted string hold in the buf.

	Upon error not enough buf size, the function will return
	the negative value to indicate error, just flip it back
	or use abs to render the original value (the successful
	interpreted string so far).

	Arguments:
	- source is the c (NULL-terminated) string to be processed.

	- buf is buffer space to store the interpreted string.

	- bufsize is buffer size, minimum is 4 (size of dword).

		bufsize must have enough space for interpreted string,
		could be max. +1/3rd extra from the original string size
		eg. \X0\X0\X0\X0 will need 16 bytes.
		On the other side, \Y000000000000000 only need 1 byte

		For each iteration, it needs at least 4 bytes to allocate
		int32 result, therefore bufsize must be at least 4 bytes;

	- outpos is out parameter to store source pos to decode.

		If it's not given NULL, the outpos out parameter
		will have the position of source that has not yet
		been processed.

		This make it easy to continue the process should
		the bufsize is too small; you can continue the
		process directly from source at outpos positon;

		It can be ignored by giving it NULL value.
*/

/*
Parse escaped chars in c string, returns binary string:

	Equivalent with printf:
	\0	null byte
	\a	alert
	\b	backspace
	\f	formfeed
	\n	newline/linefeed (0A)
	\r	carriage-return (0D)
	\t	TAB (horizontal)
	\v	vertical TAB

	Extension:
	\xNN locase-x, NN is hex digits, max 2 digits represent byte
	\XNN UPCASE-X, NN is hex digits, max 8 digits represent dword
	\yNN locase-y, NN is hex digits, max 4 digits represent word
	\YNN UPCASE-Y, NN is hex digits, arbitrary digits represent dword 
	\dNN locase-d, NN is decimal digits, arbitrary digits represent byte (max=255)
	\DNN UPCASE-D, NN is decimal digits, arbitrary digits represent dword (max=4294967295)
	(any invalid digits will terminate/conclude the interpretation)

	Control characters:
	\cC C is letter @A..Z[\]^_ represents CTRL-CHARS 00..1F
		\c@ = 0 (NULL)
		\cA = 1 [CTRL-A]
		\cC = 3 [CTRL-C]
		\cG = 7 [CTRL-G] (beep)
		\cI = 9 [CTRL-I] (TAB)
		\cZ = 26 [CTRL-Z) (EOF)
		...
		\c_ = 31
	(any invalid chars will terminate the interpretation)
*/
