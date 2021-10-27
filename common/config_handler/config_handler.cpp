#pragma warning(disable : 4996)

#include "config_handler.h"

namespace common::config_handler {
	const std::size_t BUF_SIZE = 501;

	ConfigFileHandler* ConfigFileHandler::m_instance = nullptr;

	std::unordered_map<std::string, std::string> ConfigFileHandler::config_items;

	ConfigFileHandler::~ConfigFileHandler() {
		config_items.clear();
	}

	// check a POSIX error code
	static void posixCheck(int errorCode) {
		if (errorCode != 0) {
			throw std::system_error(std::error_code(errorCode, std::generic_category()));
		}
	}

	// check the status code of functions that return an error code in `errno`.
	static void posixAssert(bool success) {
		if (!success) {
			posixCheck(errno);
		}
	}

	// Truncate the leading and trailing spaces of the string
	void Trim(std::string& str) {
		if (!str.empty())
		{
			str.erase(0, str.find_first_not_of(" "));
			str.erase(str.find_last_not_of(" ") + 1);
		}
		return;
	}

	ConfigFileHandler::FileHandle ConfigFileHandler::FileReader::openFile(const std::filesystem::path& config_file) {
		std::FILE* file = fopen(config_file.string().c_str(), "r");
		posixAssert(file != nullptr);
		return FileHandle(file, FileDeleter());
	}

	void ConfigFileHandler::FileReader::closeFile(FileHandle& file) {
		std::FILE* rawfile = file.release();

		// Close the file, throw an exception on error.
		int result = fclose(rawfile);
		posixAssert(result == 0);
	}

	bool ConfigFileHandler::FileReader::readFile(char* linebuf) {

		while (!std::feof(file_.get())) {
			if (fgets(linebuf, 500, file_.get()) == nullptr) {
				continue;
			}
			else if (linebuf[0] == 0) {
				continue;
			}
			else if (linebuf[0] == ';' || linebuf[0] == ' ' || linebuf[0] == '#' || linebuf[0] == '\t' || linebuf[0] == '\n') { // ע����
				continue;
			}

		lblprocstring:
			if (strlen(linebuf) > 0)
			{
				int length = strlen(linebuf) - 1;
				if (linebuf[length] == 10 || linebuf[length] == 13 || linebuf[length] == 32)
				{
					linebuf[length] = 0;
					goto lblprocstring;
				}
			}
			if (linebuf[0] == 0) {
				continue;
			}
			if (linebuf[0] == '[') {
				continue;
			}
			std::string line = linebuf;
			auto pos = line.find('=');
			if (pos < line.size() - 1 && pos != std::string::npos) {
				std::string config_name(pos, '\0');
				std::copy(line.begin(), line.begin() + pos, config_name.begin());

				std::string config_content(line.size() - pos - 1, '\0');
				std::copy(line.begin() + pos + 1, line.end(), config_content.begin());

				Trim(config_name);
				Trim(config_content);

				ConfigFileHandler::config_items[config_name] = config_content;
			}
			line.clear();
		}
		return true;
	}

	bool ConfigFileHandler::Load(const std::filesystem::path& config_file) {
		// open file
		auto file_reader = FileReader::open(config_file.string().c_str());
		char linebuf[BUF_SIZE];

		bool result = file_reader->readFile(linebuf);
		posixAssert(result);
        
		return true;
	}

	const std::string ConfigFileHandler::GetValue(const std::string& config_name) {
		auto config_pos = config_items.find(config_name);
		if (config_pos != config_items.end()) {
			return config_items[config_name];
		}
		return std::string();
	}

	const int ConfigFileHandler::GetValue(const std::string& config_name, const int default_value) {
		auto config_pos = config_items.find(config_name);
		if (config_pos != config_items.end()) {
			return std::stoi(config_items[config_name]);
		}
		return default_value;
	}

	const double ConfigFileHandler::GetValue(const std::string& config_name, const double default_value) {
		auto config_pos = config_items.find(config_name);
		if (config_pos != config_items.end()) {
			return std::stod(config_items[config_name]);
		}
		return default_value;
	}

	std::vector<double> ConfigFileHandler::GetVector(const std::string& config_name) {
		auto config_pos = config_items.find(config_name);
		if (config_pos != config_items.end() && !(*config_pos).second.empty()) {
            std::vector<double> result;
			auto& list = (*config_pos).second;
			auto iter = list.begin();
			std::string candidate;
			while (iter != list.end()) {
				if (*iter == '[' || *iter == ']' || *iter == ',' || *iter == ' ') {
					if (iter != list.end()) {
						++iter;
					}
					continue;
				}
				while (*iter != ',' && *iter != ']') {
					candidate.push_back(*iter);
					++iter;
				}
				result.push_back(std::stod(std::move(candidate)));
				candidate.clear();
			}
            return result;
		}
		return {};
	}
}

