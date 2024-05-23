#pragma once
#include <string>
#include <vector>

namespace con {
	// Fajl za ispis
	extern std::string LOG_FILENAME = "global_log.txt";

	// Korisnici
	extern const uint64_t NIKOLA_ID = 351451733857140736;
	extern const uint64_t VELJA_ID = 693390855532183574;
	extern const uint64_t VULIC_ID = 118432907218911233;
	extern const uint64_t ISAIJE_ID = 331818241834090496;
	extern const uint64_t BUREK_ID = 483587460610261003;
	extern const uint64_t OLA_ID = 691350585756352543;
	extern const uint64_t DARKO_ID = 563773413445861397;
	extern const uint64_t BOT_ID = 1241122981720096959;
	extern const std::vector<uint64_t> ADMIN_IDS = { NIKOLA_ID };
	extern const std::vector<std::vector<uint64_t>> IDS = {
		{ NIKOLA_ID, VELJA_ID, VULIC_ID, ISAIJE_ID, BUREK_ID, OLA_ID, DARKO_ID }, // SVI [0]
		{ NIKOLA_ID, VELJA_ID, VULIC_ID, ISAIJE_ID, BUREK_ID }, // CS [1]
		{ NIKOLA_ID, VELJA_ID, VULIC_ID, ISAIJE_ID, BUREK_ID, OLA_ID, DARKO_ID }, // GTA [2]
		{ NIKOLA_ID, VELJA_ID, VULIC_ID, OLA_ID, DARKO_ID }, // PUBG [3]
		{ NIKOLA_ID, VELJA_ID } // DEBUG [4]
	};
	extern const std::vector<std::string> GAMES = { "CS", "GTA", "PUBG", "DEBUG" };

	extern const std::string CONTENT_DEFAULT_PREGAMENAME = "Pozvan si na ";
	extern const std::string CONTENT_DEFAULT_POSTGAMENAME = " danas. Izjasniti se na Ethereal serveru sto prije!";
	extern const std::string SAY_DO_PROMPT = "botino reci";
	extern const std::string BUREK_PROMPT = "ne mogu";
	extern const std::string BUREK_REPLY = "gej";
}