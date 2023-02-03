#include "tcp_receiver.hh"

#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const auto &seg_head = seg.header();

    // LISTEN
    if (!_ackno.has_value()) {
        if (!seg_head.syn) {
            return;
        }
        _isn = seg_head.seqno;
        _ackno = _isn + 1;
    }

    // SEG_RECV
    const auto &payload_str = seg.payload().copy();
    auto abs_seqno = stream_out().bytes_written() + 1;  // syn

    auto index = unwrap(seg_head.seqno, _isn, abs_seqno) + (seg_head.syn ? 1 : 0) - 1;
    _reassembler.push_substring(payload_str, index, seg_head.fin);

    _ackno = _ackno.value() + (stream_out().bytes_written() + 1 - abs_seqno);

    // FIN_RECV
    if (stream_out().input_ended()) {
        _ackno = _ackno.value() + 1;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const { return _ackno; }

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
