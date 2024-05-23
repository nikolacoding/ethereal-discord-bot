#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <dpp/dpp.h>

#include "Constants.cpp"

namespace hpf {
	// Pisanje u imenovani fajl
	extern void write_to_file(const std::string line, const std::string filename) {
		std::filebuf fb;
		fb.open(filename, std::ios::app);
		std::ostream stream(&fb);
		stream << line;
		fb.close();
	}

	// Formatirano ispisivanje korisnicki-generisanih poruka u stdout
	// za dodati: hendlovanje @-ovima i fotografijama
	extern void log_message(const std::string author_name, const std::string content, dpp::channel* channel_ptr) {
		uint64_t current_channel_id;
		if (channel_ptr) { // iz nekog Gospodnjeg razloga ovaj pokazivac je null kada je u pitanju botov event.reply??? 
			current_channel_id = channel_ptr->id;
			const std::vector<uint64_t> channel_ids = { 418750273977188354, 559780927547375617, 655442107644903434, 655442211755786240 };
			const std::vector<std::string> channel_names = { "djeneral", "botovi-i-muzika", "visoko-vijece", "isusovci" };
			std::string channel_cout_name = channel_ptr->name;

			// s obzirom da stdout radi iskljucivo na ASCII-ju, imena kanala na cirilici ispisuju hijeroglife, te to popravljamo na ovaj nacin:
			// trazimo da li se neki od cirilicno-imenovanih kanala poklapa sa ID-jevima u 'channel_names' te ga mapira na 'channel_ids' element istog indeksa
			// ukoliko ga ne nadje, gore je vec navedena podrazumijevana vrijednost koja je ime trenutnog kanala
			for (int i = 0; i < channel_names.size(); i++) {
				uint64_t id = channel_ids[i];;
				if (id == current_channel_id) {
					channel_cout_name = channel_names[i];
					break;
				}
			}
			// provjeravamo da li je kanal gdje je dospjela zadnja poruka razlicit od kanala sa novom porukom, ukoliko jeste - ispisujemo i ime kanala
			// bez ove provjere bi se desilo ili da se ime kanala ispisuje za svaku poruku, ili ni za jednu, cineci stdout jako neurednim
			static dpp::channel* prev_channel = nullptr;
			if (prev_channel != channel_ptr) {
				std::string out_string_channelname = "\n[" + channel_cout_name + "]\n";
				write_to_file(out_string_channelname, con::LOG_FILENAME);
				std::cout << out_string_channelname;
				prev_channel = channel_ptr;
			}
		}
		std::string out_string = author_name + ": " + content + "\n";
		write_to_file(out_string, con::LOG_FILENAME);
		std::cout << out_string;
	}

	// sekvencijalna pretraga, binarna/interpolaciona nije izvodljiva s obzirom da ne mozemo garantovati sortiranost vektora,
	// a sortirati ga prije slanja u funkciju *trenutno* nije neophodno jer radimo sa sitnim vektorima
	// -> potencijalno se moze izmijeniti da funkcionise sa proizvoljnim tipovima
	extern bool is_id_in_vector(const uint64_t target_id, std::vector<uint64_t> vec) {
		for (uint64_t id : vec) if (id == target_id) return true;
		return false;
	}

	// "razbijamo" string u pojedinacne rijeci (odvojene razmacima) i vracamo vektor sa svim pojedinacnim rijecima
	extern std::vector<std::string> split_string(std::string original) {
		std::string temp;
		std::istringstream ss(original);
		std::vector<std::string> ret;

		while (ss >> temp) ret.push_back(temp);
		return ret;
	}

	// Gledamo da li je dati int64_t parametar unutar zadanih granica; ukoliko ne, podesiti ga na fallback_value
	extern void bounds_handler_int(int64_t& value, const int64_t fallback_value, const int64_t upper_bound, const int64_t lower_bound) {
		if (value < lower_bound || value > upper_bound)
			value = fallback_value;
	}

	// Konverzija Unix/epoch vremena u formatiran string sa ljudskim vremenom sljedeceg formata:
	// DD.M(M).GGGG. HH:MM:SS <- sati su opcionalni u zavisnosti od vrijednosti 'include_time'
	extern std::string format_unix_time(const double time_raw, const bool include_time = true) {
		const int64_t seconds_in_day = 86400; // 24 * 60 * 60
		const std::vector<int> days_in_month = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		const int64_t total_seconds = (int64_t)time_raw; // casting double -> int

		int64_t total_days = total_seconds / seconds_in_day; // ukupne sekunde / broj sekundi u danu
		int64_t leftover_time = total_seconds % seconds_in_day; // ostatak pri ^ cjelobrojnom dijeljenju

		int64_t day = 1, month = 1, year = 1970, sec = 0, min = 0, hr = 0;

		// lambda funkcija za brzinsko odbrojavanje odredjenog parametra vremena u zavisnosti od ukupnih sekundi 'total_time'
		auto iterate_down_time = [](int64_t& total_time, int64_t& count, int64_t step) -> void {
			while (total_time > step) {
				count++;
				total_time -= step;
			}
			};

		// posebno odbrojavanje godina s obzirom da je logika racunanja prestupnih godina odvojena
		while (total_days >= 365) {
			int subtr;
			if (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) subtr = 366;
			else subtr = 365;
			total_days -= subtr;
			year++;
		}

		// posebno odbrojavanje mjeseci s obzirom da broj dana u mjesecu varira od mjeseca do mjeseca
		int month_index = 0;
		int curr_month_days = days_in_month[month_index]; // pocinjemo na januaru (indeks 0)
		while (total_days >= curr_month_days) {
			total_days -= curr_month_days;
			month++;

			month_index = (month_index + 1) % 12;
			curr_month_days = days_in_month[month_index];
		}

		iterate_down_time(leftover_time, hr, 3600);
		iterate_down_time(leftover_time, min, 60);

		sec = leftover_time;
		day = total_days + 1; // broji se do prethodnog dana, tako da dodajemo 1 za simulaciju brojanja do danasnjeg

		std::string final_string = "";
		final_string += std::to_string(day) + ".";
		final_string += std::to_string(month) + ".";
		final_string += std::to_string(year) + ".";
		if (include_time) {
			final_string += " ";
			final_string += std::to_string(hr) + ".";
			final_string += std::to_string(min) + ":";
			final_string += std::to_string(sec);
		}

		return final_string;
	}

	extern int64_t days_since(const double time_raw) {
		const int64_t seconds_in_day = 86400; // 24 * 60 * 60
		return (int64_t)time_raw / seconds_in_day;
	}

	extern std::string say_do_string_parser(const std::string original, const std::string prompt) {
		return original.substr(prompt.size(), original.size() - prompt.size());
	}

	// Kopira string iz argumenta (posto je poslan po vrijednosti), pretvara sva slova u mala i vraca za poredjenje stringova gdje
	// velicina slova nije relevantna koliko i sam sadrzaj
	extern std::string lowercase_parse(const std::string original) {
		std::string ret_val = original;
		for (int i = 0; i < ret_val.size(); i++) {
			char* pc = &ret_val[i];
			*pc = tolower(*pc);
		}
		return ret_val;
	}

	// Provjerava da li se u stringu 'original' nalazi string 'substring'. 
	// std::string.find() vraca std::string::npos ukoliko substring nije pronadjen/validan, a ova funkcija je jedan nivo apstrakcije iznad
	extern bool contains_substring(const std::string original, const std::string substring) {
		return original.find(substring) != std::string::npos;
	}
}