#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _bitmap(capacity, false), _buffer(capacity, '\0') {}

void StreamReassembler::write_bytes() {
    std::string data = "";
    while (_bitmap.front()) {
        data += _buffer.front();
        _bitmap.pop_front();
        _buffer.pop_front();
        _bitmap.push_back(false);
        _buffer.push_back('\0');
    }
    if (data.size() > 0) {
        _output.write(data);
        _next_idx += data.size();
        _unassem_bytes -= data.size();
    }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    auto read_bytes = _output.bytes_read();
    if (index + data.size() < _next_idx || index >= read_bytes + _capacity) {
        return;
    }

    if (eof && index + data.size() <= read_bytes + _capacity) {
        _eof = true;
    }

    size_t start_idx = std::max(index, _next_idx);
    size_t end_idx = std::min(index + data.size(), read_bytes + _capacity);
    for (size_t i = start_idx; i < end_idx; ++i) {
        if (!_bitmap[i - _next_idx]) {
            _buffer[i - _next_idx] = data[i - index];
            _bitmap[i - _next_idx] = true;
            ++_unassem_bytes;
        }
    }

    write_bytes();
    if (_eof && _unassem_bytes == 0) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassem_bytes; }

bool StreamReassembler::empty() const { return _unassem_bytes == 0; }
