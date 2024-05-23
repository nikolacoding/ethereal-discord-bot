#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <dpp/dpp.h>

#include "Constants.cpp"

namespace slc {
	// Provjera poziva komande prema imenu
	extern bool get_command(const dpp::slashcommand_t& event, const std::string command_name) {
		if (event.command.get_command_name() == command_name) return true;
		return false;
	}

	// Provjera postojanja parametra prema imenu
	extern bool parameter_exists(const dpp::slashcommand_t& event, const std::string parameter_name) {
		if (event.get_parameter(parameter_name).index()) return true;
		return false;
	}

	// Uzimanje potpune vrijednosti korisnicki-definisanog parametra tipa uint64_t
	extern int64_t get_parameter_value_int(const dpp::slashcommand_t& event, const std::string parameter_name) {
		int64_t return_value = std::get<int64_t>(event.get_parameter(parameter_name));
		return return_value;
	}

	// Uzimanje potpune vrijednosti korisnicki-definisanog parametra tipa std::string
	extern std::string get_parameter_value_string(const dpp::slashcommand_t& event, const std::string parameter_name) {
		std::string return_value = std::get<std::string>(event.get_parameter(parameter_name));
		return return_value;
	}

	extern void add_command_parameter(dpp::slashcommand& command, const std::string type, const std::string command_name, const std::string parameter_description, const bool required) {
		if (hpf::lowercase_parse(type) == "int")
			command.add_option(dpp::command_option(dpp::co_integer, command_name, parameter_description, required));

		if (hpf::lowercase_parse(type) == "string")
			command.add_option(dpp::command_option(dpp::co_string, command_name, parameter_description, required));
	}
}