#pragma warning(disable : 4996)
#include <cassert>
#include <stdexcept>  // for runtime_error
#include <string>

#include "file_io.h"

namespace common::file_io {

    namespace detail {

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

        static std::string getFileAccessMode(FileAccessMode mode) {
            switch (mode) {
            case FileAccessMode::read:
                return "r";
            case FileAccessMode::write:
                return "w";
            }
            std::terminate();
        }

        static std::string getFileMode(FileMode contentMode) {
            switch (contentMode)
            {
            case FileMode::binary: return "b";
            case FileMode::text: return "";
            }
            std::terminate();
        }

        static std::string getModeString(FileAccessMode accessMode, FileMode contentMode)
        {
            return getFileAccessMode(accessMode) + getFileMode(contentMode);
        }

        FileHandle openFile(const std::filesystem::path& path, FileAccessMode accessMode, FileMode fileMode) {
            std::string modeString = getModeString(accessMode, fileMode);
            std::FILE* file = std::fopen(path.string().c_str(), modeString.c_str());
            posixAssert(file != nullptr);
            return FileHandle(file, FileDeleter());
        }

        void closeFile(FileHandle file) {
            // Make the file handle abandon ownership of the underlying pointer.
            std::FILE* rawfile = file.release();

            // Close the file, throw an exception on error.
            int result = fclose(rawfile);
            detail::posixAssert(result == 0);
        }

        bool readFile(FileReadResult& result, FileHandle& file, std::span<char>& buffer) {
            std::size_t numOfElementRead = std::fread(buffer.data(), sizeof(std::byte), buffer.size(), file.get());

            result.numBytesRead = static_cast<std::ptrdiff_t>(numOfElementRead);
            result.endOfFile = std::feof(file.get()) != 0;

            return std::ferror(file.get()) != 0;
        }

        bool writeFile(FileHandle& file, std::span<char>& buffer) {
            std::size_t numElementsWritten = std::fwrite(buffer.data(), sizeof(std::byte), buffer.size(), file.get());
            return std::ferror(file.get()) == 0 && numElementsWritten == buffer.size();
        }
    }
} // namespace arithmetic_expression_::file_reader