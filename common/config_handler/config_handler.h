#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>

namespace common::config_handler {
	class ConfigFileHandler {
	private:
		ConfigFileHandler() {}

		static ConfigFileHandler* m_instance;

	public:
		~ConfigFileHandler();

	public:
		static ConfigFileHandler* GetInstance() {
			if (m_instance == nullptr) {
				//锁（本来该加锁，但在主线程中率先执行了GetInstance函数，这样就不存在多线程调用该函数导致的需要互斥的问题，因此这里就没有实际加锁）
				if (m_instance == nullptr) {
					m_instance = new ConfigFileHandler();
				}
				//放锁
			}
			return m_instance;
		}

	public:

		struct FileDeleter {
			void operator ()(std::FILE* file) const noexcept {
				// This function is called by the destructor of `std::unique_ptr<>`, so we ignore any errors returned by
				// `std::fclose()`.
				std::fclose(file);
			}
		};

		using FileHandle = std::unique_ptr<std::FILE, FileDeleter>;

		class FileReader {
		private:
			// Define a "file handle" as a `unique_ptr<>` with a custom deleter: lifetime management is done as usual, but when
			// the resource is to be freed, `unique_ptr<>` will pass the file pointer to an instance of our deleter rather than
			// using a `delete` expression.
			FileHandle file_;

		private:
			FileHandle openFile(const std::filesystem::path& config_file);

			void closeFile(FileHandle& file);

			FileReader(const std::filesystem::path& config_file)
				: file_(std::move(openFile(config_file))) {}

		public:
			static FileReader* open(const std::filesystem::path& config_file) {
				static FileReader instance(config_file);
				return &instance;
			}

			bool readFile(char* buffer);

			~FileReader() {
				closeFile(file_);
			}
		};

	public:
		bool Load(const std::filesystem::path& config_file); // load config file
		
		const std::string GetValue(const std::string& config_name);
		
		const int GetValue(const std::string& config_name, const int default_value);

		const double GetValue(const std::string& config_name, const double default_value);

		std::vector<double>& GetValue(const std::string& config_name, std::vector<double>& value);

	public:
		static std::unordered_map<std::string, std::string> config_items;
	};
}
