/*
This program is created to copy program contents to yoob framework http://catseye.tc/ix/yoob
For some reason, Ctrl+V and Shift+Ins don't work in yoob_webstart.jnlp on Windows 7, so I emulate them by generating keystrokes.
How to use: 
- Press Edit button in yoob
- Delete old contents
- Switch to cmd line and run this program with file name as parameter
- Quickly (during 2 seconds) switch back to yoob window

author: stasoid; released to public domain
*/
#include <windows.h>
#include <stdio.h>
char* readfile(const char* fname, int* _size);

int main(int argc, char* argv[])
{
	if(argc < 2) { printf("specify filename"); return 1; }

	int len=0;
	char* str = readfile(argv[1], &len);
	
	if(!str) { printf("cannot read file"); return 1; }
	if(!len) return 0;

	Sleep(2000); // time to switch to another window

/*	//PostMessage((HWND)0x403C6, WM_KEYDOWN, 'A', 0);
	//SetActiveWindow((HWND)0x403C6);
	INPUT inp = {INPUT_KEYBOARD};
	//inp.ki.wVk = VK_RETURN;
	//inp.ki.wVk = VK_TAB;
	inp.ki.wScan = '+';
	inp.ki.dwFlags = KEYEVENTF_UNICODE;
	SendInput(1, &inp, sizeof(inp));*/

	INPUT* keys = calloc(2*len, sizeof(INPUT));
	for(int i=0; i<len; i++)
	{
		char c = str[i];
		INPUT* key = &keys[2*i];
		key->type = INPUT_KEYBOARD;

		if(c == '\n' || c == '\t')
			key->ki.wVk = c == '\n' ? VK_RETURN : VK_TAB;
		else if(c >= ' ' && c <= '~')
		{
			key->ki.wScan = c;
			key->ki.dwFlags = KEYEVENTF_UNICODE;
		}
		else
		{
			key->ki.wScan = '?';
			key->ki.dwFlags = KEYEVENTF_UNICODE;
		}

		key[1] = *key;
		key[1].ki.dwFlags |= KEYEVENTF_KEYUP;
	}

	SendInput(2*len, keys, sizeof(INPUT));
}

/////////////////////////////////////////////////////////////////////////
#include <sys/stat.h>

static int fsize(const char *filename)
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
