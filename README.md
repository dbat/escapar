# escapar
Convert string with escaped chars to its interpreted binary string

## prototype

	int escaparse(const char *source, char *buf, int bufsize, int *outpos);

	Returns the length of the interpreted string hold in the buf.

	Upon error not enough buf size, the function will return
	the negative value to indicate error, just flip it back
	or use abs to render the original value (the length of
	the successful interpreted string so far).

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


## escaped chars
Recognized escaped chars and its interpretation:
  
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
	\x[N] (locase-x),  [N] is hex digits, max 2 digits represents byte
	\X[N] (CAPITAL-X), [N] is hex digits, max 8 digits represents dword
	\y[N] (locase-y),  [N] is hex digits, max 4 digits represents word
	\Y[N] (CAPITAL-Y), [N] is hex digits, arbitrary length represents dword 
	\d[N] (locase-d),  [N] is decimal digits, arbitrary length represents byte (max=255)
	\D[N] (CAPITAL-D), [N] is decimal digits, arbitrary length represents dword (max=4294967295)
	(any invalid digits will terminate/conclude the interpretation)

	Control characters:
	\c[C] where [C] is letter @A..Z[\]^_ represents CTRL-CHARS 00..1F
		\c@ = 0 (NULL)
		\cA = 1 [CTRL-A]
		\cC = 3 [CTRL-C]
		\cG = 7 [CTRL-G] (beep)
		\cI = 9 [CTRL-I] (TAB)
		\cZ = 26 [CTRL-Z) (EOF)
		...
		\c^ = 30
		\c_ = 31
	(any invalid/unknown chars will terminate the interpretation)

