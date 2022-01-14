#include "lines.h"

using common::file_io::FileReader;

namespace common::file_io::read_lines {

    constexpr std::size_t bufSize = 1024;
    constexpr std::size_t bufSize_long = 4096;

    LineRange::LineRange(FileReader&& _fileReader)
        : fileReader_(std::move(_fileReader)), readBuf_(bufSize, '\0') {}

    bool LineRange::tryReadNextLine()
    {
        while (!(pos_ == end_ && endOfFile_))
        {
            auto posIt = readBuf_.begin() + pos_;
            auto endIt = readBuf_.begin() + end_;
            auto newlinePos = pos_ + (std::find(posIt, endIt, '\n') - posIt);
            if (newlinePos == end_ && !endOfFile_)
            {
                // No newline found in buffer, but not at the end of the file yet. We need to read more data to get the rest of
                // the current line.
                if (lineStart_ != 0)
                {
                    // If the line fragment starts somewhere in the middle of the buffer, copy it to the begin of the buffer,
                    // then read more data into the remaining buffer.
                    //
                    // Source and destination range may overlap, but `std::copy()` can handle that as long as we copy forward.
                    auto fragmentLength = end_ - lineStart_;
                    auto lineStartIt = readBuf_.begin() + lineStart_;
                    std::copy(lineStartIt, endIt, readBuf_.begin());
                    lineStart_ = 0;
                    pos_ = fragmentLength;

                    // If a previous line was longer than `bufSize`, forcing the buffer to be extended, and if the line fragment
                    // is smaller than `bufSize`, take the opportunity to reduce the buffer to its normal size. (Using a smaller
                    // buffer avoids cache thrashing.)
                    if (readBuf_.size() > bufSize && fragmentLength <= bufSize)
                    {
                        readBuf_.resize(bufSize);
                    }
                }
                else if (end_ == std::ssize(readBuf_))
                {
                    // If the entire buffer is used and no newline is found, the line must be longer than the buffer. Grow the
                    // buffer and read into the additional space at the end.
                    readBuf_.resize(readBuf_.size() + bufSize);
                    pos_ = end_;
                }

                auto readSpan = std::span(readBuf_.begin() + pos_, readBuf_.end());
                auto readResult = fileReader_.readTo(readSpan);
                end_ = pos_ + readResult.numBytesRead;
                endOfFile_ = readResult.endOfFile || readResult.numBytesRead < std::ssize(readSpan);
            }
            else
            {
                auto lineEnd = newlinePos;
                auto lineView = std::string_view(readBuf_.begin() + lineStart_, readBuf_.begin() + lineEnd);

                // Skip newline character, but only if we are not at the end of the buffer, in which case the file ended without
                // a trailing newline character.
                if (lineEnd != end_)
                {
                    pos_ = lineEnd + 1;
                    lineStart_ = pos_;
                }

                // Store the line and indicate sucess.
                nextLine_ = lineView;
                return true;
            }
        }
        return false;
    }

    LineRangeNormal::LineRangeNormal(FileReaderNormal&& _fileReader)
            : fileReader_(std::move(_fileReader)), readBuf_(bufSize_long, '\0') {}

    bool LineRangeNormal::tryReadNextLine()
    {
        while (!(pos_ == end_ && endOfFile_))
        {
            auto posIt = readBuf_.begin() + pos_;
            auto endIt = readBuf_.begin() + end_;
            auto newlinePos = pos_ + (std::find(posIt, endIt, '\n') - posIt);
            if (newlinePos == end_ && !endOfFile_)
            {
                // No newline found in buffer, but not at the end of the file yet. We need to read more data to get the rest of
                // the current line.
                if (lineStart_ != 0)
                {
                    // If the line fragment starts somewhere in the middle of the buffer, copy it to the begin of the buffer,
                    // then read more data into the remaining buffer.
                    //
                    // Source and destination range may overlap, but `std::copy()` can handle that as long as we copy forward.
                    auto fragmentLength = end_ - lineStart_;
                    auto lineStartIt = readBuf_.begin() + lineStart_;
                    std::copy(lineStartIt, endIt, readBuf_.begin());
                    lineStart_ = 0;
                    pos_ = fragmentLength;

                    // If a previous line was longer than `bufSize`, forcing the buffer to be extended, and if the line fragment
                    // is smaller than `bufSize`, take the opportunity to reduce the buffer to its normal size. (Using a smaller
                    // buffer avoids cache thrashing.)
                    if (readBuf_.size() > bufSize && fragmentLength <= bufSize)
                    {
                        readBuf_.resize(bufSize);
                    }
                }
                else if (end_ == std::ssize(readBuf_))
                {
                    // If the entire buffer is used and no newline is found, the line must be longer than the buffer. Grow the
                    // buffer and read into the additional space at the end.
                    readBuf_.resize(readBuf_.size() + bufSize);
                    pos_ = end_;
                }

                auto readSpan = std::span(readBuf_.begin() + pos_, readBuf_.end());
                auto readResult = fileReader_.readTo(readSpan);
                end_ = pos_ + readResult.numBytesRead;
                endOfFile_ = readResult.endOfFile || readResult.numBytesRead < std::ssize(readSpan);
            }
            else
            {
                auto lineEnd = newlinePos;
                auto lineView = std::string_view(readBuf_.begin() + lineStart_, readBuf_.begin() + lineEnd);

                // Skip newline character, but only if we are not at the end of the buffer, in which case the file ended without
                // a trailing newline character.
                if (lineEnd != end_)
                {
                    pos_ = lineEnd + 1;
                    lineStart_ = pos_;
                }

                // Store the line and indicate sucess.
                nextLine_ = lineView;
                return true;
            }
        }
        return false;
    }
} // namespace arithmetic_expression_::read_lines