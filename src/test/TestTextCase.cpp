// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "doctest.h"
#include "text/TextSupport.h"

#include <string>

TEST_CASE("UTF-8 upper and lower case")
{
	using Text::utf8_to_lower;
	using Text::utf8_to_upper;

	SUBCASE("plain English is converted as before")
	{
		CHECK(utf8_to_upper("Hull 100%, fuel_scoop") == "HULL 100%, FUEL_SCOOP");
		CHECK(utf8_to_lower("Hull 100%, FUEL_SCOOP") == "hull 100%, fuel_scoop");
		CHECK(utf8_to_upper("") == "");
	}

	SUBCASE("Cyrillic")
	{
		CHECK(utf8_to_upper("Ии") == "ИИ");
		CHECK(utf8_to_lower("Ии") == "ии");
		CHECK(utf8_to_upper("водород, ёж, їжак") == "ВОДОРОД, ЁЖ, ЇЖАК");
		CHECK(utf8_to_lower("ВОДОРОД, ЁЖ, ЇЖАК") == "водород, ёж, їжак");
	}

	SUBCASE("Greek")
	{
		CHECK(utf8_to_upper("υδρογόνο") == "ΥΔΡΟΓΌΝΟ");
		CHECK(utf8_to_lower("ΥΔΡΟΓΌΝΟ") == "υδρογόνο");
		// a final sigma has the same capital as any other sigma
		CHECK(utf8_to_upper("σοφός") == "ΣΟΦΌΣ");
	}

	SUBCASE("accented Latin")
	{
		CHECK(utf8_to_upper("wodór, žluťoučký kůň, ÿ") == "WODÓR, ŽLUŤOUČKÝ KŮŇ, Ÿ");
		CHECK(utf8_to_lower("WODÓR, ŽLUŤOUČKÝ KŮŇ, Ÿ") == "wodór, žluťoučký kůň, ÿ");
		CHECK(utf8_to_upper("ősz, łódź, șold, ąę") == "ŐSZ, ŁÓDŹ, ȘOLD, ĄĘ");
		CHECK(utf8_to_lower("ŐSZ, ŁÓDŹ, ȘOLD, ĄĘ") == "ősz, łódź, șold, ąę");
		// signs that sit among the letters are not letters
		CHECK(utf8_to_upper("2 × 3 ÷ 4") == "2 × 3 ÷ 4");
		CHECK(utf8_to_lower("2 × 3 ÷ 4") == "2 × 3 ÷ 4");
		// no single capital letter exists for these
		CHECK(utf8_to_upper("ß") == "ß");
	}

	SUBCASE("other scripts and broken text pass through")
	{
		CHECK(utf8_to_upper("日本語 한국어 العربية €") == "日本語 한국어 العربية €");
		CHECK(utf8_to_lower("日本語 한국어 العربية €") == "日本語 한국어 العربية €");

		// a lead byte with nothing after it, and a stray continuation byte
		const std::string broken = std::string("ab\xd0") + "\x80" + "cd\xd0";
		CHECK(utf8_to_upper(broken) == std::string("AB\xd0") + "\x80" + "CD\xd0");

		// a zero byte in the middle does not cut the string short
		const std::string withZero("a\0\xd0\xb8", 4);
		CHECK(utf8_to_upper(withZero) == std::string("A\0\xd0\x98", 4));
	}
}
