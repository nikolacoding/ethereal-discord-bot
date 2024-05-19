#include <iostream>
#include <dpp/dpp.h>
#include <ctime>
// Implementacija sa direktnim pristupom .cpp datotekama umjesto rada sa headerima zbog odredjenih problema pri kompilaciji
#include "Constants.cpp"
#include "Helpers.cpp"
#include "token.cpp"

const extern std::string BOT_TOKEN; // from "token.cpp"
// const std::string BOT_TOKEN = "-->YOUR TOKEN GOES HERE AS A STRING<--";
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
				{ "rankthem", "Rangiraj ponudjene rijeci!", bot.me.id }, // /rankthem <rijeci odvojene razmakom>
				{ "info", "Ispisi detaljne informacije o sebi.", bot.me.id}
			};
			commands[0].add_option(dpp::command_option(dpp::co_integer, "opcija", "Opcije sazivanja: 0 -> SVI | 1 -> CS | 2 -> GTA | 3 -> PUBG", true));
			commands[0].add_option(dpp::command_option(dpp::co_string, "poruka", "Poruka pri sazivanju", false));

			commands[1].add_option(dpp::command_option(dpp::co_string, "ponudjeno", "Unesi rijeci odvojene zarezima za izbor", true));

			commands[2].add_option(dpp::command_option(dpp::co_string, "ponudjeno", "Unesi rijeci odvojene zarezima za rangiranje", true));

			bot.global_bulk_command_create(commands);
		}
	});

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
				const std::vector<std::string> words = split_string(param);

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
				std::vector<std::string> words = split_string(param);
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
		
		if (event.command.get_command_name() == "info") {
			const dpp::user issuing_user = event.command.get_issuing_user();
			const std::string username = issuing_user.global_name;

			const double creation_time_unix = issuing_user.get_creation_time();
			// napisati funkciju za konverziju unix vremena u ljudsko
			std::string time_formatted;

			dpp::embed embed = dpp::embed().
				set_color(0xFF9900).
				set_title(username).
				set_description("").
				set_thumbnail("https://i.imgur.com/vnQ2izi.jpeg").
				add_field(
					"Datum kreiranja naloga",
					time_formatted
				).
				add_field(
					"Datum ulaska na server",
					"1.1.1974.",
					true
				).
				set_footer(dpp::embed_footer().set_text("mudo labudovo"));
			embed.timestamp = time(0);
			bot.message_create(dpp::message(event.command.channel_id, embed));
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

		log_message(msg.author.username, msg_str_raw, channelptr);
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
