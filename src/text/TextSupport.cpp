// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "TextSupport.h"
#include <cassert>

namespace Text {

	// returns num bytes consumed, or 0 for end/bogus
	int utf8_decode_char(Uint32 *chr, const char *src)
	{
		unsigned int c = *(reinterpret_cast<const unsigned char *>(src));
		if (!c) {
			*chr = c;
			return 0;
		}
		if (!(c & 0x80)) {
			*chr = c;
			return 1;
		} else if (c >= 0xf0) {
			if (!src[1] || !src[2] || !src[3]) return 0;
			c = (c & 0x7) << 18;
			c |= (src[1] & 0x3f) << 12;
			c |= (src[2] & 0x3f) << 6;
			c |= src[3] & 0x3f;
			*chr = c;
			return 4;
		} else if (c >= 0xe0) {
			if (!src[1] || !src[2]) return 0;
			c = (c & 0xf) << 12;
			c |= (src[1] & 0x3f) << 6;
			c |= src[2] & 0x3f;
			*chr = c;
			return 3;
		} else {
			if (!src[1]) return 0;
			c = (c & 0x1f) << 6;
			c |= src[1] & 0x3f;
			*chr = c;
			return 2;
		}
	}

	// encode one Unicode code-point as UTF-8
	//  chr: the Unicode code-point
	//  buf: a character buffer, which must have space for at least 4 bytes
	//       (i.e., assigning to buf[3] must be a valid operation)
	//  returns: number of bytes in the encoded character
	int utf8_encode_char(Uint32 chr, char buf[4])
	{
		unsigned char *ubuf = reinterpret_cast<unsigned char *>(buf);
		if (chr <= 0x7f) {
			ubuf[0] = chr;
			return 1;
		} else if (chr <= 0x7ff) {
			ubuf[0] = 0xc0 | (chr >> 6);
			ubuf[1] = 0x80 | (chr & 0x3f);
			return 2;
		} else if (chr <= 0xffff) {
			ubuf[0] = 0xe0 | (chr >> 12);
			ubuf[1] = 0x80 | ((chr >> 6) & 0x3f);
			ubuf[2] = 0x80 | (chr & 0x3f);
			return 3;
		} else if (chr <= 0x10fff) {
			ubuf[0] = 0xf0 | (chr >> 18);
			ubuf[1] = 0x80 | ((chr >> 12) & 0x3f);
			ubuf[2] = 0x80 | ((chr >> 6) & 0x3f);
			ubuf[3] = 0x80 | (chr & 0x3f);
			return 4;
		} else {
			assert(0 && "Invalid Unicode code-point.");
			return 0;
		}
	}

	Uint32 unicode_to_upper(Uint32 c)
	{
		// Basic Latin and Latin-1
		if (c < 0x80) return (c >= 'a' && c <= 'z') ? c - 0x20 : c;
		if (c >= 0xe0 && c <= 0xfe) return c != 0xf7 ? c - 0x20 : c;
		if (c == 0xff) return 0x178;
		// Latin Extended-A; the pairs sit next to each other, but which of
		// the two comes first changes twice along the way
		if (c >= 0x100 && c <= 0x17f) {
			if (c == 0x131) return 'I';
			if (c == 0x17f) return 'S';
			if (c == 0x138 || c == 0x149) return c;
			const bool lowerIsEven = (c >= 0x139 && c <= 0x148) || c >= 0x179;
			return ((c & 1) == (lowerIsEven ? 0 : 1)) ? c - 1 : c;
		}
		// Romanian comma-below letters
		if (c == 0x219 || c == 0x21b) return c - 1;
		// Greek
		if (c >= 0x3b1 && c <= 0x3cb) return c == 0x3c2 ? 0x3a3 : c - 0x20;
		if (c == 0x3ac) return 0x386;
		if (c >= 0x3ad && c <= 0x3af) return c - 0x25;
		if (c == 0x3cc) return 0x38c;
		if (c == 0x3cd || c == 0x3ce) return c - 0x3f;
		// Cyrillic
		if (c >= 0x430 && c <= 0x44f) return c - 0x20;
		if (c >= 0x450 && c <= 0x45f) return c - 0x50;
		if ((c >= 0x460 && c <= 0x481) || (c >= 0x48a && c <= 0x4bf) || (c >= 0x4d0 && c <= 0x52f))
			return (c & 1) ? c - 1 : c;
		if (c >= 0x4c1 && c <= 0x4ce) return (c & 1) ? c : c - 1;
		if (c == 0x4cf) return 0x4c0;
		return c;
	}

	Uint32 unicode_to_lower(Uint32 c)
	{
		// Basic Latin and Latin-1
		if (c < 0x80) return (c >= 'A' && c <= 'Z') ? c + 0x20 : c;
		if (c >= 0xc0 && c <= 0xde) return c != 0xd7 ? c + 0x20 : c;
		// Latin Extended-A
		if (c >= 0x100 && c <= 0x17f) {
			if (c == 0x130) return 'i';
			if (c == 0x178) return 0xff;
			if (c == 0x138 || c == 0x149 || c == 0x17f) return c;
			const bool upperIsOdd = (c >= 0x139 && c <= 0x148) || c >= 0x179;
			return ((c & 1) == (upperIsOdd ? 1 : 0)) ? c + 1 : c;
		}
		// Romanian comma-below letters
		if (c == 0x218 || c == 0x21a) return c + 1;
		// Greek
		if (c >= 0x391 && c <= 0x3ab) return c != 0x3a2 ? c + 0x20 : c;
		if (c == 0x386) return 0x3ac;
		if (c >= 0x388 && c <= 0x38a) return c + 0x25;
		if (c == 0x38c) return 0x3cc;
		if (c == 0x38e || c == 0x38f) return c + 0x3f;
		// Cyrillic
		if (c >= 0x410 && c <= 0x42f) return c + 0x20;
		if (c >= 0x400 && c <= 0x40f) return c + 0x50;
		if ((c >= 0x460 && c <= 0x481) || (c >= 0x48a && c <= 0x4bf) || (c >= 0x4d0 && c <= 0x52f))
			return (c & 1) ? c : c + 1;
		if (c >= 0x4c1 && c <= 0x4ce) return (c & 1) ? c + 1 : c;
		if (c == 0x4c0) return 0x4cf;
		return c;
	}

	static std::string utf8_convert_case(std::string_view str, Uint32 (*convert)(Uint32))
	{
		std::string out;
		out.reserve(str.size());

		for (size_t i = 0; i < str.size();) {
			const unsigned char lead = str[i];

			// Every letter with a case mapping here is one or two bytes long.
			// Anything else is copied as it is, one byte at a time, which
			// also carries longer characters and invalid bytes through.
			if (lead < 0x80) {
				out.push_back(char(convert(lead)));
				i += 1;
			} else if (lead >= 0xc2 && lead <= 0xdf && i + 1 < str.size() && (str[i + 1] & 0xc0) == 0x80) {
				char buf[4];
				const Uint32 chr = convert(((lead & 0x1f) << 6) | (str[i + 1] & 0x3f));
				out.append(buf, utf8_encode_char(chr, buf));
				i += 2;
			} else {
				out.push_back(str[i]);
				i += 1;
			}
		}

		return out;
	}

	std::string utf8_to_upper(std::string_view str)
	{
		return utf8_convert_case(str, unicode_to_upper);
	}

	std::string utf8_to_lower(std::string_view str)
	{
		return utf8_convert_case(str, unicode_to_lower);
	}

} // namespace Text
