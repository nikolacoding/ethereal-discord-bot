#include "MyBot.h"
#include <iostream>
#include <dpp/dpp.h>
#include <fstream>
const std::string BOT_TOKEN = [YOUR TOKEN HERE AS STRING];
const std::string LOG_FILENAME = "global_log.txt";

static bool is_id_in_vector(const uint64_t target_id, std::vector<uint64_t> vec) {
	// sekvencijalna pretraga, binarna/interpolaciona nije izvodljiva s obzirom da ne mozemo garantovati sortiranost vektora,
	// a sortirati ga prije slanja u funkciju *trenutno* nije neophodno jer radimo sa sitnim vektorima
	for (uint64_t id : vec) if (id == target_id) return true;
	return false;
}

static void write_to_file(const std::string line, const std::string filename) {
	// pisanje u fajl
	std::filebuf fb;
	fb.open(filename, std::ios::app);
	std::ostream stream(&fb);
	stream << line;
	fb.close();
}

static std::vector<std::string> splitString(std::string original) {
	// "razbijamo" string u pojedinacne rijeci (odvojene razmacima) i vracamo vektor sa svim pojedinacnim rijecima
	std::string temp;
	std::istringstream ss(original);
	std::vector<std::string> ret;

	while (ss >> temp) ret.push_back(temp);
	return ret;
}

int main()
{
	srand((unsigned)time(0));

	dpp::cluster bot(BOT_TOKEN, dpp::i_default_intents | dpp::i_message_content);
	bot.on_log(dpp::utility::cout_logger());
	bot.on_ready([&bot](const dpp::ready_t& event) {
		if (dpp::run_once<struct register_bot_commands>()) {
			std::vector<dpp::slashcommand> commands{
				{ "sazovi", "Sazovi barjake!", bot.me.id }, // /sazovi <opcija> <poruka>
				{ "pickone", "Biram jednu od ponudjenih rijeci!", bot.me.id }, // /pickone <rijeci odvojene razmakom>
				{ "rankthem", "Rangiraj ponudjene rijeci!", bot.me.id } // /rankthem <rijeci odvojene razmakom>
			};
			commands[0].add_option(dpp::command_option(dpp::co_integer, "opcija", "Opcije sazivanja: 0 -> SVI | 1 -> CS | 2 -> GTA | 3 -> PUBG", true));
			commands[0].add_option(dpp::command_option(dpp::co_string, "poruka", "Poruka pri sazivanju", false));

			commands[1].add_option(dpp::command_option(dpp::co_string, "ponudjeno", "Unesi rijeci odvojene zarezima", true));

			commands[2].add_option(dpp::command_option(dpp::co_string, "ponudjeno", "Unesi rijeci odvojene zarezima", true));

			bot.global_bulk_command_create(commands);
		}
	});

	constexpr uint64_t NIKOLA_ID = 351451733857140736;
	constexpr uint64_t VELJA_ID = 693390855532183574;
	constexpr uint64_t VULIC_ID = 118432907218911233;
	constexpr uint64_t ISAIJE_ID = 331818241834090496;
	constexpr uint64_t BUREK_ID = 483587460610261003;
	constexpr uint64_t OLA_ID = 691350585756352543;
	constexpr uint64_t DARKO_ID = 563773413445861397;
	constexpr uint64_t BOT_ID = 1241122981720096959;

	// Slash komande
	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) -> dpp::task<void> {
		if (event.command.get_command_name() == "sazovi") { // /sazovi <opcija> <poruka>
			const std::vector<uint64_t> admin_ids = { NIKOLA_ID };
			const std::vector<std::vector<uint64_t>> ids = {
				{ NIKOLA_ID, VELJA_ID, VULIC_ID, ISAIJE_ID, BUREK_ID, OLA_ID, DARKO_ID }, // SVI [0]
				{ NIKOLA_ID, VELJA_ID, VULIC_ID, ISAIJE_ID, BUREK_ID }, // CS [1]
				{ NIKOLA_ID, VELJA_ID, VULIC_ID, ISAIJE_ID, BUREK_ID, OLA_ID, DARKO_ID }, // GTA [2]
				{ NIKOLA_ID, VELJA_ID, VULIC_ID, OLA_ID, DARKO_ID }, // PUBG [3]
				{ NIKOLA_ID } // DEBUG [4]
			};
			
			if (is_id_in_vector(event.command.get_issuing_user().id, admin_ids)) {
				const std::string content_default_part1 = "Pozvan si na ";
				const std::string content_default_part2 = " danas. Izjasniti se na Ethereal serveru sto prije!";
				// ^ ruzna implementacija, popraviti kasnije
				const std::vector<std::string> igre = { "CS", "GTA", "PUBG", "DEBUG" };
				
				std::string content;
				int opcija;

				// ako komanda nije dobila obavezan parametar "poruka", pise se greska i prekida izvrsavanje
				if (!event.get_parameter("opcija").index()) {
					event.reply("Opcija nije unesena.");
					co_return;
				}
				else opcija = std::get<int64_t>(event.get_parameter("opcija"));
				if (opcija > 3 || opcija < 0) opcija = 3; // ako je izabrana opcija van ocekivanog opsega, stavlja se na DEBUG po defaultu

				// ako komanda nije dobila neobavezan parametar "poruka", 'content' uzima gorenavedenu podrazumijevanu vrijednost
				if (!event.get_parameter("poruka").index()) content = content_default_part1 + igre[opcija] + content_default_part2;
				else content = std::get<std::string>(event.get_parameter("poruka"));

				for (uint64_t id : ids[opcija]) { // prodji kroz vektor izabrane igre ('current_vec') i posalji DM svakom clanu
					dpp::snowflake user(id);
					bot.direct_message_create(user, dpp::message(content), [event, user](const dpp::confirmation_callback_t& callback) {
						if (callback.is_error()) {
							event.reply("Nije moguce poslati DM jednom ili vise korisnika.");
						}
					});
				}

				event.reply("Sazvani!");
			}
			else {
				event.reply("Nemas dozvolu da sazoves.");
			}
		}
		
		if (event.command.get_command_name() == "pickone") { // /pickone <rijeci odvojene razmakom>
			if (!event.get_parameter("ponudjeno").index()) {
				event.reply("Nisi zadao izbor.");
			}
			else {
				const std::string param = std::get<std::string>(event.get_parameter("ponudjeno"));
				const std::vector<std::string> words = splitString(param);

				int index = std::rand() % words.size();
				std::string choice = words[index];
				event.reply(choice);
			}
		}

		if (event.command.get_command_name() == "rankthem") { // /rankthem <rijeci odvojene razmakom>
			if (!event.get_parameter("ponudjeno").index()) {
				event.reply("Nisi zadao izbor.");
			}
			else {
				const std::string param = std::get<std::string>(event.get_parameter("ponudjeno"));
				std::vector<std::string> words = splitString(param);
				std::string reply = "";
				int rank = 1;

				while (!words.empty()) {
					int size = (int)words.size(); // uzima duzinu vektora 'size'
					int current = rand() % size; // bira jedan nasumican indeks [0, size)
					reply += std::to_string(rank++) + ". " + words[current] + "\n"; // 1. jednaopcija\n2. drugaopcija\n ...
					words.erase(words.begin() + current); // brise uzetu rijec iz vektora
				}

				event.reply(reply);
			}
		}
		
		// nova komanda

		co_return;
	});

	// Detekcija poruka u kanalu bez slasha ili bilo kakvog specijalnog formata
	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		auto parseMessage = [](const std::string original) -> std::string {
			// kopira string iz argumenta (posto je poslan po vrijednosti), pretvara sva slova u mala i vraca
			std::string ret_val = original;
			for (int i = 0; i < ret_val.size(); i++) {
				char* pc = &ret_val[i];
				*pc = tolower(*pc);
			}
			return ret_val;
		};
		auto getSubstr = [](const std::string original, const int skip) -> std::string {
			// preskace prvih 'int skip' charova stringa i vraca sve poslije
			return original.substr(skip, original.size() - skip);
		};
		auto logMessage = [](const std::string author_name, const std::string content, dpp::channel *channel_ptr) -> void {
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
		};

		const dpp::message msg = event.msg;
		// dpp::snowflake channel_sfid(msg.channel_id);
		dpp::channel *channelptr = dpp::find_channel(msg.channel_id);

		const uint64_t self_id = BOT_ID;
		const uint64_t msg_author_id = msg.author.id;

		const std::string say_do_prompt = "botino reci";
		const std::string burek_prompt = "ne mogu";
		const int say_do_prompt_length = (int)say_do_prompt.size();

		const std::string msg_str_raw = msg.content;
		const std::string msg_str_parsed = parseMessage(msg_str_raw);

		logMessage(msg.author.username, msg_str_raw, channelptr);
		if (msg_str_parsed.find(say_do_prompt) != std::string::npos && msg_author_id != self_id) {
			std::string reply = getSubstr(msg_str_raw, say_do_prompt_length);
			event.reply(reply, false);
			return;
		}

		if (msg_author_id == BUREK_ID && msg_str_parsed.find(burek_prompt) != std::string::npos) {
			event.reply("gej", false);
			return;
		}
	});

	bot.start(dpp::st_wait);
	return 0;
}
