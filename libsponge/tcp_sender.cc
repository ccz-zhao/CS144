#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _current_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _outbound_size; }

void TCPSender::fill_window() {
    size_t curr_window_size = (_last_window_size == 0) ? 1 : _last_window_size;
    while (curr_window_size > _outbound_size) {
        TCPSegment seg;
        // seqno
        seg.header().seqno = next_seqno();

        // syn
        if (!_syn_ready) {
            seg.header().syn = true;
            _syn_ready = true;
        }

        // payload
        size_t payload_size =
            std::min(TCPConfig::MAX_PAYLOAD_SIZE, curr_window_size - _outbound_size - seg.header().syn);
        std::string payload = stream_in().read(payload_size);
        seg.payload() = Buffer(std::move(payload));

        // fin
        if (!_fin_ready && stream_in().eof() && curr_window_size > _outbound_size + seg.length_in_sequence_space()) {
            seg.header().fin = true;
            _fin_ready = true;
        }

        const auto seg_len = seg.length_in_sequence_space();
        if (seg_len == 0) {
            break;
        }
        _outbound_size += seg_len;
        _next_seqno += seg_len;

        _segments_out.push(seg);
        _outbound_seg.push(seg);

        if (seg.header().fin) {
            break;
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    auto abs_ackno = unwrap(ackno, _isn, _next_seqno);
    if (abs_ackno > _next_seqno) {
        return;
    }
    auto last_sz = _outbound_seg.size();
    while (!_outbound_seg.empty()) {
        auto &seg = _outbound_seg.front();
        auto seg_len = seg.length_in_sequence_space();
        auto seqno = unwrap(seg.header().seqno, _isn, _next_seqno);
        if (seqno + seg_len > abs_ackno) {
            break;
        }
        _outbound_seg.pop();
        _outbound_size -= seg_len;
    }
    if (last_sz != _outbound_seg.size()) {
        _ms_since_last_tick = 0;
        _current_retransmission_timeout = _initial_retransmission_timeout;
    }
    _consecutive_retransmit_nums = 0;
    _last_window_size = window_size;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _ms_since_last_tick += ms_since_last_tick;
    if (!_outbound_seg.empty() && _ms_since_last_tick >= _current_retransmission_timeout) {
        auto &seg = _outbound_seg.front();
        _segments_out.push(seg);
        if (_last_window_size > 0) {
            ++_consecutive_retransmit_nums;
            _current_retransmission_timeout *= 2;
        }
        _ms_since_last_tick = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmit_nums; }

void TCPSender::send_empty_segment() {}
