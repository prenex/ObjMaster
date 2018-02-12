#ifndef OBJMASTER_WIN_COMPAT
#define OBJMASTER_WIN_COMPAT

// This file is intended to add various clever or dirty tricks to achieve windows MSVC compatibility...
// Only do these things if the compiler is M$C...
#ifdef _MSC_VER

#include <vector>
#include <string>
#include <stdexcept>
// Silly hax for getting rid of errors arising from missing proper posix method names on windows MSVC...
#define strdup _strdup
// strtop_r is not present in MSVC, but there is this random method that basically do the same...
#define strtok_r strtok_s
// strncpy seems to be unsafe in microsoft opinions so we need to hack around this a bit
#define strncpy windows_hacky_strncpy
static char *windows_hacky_strncpy(char *dest, const char *src, size_t n) {
	// I think this is only "more safe" when chars are not 8 bit represented...
	strncpy_s(dest, n * sizeof(char), src, n);
	return dest;
}

/** Various helper functions for UTF-8 and UTF-16 (mostly only needed on windows) */
class UtfHelper {
public:
	/** Converts an utf8 string to utf16 - one could use codecvt but many platforms deprecate it completely or plain miss implementation for that */
	// Rem.: Please do not use wstring at all when on "proper" operating systems - like *nix, *bsd, android, emscripten/webgl etc...
	static inline std::wstring utf8_to_utf16(const std::string& utf8)
	{
		std::vector<unsigned long> unicode;
		size_t i = 0;
		while (i < utf8.size())
		{
			unsigned long uni;
			size_t todo;
			bool error = false;
			unsigned char ch = utf8[i++];
			if (ch <= 0x7F)
			{
				uni = ch;
				todo = 0;
			}
			else if (ch <= 0xBF)
			{
				throw std::logic_error("not a UTF-8 string");
			}
			else if (ch <= 0xDF)
			{
				uni = ch & 0x1F;
				todo = 1;
			}
			else if (ch <= 0xEF)
			{
				uni = ch & 0x0F;
				todo = 2;
			}
			else if (ch <= 0xF7)
			{
				uni = ch & 0x07;
				todo = 3;
			}
			else
			{
				throw std::logic_error("not a UTF-8 string");
			}
			for (size_t j = 0; j < todo; ++j)
			{
				if (i == utf8.size())
					throw std::logic_error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)
					throw std::logic_error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF) {
				// FIXME: Windows enables path names with these although they are not standard. What to do? I choose not to implement them...
				throw std::logic_error("not a UTF-8 string");
			}
			if (uni > 0x10FFFF)
				throw std::logic_error("not a UTF-8 string");
			unicode.push_back(uni);
		}
		std::wstring utf16;
		for (size_t i = 0; i < unicode.size(); ++i)
		{
			unsigned long uni = unicode[i];
			if (uni <= 0xFFFF)
			{
				utf16 += (wchar_t)uni;
			}
			else
			{
				// Support for the whole code-space is here (bigger than 16 bit values)
				uni -= 0x10000;
				// Unicode is a 20 bits wide code-space
				// When we need more than 16 bits
				// we first take the upper 10 bits and add that to 0xD800 => first char
				utf16 += (wchar_t)((uni >> 10) + 0xD800);
				// Then we take the lower 10 bits by and-ing with 0x3FF and add 0xDC00 => second char
				utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
				// See: https://en.wikipedia.org/wiki/UTF-16#U+D800_to_U+DFFF
			}
		}
		return utf16;
	}
};
#endif

#endif