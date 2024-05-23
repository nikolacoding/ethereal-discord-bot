#include <iostream>
#include <dpp/dpp.h>
#include <ctime>
#include <windows.h>
// Implementacija sa direktnim pristupom .cpp datotekama umjesto rada sa headerima zbog odredjenih problema pri kompilaciji
#include "Helpers.cpp"
#include "SlashCommands.cpp"
#include "Constants.cpp"
#include "token.cpp"

/*
	Standardni namespaceovi:
	"hpf::" iz Helpers.cpp -> pomocne funkcije
	"slc::" iz SlashCommands.cpp -> funkcije rada sa slash komandama
	"con::" iz Constants.cpp -> konstante
*/

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
				{ "sazovi", "Sazovi barjake!", bot.me.id }, //						/sazovi <opcija> <poruka>
				{ "pickone", "Biram jednu od ponudjenih rijeci!", bot.me.id }, //	/pickone <rijeci odvojene razmakom>
				{ "rankthem", "Rangiraj ponudjene rijeci!", bot.me.id }, //			/rankthem <rijeci odvojene razmakom>
				{ "info", "Ispisi detaljne informacije o sebi.", bot.me.id} //		/info
			};

			slc::add_command_parameter(commands[0], "int", "opcija", "Opcije sazivanja: 0 -> SVI | 1 -> CS | 2 -> GTA | 3 -> PUBG", true);
			slc::add_command_parameter(commands[0], "string", "poruka", "Poruka pri sazivanju", false);
			slc::add_command_parameter(commands[1], "string", "ponudjeno", "Unesi rijeci odvojene zarezima za izbor", true);
			slc::add_command_parameter(commands[2], "string", "ponudjeno", "Unesi rijeci odvojene zarezima za rangiranje", true);

			bot.global_bulk_command_create(commands);
		}
	});

	// Slash komande
	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) -> dpp::task<void> {
		if (slc::get_command(event, "sazovi")) { // /sazovi <opcija> <poruka>
			if (hpf::is_id_in_vector(event.command.get_issuing_user().id, con::ADMIN_IDS)) {
				int64_t opcija;
				std::string content;

				if (!slc::parameter_exists(event, "opcija")) {
					event.reply("Opcija nije unesena.");
					co_return;
				}
				else opcija = slc::get_parameter_value_int(event, "opcija");
				hpf::bounds_handler_int(opcija, 4, 0, 4); // 0 - 4, default: 4

				// ako komanda nije dobila neobavezan parametar "poruka", 'content' uzima podrazumijevanu vrijednost
				if (!slc::parameter_exists(event, "poruka")) content = con::CONTENT_DEFAULT_PREGAMENAME + con::GAMES[opcija] + con::CONTENT_DEFAULT_POSTGAMENAME;
				else content = slc::get_parameter_value_string(event, "poruka");

				for (uint64_t id : con::IDS[opcija]) { // prodji kroz vektor izabrane igre i posalji DM svakom clanu
					dpp::snowflake user_sfid(id);
					bot.direct_message_create(user_sfid, dpp::message(content));
					Sleep(1000); // ne znam da li ovo pomaze da se obidje potencijalni rate limit, pogledati poslije, trenutno radi normalno
				}

				event.reply("Sazvani!");
				co_return;
			}
			else {
				event.reply("Nemas dozvolu da sazoves.");
				co_return;
			}
		}
		
		if (slc::get_command(event, "pickone")) { // /pickone <rijeci odvojene razmakom>
			if (!slc::parameter_exists(event, "ponudjeno")) {
				event.reply("Nisi zadao izbor.");
				co_return;
			}
			else {
				const std::string param = slc::get_parameter_value_string(event, "ponudjeno");
				const std::vector<std::string> words = hpf::split_string(param);

				int index = std::rand() % words.size();
				std::string choice = words[index];
				event.reply(choice);

				co_return;
			}
		}

		if (slc::get_command(event, "rankthem")) { // /rankthem <rijeci odvojene razmakom>
			if (!slc::parameter_exists(event, "ponudjeno")) {
				event.reply("Nisi zadao izbor.");
				co_return;
			}
			else {
				const std::string param = slc::get_parameter_value_string(event, "ponudjeno");
				std::vector<std::string> words = hpf::split_string(param);
				std::string reply = "";
				int rank = 1;

				while (!words.empty()) {
					int size = (int)words.size(); // uzima duzinu vektora 'size'
					int current = rand() % size; // bira jedan nasumican indeks [0, size)
					reply += std::to_string(rank++) + ". " + words[current] + "\n"; // 1. jednaopcija\n2. drugaopcija\n ...
					words.erase(words.begin() + current); // brise uzetu rijec iz vektora
				}

				event.reply(reply);
				co_return;
			}
		}
		
		if (slc::get_command(event, "info")) {
			const dpp::user issuing_user = event.command.get_issuing_user();
			const dpp::resolved_user r_issuing_user(issuing_user);
			int64_t server_join_date_raw = static_cast<int64_t>(r_issuing_user.member.joined_at);
			// ^ ovo je 0 iz nekog razloga

			const std::string username = issuing_user.global_name;
			const std::string account_creation_date = hpf::format_unix_time(issuing_user.get_creation_time(), true); // vrijeme pravljenja naloga
			const std::string server_join_date = hpf::format_unix_time(server_join_date_raw);
			const std::string user_profile_picture_url = issuing_user.get_avatar_url();

			dpp::embed embed = dpp::embed().
				set_color(0xFF9900).
				set_title(username).
				set_description("").
				set_thumbnail(user_profile_picture_url).
				add_field(
					"Datum kreiranja naloga",
					account_creation_date,
					true
				).
				add_field(
					"Datum ulaska na server",
					server_join_date,
					true
				).
				set_footer(dpp::embed_footer().set_text("Clan je " + std::to_string(hpf::days_since(server_join_date_raw)) + " dana."));
			embed.timestamp = time(0);
			bot.message_create(dpp::message(event.command.channel_id, embed));

			event.reply("Pisu se informacije o tebi: ");
			co_return;
		}

		// nova komanda
		co_return;
	});

	// Detekcija poruka u kanalu bez slasha ili bilo kakvog specijalnog formata
	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		const dpp::message msg = event.msg;
		dpp::channel *channelptr = dpp::find_channel(msg.channel_id); // pokazivac na instancu trenutnog kanala

		const uint64_t self_id = con::BOT_ID;
		const uint64_t msg_author_id = msg.author.id;

		const std::string msg_str_raw = msg.content; // raw message content
		const std::string msg_str_parsed = hpf::lowercase_parse(msg.content); // all-lowercase message content

		hpf::log_message(msg.author.username, msg_str_raw, channelptr);
		if (hpf::contains_substring(msg_str_parsed, con::SAY_DO_PROMPT) && msg_author_id != self_id) {
			std::string reply = hpf::say_do_string_parser(msg_str_raw, con::SAY_DO_PROMPT);
			event.reply(reply, false);
			return;
		}

		if (msg_author_id == con::BUREK_ID && hpf::contains_substring(msg_str_parsed, con::BUREK_PROMPT)) {
			event.reply(con::BUREK_REPLY, false);
			return;
		}
	});

	bot.start(dpp::st_wait);
	return 0;
}
