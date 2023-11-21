// 작성자: 윤정도

#pragma once

#include <string>
#include <cstdlib>
#include <cstring>

class stream_buffer_abstract 
{
public:
    stream_buffer_abstract()
        : _read_pos(0)
        , _write_pos(0)
    {}

    virtual ~stream_buffer_abstract() = default;
    virtual std::string read_string(int len) = 0;
    virtual int read_int() = 0;
    virtual void write_int(int value) = 0;
    virtual void write_string(const std::string& str) = 0;
    virtual void write_bytes(char* data, int len) = 0;
    virtual bool readable(int len) = 0;
    virtual bool writeable(int len) = 0;
    virtual int peek_int() = 0;
    virtual void pop_reads() = 0;

    void reset_pos() {
        _read_pos = 0;
        _write_pos = 0;
    }

    void move_read_pos(int len) {
        _read_pos += len;
    }

    void move_write_pos(int len) {
        _write_pos += len;
    }

    void set_read_pos(int pos) { _read_pos = pos; }

    virtual char* readable_data() = 0; 
    virtual char* writeable_data() = 0; 
    virtual int readable_size() const = 0; 
    virtual int writeable_size() const = 0; 
    virtual int get_read_pos() const = 0; 
    virtual int get_write_pos() const = 0; 
protected:
    int _read_pos;
    int _write_pos;
};
