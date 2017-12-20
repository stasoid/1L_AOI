/*
This file implements 1L_AOI language http://esolangs.org/wiki/1L_AOI
Specify -eu to use 1L_AOI_EU dialect. See comment for `if (!eu)` to learn differences between dialects.

This file is based on yoob implementaion of 1L_AOI, https://github.com/catseye/yoob/blob/master/src/lang/OneLAOIState.java
Differences from yoob implementation:
- in yoob tape is infinite in both directions, but here only nonnegative indexes allowed and size is limited to 4096 cells (cell index = 0..4095)

author: stasoid; released to public domain
*/

#include <malloc.h>
#include <memory.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
//#include <conio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <assert.h>
// display "warning C4706: assignment within conditional expression" at warning level 3 in Visual Studio
#pragma warning(3: 4706)

#define byte unsigned char
#define bool char
#define false 0
#define true  1

#define elif else if
//#define assert(e) ((e) ? __noop : __debugbreak())
#define max(a,b) ((a)>(b)?(a):(b))

char* usage = "Usage: 1l_aoi [-eu] filename";
bool eu = false;

char* readfile(const char* fname, int* _size);
void readcode(char* str, int len);
void doIO();
bool step();
void debugprint();

int main(int argc, char* argv[])
{
	//getc(stdin);
	//putchar('A');putchar('B');
	//getch();
	//return 0;

	char* filename = 0;
	if(argc < 2) { puts(usage); return 1; }
	if(argv[1][0] == '-')
	{
		if(!(argv[1][1] == 'e' && argv[1][2] == 'u')) { puts(usage); return 1; }
		if(argc < 3) { puts(usage); return 1; }
		eu = true;
		filename = argv[2];
	}
	else
		filename = argv[1];

	int len=0;
	char* str = readfile(filename, &len);
	
	if(!str) { printf("cannot read file"); return 1; }

	readcode(str, len);

	while(step());

	debugprint();
}

int width, height;
char* code;
// zero-based, x grows right, y grows down
int x=0,y=1;
typedef enum{left,right,up,down} edir;
edir dir = right;
#define tapesize 4096
byte tape[tapesize] = {0, 1};
int cellindex = 2;

void debugprint()
{
	fprintf(stderr, "\ntape:[%d,%d,%d] index:%d", tape[0], tape[1], tape[2], cellindex);
}


#define xyok(x,y) ( (x) >= 0 && (x) < width && (y) >= 0 && (y) < height )

char get_ahead_left()
{
	int _x=x, _y=y;
	switch(dir)
	{
	case left:  _x--, _y++; break;
	case right: _x++, _y--; break;
	case up:    _x--, _y--; break;
	case down:  _x++, _y++; break;
	}

	return xyok(_x,_y) ? code[_x+_y*width] : ' ';
}

char get_ahead_right()
{
	int _x=x, _y=y;
	switch(dir)
	{
	case left:  _x--, _y--; break;
	case right: _x++, _y++; break;
	case up:    _x++, _y--; break;
	case down:  _x--, _y++; break;
	}

	return xyok(_x,_y) ? code[_x+_y*width] : ' ';
}

#define DX(dir) ((dir) == left ? -1 : (dir) == right ? 1 : 0)
#define DY(dir) ((dir) == up   ? -1 : (dir) == down  ? 1 : 0)
#define opposite(dir) ((dir) == left ? right : (dir) == right ? left : (dir) == up ? down : up) // turn 180 degrees

edir rotate(edir _dir, int angle)
{
	assert(angle == 90 || angle == -90 || angle == 180);
	if(angle == 180) return opposite(_dir);
	
	bool clockwise = angle == 90; // turn right

	if  (_dir == right) _dir = clockwise ? down : up;
	elif(_dir == left)  _dir = clockwise ? up : down;
	elif(_dir == down)  _dir = clockwise ? left : right;
	elif(_dir == up)    _dir = clockwise ? right : left;
	else assert(0 && "Invalid direction");
	return _dir;
}

bool step()
{
	char c = code[x+y*width];
	if(c == '+')
	{
		/* If the Command Pointer passes through a + sign then the following is evaluated: */
		if(dir == up)
		{
			// Up -- Increase MP Cell by one
			if(cellindex == 1)
				doIO();
			else
				tape[cellindex]++;
		}
		elif(dir == down)
		{
			// Down -- Move MP Right
			if(cellindex == tapesize-1) { puts("attempt at increasing cellindex past tapesize-1"); exit(1); }
			cellindex++;
		}
		elif(dir == right)
		{
			// Right -- Move MP Left
			if(cellindex == 0) { puts("attempt at decreasing cellindex below 0"); exit(1); }
			cellindex--;
		}
		else
		{
			// Left -- Decrease MP Cell by one
			if(cellindex == 1)
				doIO();
			else
				tape[cellindex]--;
		}
	}

	/*
	* If the Command Pointer passes by a + sign, the effect is determined as follows.
	* Normally, the Command Pointer will turn away from the +.
	* If however, the Command Pointer would have been turned left, and the
	*   the Memory Pointer cell is zero, no turn occurs and the Command Pointer proceeds straight.
	* (The + sign must be diagonally opposite the
	* point at which the CP is required to turn.)
	*/

	// Check the two diagonally-in-front-of squares for +'s

	bool rotateRight = get_ahead_left() == '+';
	bool rotateLeft = get_ahead_right() == '+';
	byte b = tape[cellindex];

	if (!eu)
	{
		/*
		* Here's where 1L_AOI and 1L_AOI_EU differ.  In 1L_AOI, deflection is
		* conditional, full stop.
		*/
		rotateRight = rotateRight && (b!=0);
		rotateLeft = rotateLeft && (b!=0);
		
		if(rotateLeft && rotateRight)
			dir = rotate(dir, 180);
		elif(rotateRight)
			dir = rotate(dir, 90);
		elif(rotateLeft)
			dir = rotate(dir, -90);
	}
	else
	{
		/*
		* In 1L_AOI_EU, deflection to the right is conditional on zero (turns right if current cell is 0),
		* deflection to the left is conditional on non-zero (turns left if current cell is nonzero).
		*/
		rotateRight = rotateRight && (b==0);
		rotateLeft = rotateLeft && (b!=0);
		
		if(rotateLeft && rotateRight)
			dir = rotate(dir, 180);  // never happens because cell cannot be zero and nonzero simultaneously
		elif(rotateRight)
			dir = rotate(dir, 90);
		elif(rotateLeft)
			dir = rotate(dir, -90);
	}

	x += DX(dir);
	y += DY(dir);

	return xyok(x,y);
}


/*
 * I/O is the same as 2L:
 * "The two leftmost tape locations, called TL0 (Tape Location 0) and TL1 (Tape Location 1)
 * respectively, are significant. TL1 doesn't actually hold a value, it merely causes an I/O
 * operation if you attempt to increment or decrement it. If the value at TL0 is 0, and you
 * attempt to change the value of TL1, a character will be read from input into TL0. If TL0
 * is not 0, and you attempt to change the value of TL1, a character will be outputted from
 * the value of TL0."
 */
void doIO()
{
	if(!tape[0]) tape[0] = getchar();
	else putchar(tape[0]);
}


int getlinelen(char* str, int len)
{
	int i;
	for(i=0; i<len && str[i]!='\n'; i++);
	// if we searched entire str and did not find any newlines (i==len) then number of chars in the line is len (entire str)
	// otherwise it is i+1 (\n included)
	return i==len ? len : i+1;
}

int get_newline_size(char* line, int linelen)
{
	if(linelen >= 2 && line[linelen-1] == '\n' && line[linelen-2] == '\r') return 2;
	if(linelen >= 1 && line[linelen-1] == '\n') return 1;
	return 0;
}


// this should work with files containing null bytes
// readfile adds null byte, but readcode does not rely on that
void readcode(char* str, int len)
{
	int totallen = len;
	char* line = str;
	width = height = 0;
	// calc width/height (max line length and number of lines)
	while(len > 0)
	{
		int linelen = getlinelen(line, len);
		int crlf = get_newline_size(line, linelen);
		width = max(width, linelen - crlf);
		height++;
		line += linelen;
		len -= linelen;
	}
	
	len = totallen;
	if(len>0 && str[len-1]=='\n') height++;

	code = calloc(width*height, 1);

	line = str;
	int y=0;
	while(len > 0)
	{
		int linelen = getlinelen(line, len);
		for(int x=0; x < linelen; x++)
			code[x+y*width] = line[x];
		y++;
		line += linelen;
		len -= linelen;
	}

	if(height < 2) { puts("file should have at least 2 lines"); exit(1); }
}


int fsize(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) == 0)
		return st.st_size;

	return -1;
}

// adds null byte, but also optionally returns size
char* readfile(const char* fname, int* _size)
{
	int size;
	char* buf;

	FILE* f = fopen(fname, "rb");
	if (!f) return 0;
	size = fsize(fname);
	buf = (char*)malloc(size + 1);
	fread(buf, 1, size, f);
	buf[size] = 0;
	fclose(f);
	if(_size) *_size = size;
	return buf;
}
