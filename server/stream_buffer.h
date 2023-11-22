// 작성자: 윤정도
//
#pragma once

#include <array>
#include <exception>

#include "stream_buffer_abstract.h"

template <int buffer_size>
class stream_buffer : public stream_buffer_abstract 
{
    using t_buffer = stream_buffer<buffer_size>;
public:
    stream_buffer() = default;
    ~stream_buffer() override {}
    stream_buffer(const t_buffer&) = delete;

    std::string read_string(int len) override {
        if (!readable(len)) {
            return {};
        }

        std::string ret(len, '\0');
        for (int i = _read_pos, j = 0; j < len; ++i, ++j) {
            ret[j] = _buf[i];
        }

        _read_pos += len; 
        return ret;
    }

    int read_int() override {
        if (!readable(sizeof(int)))
            return -1;

        int ret;
        std::memcpy(&ret, _buf.data() + _read_pos, sizeof(int));
        _read_pos += sizeof(int);
        return ret;
    }

    void write_int(int value) override {
        if (!writeable(sizeof(int))) {
            throw std::exception();
        }

        std::memcpy(_buf.data() + _write_pos, &value, sizeof(int));
        _write_pos += sizeof(int);
    }

    void write_string(const std::string& str) override {
        int len = str.length();
        if (!writeable(len)) {
            throw std::exception();
        }


        std::memcpy(_buf.data() + _write_pos, str.data(), len);
        _write_pos += len;
    }

    void write_bytes(char* data, int len) override {
        if (!writeable(len)) {
            throw std::exception();
        }

        std::memcpy(_buf.data() + _write_pos, data, len);
        _write_pos += len;
    }

    bool readable(int len) override {
        return _write_pos - _read_pos  >= len;
    }

    bool writeable(int len) override {
        return buffer_size - _write_pos >= len;
    }
    
    int peek_int() override {
        if (!readable(sizeof(int)))
            return -1;

        int ret;
        std::memcpy(&ret, _buf.data(), sizeof(int));
        return ret;
    }

    // read_pos 만큼 앞으로 당김
    void pop_reads() override {
        if (_read_pos <= 0)
            return;

        std::memcpy(_buf.data(), _buf.data() + _read_pos, _write_pos - _read_pos);
        _write_pos -= _read_pos;
        _read_pos = 0;
    }

    
    char* readable_data() override { return _buf.data() + _read_pos; }
    char* writeable_data() override { return _buf.data() + _write_pos; }
    int readable_size() const override { return _write_pos - _read_pos; }
    int writeable_size() const override { return buffer_size - _write_pos; }
    int get_read_pos() const override { return _read_pos; }
    int get_write_pos() const override { return _write_pos; }
private:
    std::array<char, buffer_size> _buf;
};
