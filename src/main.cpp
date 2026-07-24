/*----------------------------------------------------------
This Source Code Form is subject to the terms of the 
Mozilla Public License, v.2.0. If a copy of the MPL 
was not distributed with this file, You can obtain one 
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////
//	Author:			disa_da
//	E-mail:			disa_da2@mail.ru
/////////////////////////////////////////////////////////////////////////////

/**
    2014-2022       dmpas       sergey(dot)batanov(at)dmpas(dot)ru
    2019-2020       fishca      fishcaroot(at)gmail(dot)com
 */

// main.cpp : Defines the entry point for the console application.
//

#include "V8File.h"
#include "version.h"
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;
using namespace v8unpack;

typedef int (*handler_t)(vector<string> &argv);
void read_param_file(const char *filename, vector< vector<string> > &list);
handler_t get_run_mode(const vector<string> &args, int &arg_base, bool &allow_listfile);

static bool option_quiet = false;
static bool option_verbose = false;
static bool option_json = false;
static bool option_force = false;

static string json_escape(const string &value)
{
	ostringstream out;
	for (unsigned char c : value) {
		switch (c) {
		case '\\': out << "\\\\"; break;
		case '"': out << "\\\""; break;
		case '\n': out << "\\n"; break;
		case '\r': out << "\\r"; break;
		case '\t': out << "\\t"; break;
		default: out << static_cast<char>(c); break;
		}
	}
	return out.str();
}

struct file_info_t {
	bool exists = false;
	bool valid = false;
	bool format16 = false;
	uintmax_t size = 0;
};

static file_info_t inspect_file(const string &filename)
{
	file_info_t result;
	std::error_code ec;
	result.exists = std::filesystem::is_regular_file(filename, ec);
	if (!result.exists || ec) return result;
	result.size = std::filesystem::file_size(filename, ec);
	ifstream input(filename, ios::binary);
	result.valid = input && IsV8File(input);
	if (result.valid) result.format16 = IsV8File16(input);
	return result;
}

static string file_info_json(const string &filename, const file_info_t &info)
{
	ostringstream out;
	out << "{\"file\":\"" << json_escape(filename) << "\","
		<< "\"exists\":" << (info.exists ? "true" : "false") << ","
		<< "\"valid\":" << (info.valid ? "true" : "false") << ","
		<< "\"size\":" << info.size << ","
		<< "\"format\":\"" << (info.valid ? (info.format16 ? "8.3.16+" : "legacy") : "unknown") << "\","
		<< "\"toolVersion\":\"" << V8P_VERSION << "\"}";
	return out.str();
}

static bool output_available(const string &path)
{
	if (option_force || !std::filesystem::exists(path)) return true;
	cerr << "Output already exists (use --force): " << path << endl;
	return false;
}

class null_buffer_t : public std::streambuf {
protected:
	int overflow(int c) override { return c; }
};

int usage(vector<string> &)
{
	cout << endl;
	cout << "v8unpack " << V8P_VERSION << " (" << V8P_BUILD_SIGNATURE << ")"
		 << " Copyright (c) " << V8P_RIGHT << endl;

	cout << endl;
	cout << "Unpack, pack, deflate and inflate 1C v8 file (*.cf)" << endl;
	cout << endl;
	cout << "V8UNPACK" << endl;
	cout << "  -U[NPACK]            in_filename.cf     out_dirname [block_name]" << endl;
	cout << "  -U[NPACK]  -L[IST]   listfile" << endl;
	cout << "  -PA[CK]              in_dirname         out_filename.cf" << endl;
	cout << "  -PA[CK]    -L[IST]   listfile" << endl;
	cout << "  -I[NFLATE]           in_filename.data   out_filename" << endl;
	cout << "  -I[NFLATE] -L[IST]   listfile" << endl;
	cout << "  -D[EFLATE]           in_filename        filename.data" << endl;
	cout << "  -D[EFLATE] -L[IST]   listfile" << endl;
	cout << "  -P[ARSE]             in_filename        out_dirname [block_name1 block_name2 ...]" << endl;
	cout << "  -P[ARSE]   -L[IST]   listfile" << endl;
	cout << "  -B[UILD] [-N[OPACK]] in_dirname         out_filename" << endl;
	cout << "  -B[UILD] [-N[OPACK]] -L[IST] listfile" << endl;
	cout << "  -L[IST]              listfile" << endl;
	
	cout << "  -LISTFILES|-LF       in_filename" << endl;
	cout << "  check                in_filename [--json]" << endl;
	cout << "  info                 in_filename [--json]" << endl;
	cout << "  manifest             in_filename out_manifest.json" << endl;
	cout << "  Global options: --force --quiet --verbose --json" << endl;

	cout << "  -E[XAMPLE]" << endl;
	cout << "  -BAT" << endl;
	cout << "  -V[ERSION]" << endl;

	return 0;
}

int version(vector<string> &)
{
	cout << V8P_VERSION << " (" << V8P_BUILD_SIGNATURE << ")" << endl;
	return 0;
}

int inflate(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	int ret = Inflate(argv[0], argv[1]);
	return ret;
}

int deflate(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	int ret = Deflate(argv[0], argv[1]);
	return ret;
}

int unpack(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	int ret = UnpackToFolder(argv[0], argv[1], argv[2], true);
	return ret;
}

int pack(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	int ret = PackFromFolder(argv[0], argv[1]);
	return ret;
}

int parse(vector<string> &argv)
{

	if (argv.size() < 2) {
		return V8UNPACK_SHOW_USAGE;
	}
	if (!output_available(argv[1])) return 1;

	vector<string> filter;
	for (size_t i = 2; i < argv.size(); i++) {
		if (!argv[i].empty()) {
			filter.push_back(argv[i]);
		}
	}

	return Parse(argv[0], argv[1], filter);
}

int list_files(vector<string> &argv)
{
	if (argv.empty() || argv[0].empty()) return V8UNPACK_SHOW_USAGE;
	int ret = ListFiles(argv[0]);
	return ret;
}

int process_list(vector<string> &argv)
{
	if (argv.empty()) {
		return V8UNPACK_SHOW_USAGE;
	}

	vector< vector<string> > commands;
	read_param_file(argv.at(0).c_str(), commands);

	for (auto command : commands) {

		int arg_base = 0;
		bool allow_listfile = false;

		handler_t handler = get_run_mode(command, arg_base, allow_listfile);

		command.erase(command.begin());
		int ret = handler(command);
		if (ret != 0) {
			// выходим по первой ошибке
			return ret;
		}
	}

	return 0;
}

int bat(vector<string> &)
{
	cout << "if %1 == P GOTO PACK" << endl;
	cout << "if %1 == p GOTO PACK" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":UNPACK" << endl;
	cout << "V8Unpack.exe -unpack      %2                              %2.unp" << endl;
	cout << "V8Unpack.exe -undeflate   %2.unp\\metadata.data            %2.unp\\metadata.data.und" << endl;
	cout << "V8Unpack.exe -unpack      %2.unp\\metadata.data.und        %2.unp\\metadata.unp" << endl;
	cout << "GOTO END" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":PACK" << endl;
	cout << "V8Unpack.exe -pack        %2.unp\\metadata.unp            %2.unp\\metadata_new.data.und" << endl;
	cout << "V8Unpack.exe -deflate     %2.unp\\metadata_new.data.und   %2.unp\\metadata.data" << endl;
	cout << "V8Unpack.exe -pack        %2.unp                         %2.new.cf" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << ":END" << endl;

	return 0;
}

int example(vector<string> &)
{
	cout << "" << endl;
	cout << "" << endl;
	cout << "UNPACK" << endl;
	cout << "V8Unpack.exe -unpack      1Cv8.cf                         1Cv8.unp" << endl;
	cout << "V8Unpack.exe -undeflate   1Cv8.unp\\metadata.data          1Cv8.unp\\metadata.data.und" << endl;
	cout << "V8Unpack.exe -unpack      1Cv8.unp\\metadata.data.und      1Cv8.unp\\metadata.unp" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << "PACK" << endl;
	cout << "V8Unpack.exe -pack        1Cv8.unp\\metadata.unp           1Cv8.unp\\metadata_new.data.und" << endl;
	cout << "V8Unpack.exe -deflate     1Cv8.unp\\metadata_new.data.und  1Cv8.unp\\metadata.data" << endl;
	cout << "V8Unpack.exe -pack        1Cv8.und                        1Cv8_new.cf" << endl;
	cout << "" << endl;
	cout << "" << endl;

	return 0;
}

int build(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	int ret = BuildCfFile(argv[0], argv[1], false);
	return ret;
}

int build_nopack(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	int ret = BuildCfFile(argv[0], argv[1], true);
	return ret;
}

int check(vector<string> &argv)
{
	if (argv.empty() || argv[0].empty()) return V8UNPACK_SHOW_USAGE;
	const auto info = inspect_file(argv[0]);
	if (option_json) {
		cout << file_info_json(argv[0], info) << endl;
	} else if (!option_quiet) {
		cout << argv[0] << ": " << (info.valid ? "valid" : "invalid") << endl;
	}
	return info.valid ? 0 : 2;
}

int info(vector<string> &argv)
{
	if (argv.empty() || argv[0].empty()) return V8UNPACK_SHOW_USAGE;
	const auto result = inspect_file(argv[0]);
	if (option_json) {
		cout << file_info_json(argv[0], result) << endl;
	} else if (!option_quiet) {
		cout << "File: " << argv[0] << endl
			 << "Size: " << result.size << " bytes" << endl
			 << "Valid container: " << (result.valid ? "yes" : "no") << endl
			 << "Format: " << (result.valid ? (result.format16 ? "8.3.16+" : "legacy") : "unknown") << endl;
	}
	return result.valid ? 0 : 2;
}

int manifest(vector<string> &argv)
{
	if (argv.size() < 2 || argv[0].empty() || argv[1].empty()) return V8UNPACK_SHOW_USAGE;
	if (!output_available(argv[1])) return 1;
	const auto result = inspect_file(argv[0]);
	ofstream output(argv[1], ios::binary);
	if (!output) return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
	output << file_info_json(argv[0], result) << '\n';
	if (!option_quiet && !option_json) cout << "Manifest written: " << argv[1] << endl;
	if (option_json) cout << file_info_json(argv[0], result) << endl;
	return result.valid ? 0 : 2;
}

handler_t get_run_mode(const vector<string> &args, int &arg_base, bool &allow_listfile)
{
	if (args.size() - arg_base < 1) {
		allow_listfile = false;
		return usage;
	}

	allow_listfile = true;
	string cur_mode(args[arg_base]);
	transform(cur_mode.begin(), cur_mode.end(), cur_mode.begin(),
			[](unsigned char c) { return static_cast<char>(tolower(c)); });

	arg_base += 1;
	if (cur_mode == "-version" || cur_mode == "-v" || cur_mode == "--version" || cur_mode == "version") {
		allow_listfile = false;
		return version;
	}

	if (cur_mode == "-inflate" || cur_mode == "-i" || cur_mode == "-und" || cur_mode == "-undeflate" || cur_mode == "inflate") {
		return inflate;
	}

	if (cur_mode == "-deflate" || cur_mode == "-d" || cur_mode == "deflate") {
		return deflate;
	}

	if (cur_mode == "-unpack" || cur_mode == "-u" || cur_mode == "-unp" || cur_mode == "unpack") {
		return unpack;
	}

	if (cur_mode == "-pack" || cur_mode == "-pa" || cur_mode == "pack") {
		return pack;
	}

	if (cur_mode == "-parse" || cur_mode == "-p" || cur_mode == "parse") {
		return parse;
	}

	if (cur_mode == "-build" || cur_mode == "-b" || cur_mode == "build") {

		bool dont_pack = false;

		while ((int)args.size() > arg_base) {
			string arg2(args[arg_base]);
		transform(arg2.begin(), arg2.end(), arg2.begin(),
				[](unsigned char c) { return static_cast<char>(tolower(c)); });
			if (arg2 == "-n" || arg2 == "-nopack") {
				arg_base++;
				dont_pack = true;
			} else {
				break;
			}
		}
		return dont_pack ? build_nopack : build;
	}

	allow_listfile = false;
	if (cur_mode == "-bat") {
		return bat;
	}

	if (cur_mode == "-example" || cur_mode == "-e") {
		return example;
	}

	if (cur_mode == "-list" || cur_mode == "-l") {
		return process_list;
	}

	if (cur_mode == "-listfiles" || cur_mode == "-lf") {
		return list_files;
	}
	if (cur_mode == "check" || cur_mode == "-check") return check;
	if (cur_mode == "info" || cur_mode == "-info") return info;
	if (cur_mode == "manifest" || cur_mode == "-manifest") return manifest;
	if (cur_mode == "--help" || cur_mode == "-h" || cur_mode == "help") return usage;

	return nullptr;
}

void read_param_file(const char *filename, vector< vector<string> > &list)
{
	ifstream in(filename);
	string line;
	while (getline(in, line)) {

		vector<string> current_line;

		stringstream ss;
		ss.str(line);

		string item;
		while (getline(ss, item, ';')) {
			current_line.push_back(item);
		}

		while (current_line.size() < 5) {
			// Дополним пустыми строками, чтобы избежать лишних проверок
			current_line.emplace_back("");
		}

		list.push_back(current_line);
	}
}

int main(int argc, char* argv[])
{
	int arg_base = 1;
	bool allow_listfile = false;
	vector<string> args;
	args.emplace_back(argv[0]);
	for (int i = 1; i < argc; i++) {
		string argument(argv[i]);
		if (argument == "--quiet") option_quiet = true;
		else if (argument == "--verbose") option_verbose = true;
		else if (argument == "--json") option_json = true;
		else if (argument == "--force") option_force = true;
		else args.push_back(std::move(argument));
	}
	handler_t handler = get_run_mode(args, arg_base, allow_listfile);

	vector<string> cli_args;

	if (handler == nullptr) {
		usage(cli_args);
		return 1;
	}

	if (allow_listfile && arg_base < static_cast<int>(args.size())) {
		string a_list(args[arg_base]);
		transform(a_list.begin(), a_list.end(), a_list.begin(),
				[](unsigned char c) { return static_cast<char>(tolower(c)); });
		if (a_list == "-list" || a_list == "-l") {
			if (arg_base + 1 >= static_cast<int>(args.size())) {
				cerr << "List file argument is missing" << endl;
				return 1;
			}
			// Передан файл с параметрами
			vector< vector<string> > param_list;
			read_param_file(args[arg_base + 1].c_str(), param_list);

			int ret = 0;

			for (auto argv_from_file : param_list) {
				int ret1 = handler(argv_from_file);
				if (ret1 != 0 && ret == 0) {
					ret = ret1;
				}
			}

			return ret;
		}
	}

	for (size_t i = static_cast<size_t>(arg_base); i < args.size(); i++) {
		cli_args.emplace_back(args[i]);
	}
	while (cli_args.size() < 3) {
		// Дополним пустыми строками, чтобы избежать лишних проверок
		cli_args.emplace_back("");
	}

	if (option_verbose && args.size() > 1) cerr << "Running " << args[1] << endl;
	null_buffer_t null_buffer;
	auto *original = cout.rdbuf();
	const bool handler_owns_output = handler == check || handler == info || handler == manifest
		|| handler == version || handler == usage;
	if ((option_quiet || option_json) && !handler_owns_output) cout.rdbuf(&null_buffer);
	int ret = handler(cli_args);
	cout.rdbuf(original);
	if (option_json && !handler_owns_output) {
		cout << "{\"command\":\"" << json_escape(args[1]) << "\",\"success\":"
			 << (ret == 0 ? "true" : "false") << ",\"exitCode\":" << ret << "}" << endl;
	}
	if (ret == V8UNPACK_SHOW_USAGE) {
		usage(cli_args);
	}
	return ret;
}
