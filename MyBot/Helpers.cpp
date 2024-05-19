#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "Constants.cpp"

void write_to_file(const std::string line, const std::string filename) {
	// pisanje u fajl
	std::filebuf fb;
	fb.open(filename, std::ios::app);
	std::ostream stream(&fb);
	stream << line;
	fb.close();
}

void log_message(const std::string author_name, const std::string content, dpp::channel* channel_ptr){
	// ispisuje kanal, autora i poruku na stdout
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
			write_to_file(out_string_channelname, LOG_FILENAME);
			std::cout << out_string_channelname;
			prev_channel = channel_ptr;
		}
	}
	std::string out_string = author_name + ": " + content + "\n";
	write_to_file(out_string, LOG_FILENAME);
	std::cout << out_string;
}

bool is_id_in_vector(const uint64_t target_id, std::vector<uint64_t> vec) {
	// sekvencijalna pretraga, binarna/interpolaciona nije izvodljiva s obzirom da ne mozemo garantovati sortiranost vektora,
	// a sortirati ga prije slanja u funkciju *trenutno* nije neophodno jer radimo sa sitnim vektorima
	for (uint64_t id : vec) if (id == target_id) return true;
	return false;
}

std::vector<std::string> split_string(std::string original) {
	// "razbijamo" string u pojedinacne rijeci (odvojene razmacima) i vracamo vektor sa svim pojedinacnim rijecima
	std::string temp;
	std::istringstream ss(original);
	std::vector<std::string> ret;

	while (ss >> temp) ret.push_back(temp);
	return ret;
}