#pragma once
#include <span>
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

    // Singleton
    class FileReader {
    private:
        detail::FileHandle file_;

        FileReader(const std::filesystem::path& path, FileMode& mode)
            : file_(std::move(detail::openFile(path, detail::FileAccessMode::read, mode))) {}

    public:
        static FileReader* open(const std::filesystem::path& path, FileMode&& mode) {
            static FileReader instance(path, mode);
            return &instance;
        }

        [[nodiscard]] FileReadResult readTo(std::span<char>& buffer) {
            FileReadResult result;

            if (detail::readFile(result, file_, buffer) != 0) {
                throw std::runtime_error("failed to read from file");
            }

            return result;
        }

        void close() {
            detail::closeFile(std::move(file_));
        }
    };

    // Singleton
    class FileWriter {
    private:
        detail::FileHandle file_;

        FileWriter(const std::filesystem::path& path, FileMode& mode)
            : file_(std::move(detail::openFile(path, detail::FileAccessMode::read, mode))) {}

    public:
        static FileWriter* open(const std::filesystem::path& path, FileMode&& mode) {
            static FileWriter instance(path, mode);
            return &instance;
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
} // namespace common::file_io