#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    if (_input_end) {
        return 0;
    }
    size_t sz = std::min(data.size(), _capacity - _deque.size());
    for (size_t i = 0; i < sz; ++i) {
        _deque.push_back(data[i]);
    }
    _write_bytes += sz;
    return sz;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const { return string(_deque.begin(), _deque.begin() + len); }

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    for (size_t i = 0; i < len; ++i) {
        _deque.pop_front();
    }
    _read_bytes += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t sz = std::min(len, _deque.size());
    auto res = peek_output(sz);
    pop_output(sz);
    return res;
}

void ByteStream::end_input() { _input_end = true; }

bool ByteStream::input_ended() const { return _input_end; }

size_t ByteStream::buffer_size() const { return _deque.size(); }

bool ByteStream::buffer_empty() const { return _deque.empty(); }

bool ByteStream::eof() const { return _input_end && _deque.empty(); }

size_t ByteStream::bytes_written() const { return _write_bytes; }

size_t ByteStream::bytes_read() const { return _read_bytes; }

size_t ByteStream::remaining_capacity() const { return _capacity - _deque.size(); }
