#pragma once
#include <span>
#include <string>
#include <vector>
#include <cstdio>      // for FILE etc.
#include <memory>      // for unique_ptr<>
#include <cstddef>     // for byte
#include <utility>     // for move()
#include <filesystem>  // for path

namespace common::file_io {
    // General helper functions and types
    enum class FileMode { binary, text };

    struct FileReadResult
    {
        std::ptrdiff_t numBytesRead;
        bool endOfFile;
    };

    namespace detail {

        enum class FileAccessMode { read, write };

        struct FileDeleter
        {
            void operator ()(std::FILE* file) const noexcept
            {
                // This function is called by the destructor of `std::unique_ptr<>`, so we ignore any errors returned by
                // `std::fclose()`.
                std::fclose(file);
            }
        };

        // Define a "file handle" as a `unique_ptr<>` with a custom deleter: lifetime management is done as usual, but when
        // the resource is to be freed, `unique_ptr<>` will pass the file pointer to an instaance of our deleter rather than
        // using a `delete` expression.
        using FileHandle = std::unique_ptr<std::FILE, FileDeleter>;

        FileHandle openFile(const std::filesystem::path& path, FileAccessMode accessMode, FileMode fileMode);

        void closeFile(FileHandle file);

        bool readFile(FileReadResult& result, FileHandle& file, std::span<char>& buffer);

        bool writeFile(FileHandle& file, std::span<char>& buffer);
    } // namespace detail

    class FileReader {
    private:
        detail::FileHandle file_;

    public:
        FileReader() {
            file_.reset();
        }

        std::FILE* GetFilePointer() {
            return file_.get();
        }

        void open(const std::filesystem::path &path, FileMode mode) {
            file_ = std::move(detail::openFile(path, detail::FileAccessMode::read, mode));
            return ;
        }

        [[nodiscard]] FileReadResult readTo(std::span<char>& buffer) {
            FileReadResult result{};

            if (detail::readFile(result, file_, buffer) != 0) {
                throw std::runtime_error("failed to read from file");
            }

            return result;
        }

        void close() {
            detail::closeFile(std::move(file_));
        }
    };

    class FileWriter {
    private:
        detail::FileHandle file_;

    public:
        FileWriter() {
            file_.reset();
        }

        std::FILE* GetFilePointer() {
            return file_.get();
        }

        void open(const std::filesystem::path &path, FileMode mode) {
            file_ = std::move(detail::openFile(path, detail::FileAccessMode::write, mode));
            return ;
        }

        void write(std::span<char>& buffer) {
            if (!detail::writeFile(file_, buffer)) {
                throw std::runtime_error("failed to write to file");
            }
        }

        void close() {
            detail::closeFile(std::move(file_));
        }
    };

    std::vector<std::string> SplitPath(std::string& filename);
} // namespace common::file_io