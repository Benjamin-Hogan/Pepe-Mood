#ifndef _MJPEGCLASS_H_
#define _MJPEGCLASS_H_

#define READ_BUFFER_SIZE 1024
#define MAXOUTPUTSIZE (MAX_BUFFERED_PIXELS / 16 / 16)

#include <Arduino.h>
#include <SD.h>
#include <JPEGDEC.h>

class MjpegClass
{
public:
    bool setup(Stream *input, uint8_t *mjpeg_buf, JPEG_DRAW_CALLBACK *pfnDraw, bool useBigEndian)
    {
        _input = input;
        _mjpeg_buf = mjpeg_buf;
        _pfnDraw = pfnDraw;
        _useBigEndian = useBigEndian;
        _inputindex = 0;

        if (!_read_buf)
        {
            _read_buf = (uint8_t *)malloc(READ_BUFFER_SIZE);
        }

        return (_read_buf != NULL);
    }

    bool readMjpegBuf()
    {
        if (_inputindex == 0)
        {
            _buf_read = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
            _inputindex += _buf_read;
        }
        _mjpeg_buf_offset = 0;
        int i = 0;
        bool found_FFD8 = false;
        while ((_buf_read > 0) && (!found_FFD8))
        {
            i = 0;
            while ((i < _buf_read) && (!found_FFD8))
            {
                if ((_read_buf[i] == 0xFF) && (_read_buf[i + 1] == 0xD8)) // JPEG header
                {
                    found_FFD8 = true;
                }
                ++i;
            }
            if (found_FFD8)
            {
                --i;
            }
            else
            {
                _buf_read = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
            }
        }
        uint8_t *_p = _read_buf + i;
        _buf_read -= i;
        bool found_FFD9 = false;
        if (_buf_read > 0)
        {
            i = 3;
            while ((_buf_read > 0) && (!found_FFD9))
            {
                if ((_mjpeg_buf_offset > 0) && (_mjpeg_buf[_mjpeg_buf_offset - 1] == 0xFF) && (_p[0] == 0xD9)) // JPEG trailer
                {
                    found_FFD9 = true;
                }
                else
                {
                    while ((i < _buf_read) && (!found_FFD9))
                    {
                        if ((_p[i] == 0xFF) && (_p[i + 1] == 0xD9)) // JPEG trailer
                        {
                            found_FFD9 = true;
                            ++i;
                        }
                        ++i;
                    }
                }

                memcpy(_mjpeg_buf + _mjpeg_buf_offset, _p, i);
                _mjpeg_buf_offset += i;
                size_t o = _buf_read - i;
                if (o > 0)
                {
                    memcpy(_read_buf, _p + i, o);
                    _buf_read = _input->readBytes(_read_buf + o, READ_BUFFER_SIZE - o);
                    _p = _read_buf;
                    _inputindex += _buf_read;
                    _buf_read += o;
                }
                else
                {
                    _buf_read = _input->readBytes(_read_buf, READ_BUFFER_SIZE);
                    _p = _read_buf;
                    _inputindex += _buf_read;
                }
                i = 0;
            }
            if (found_FFD9)
            {
                return true;
            }
        }

        return false;
    }

    bool drawJpg()
    {
        _remain = _mjpeg_buf_offset;
        if (_useBigEndian)
        {
            _jpeg.setPixelType(RGB565_BIG_ENDIAN);
        }
        _jpeg.openRAM(_mjpeg_buf, _remain, _pfnDraw);
        _jpeg.decode(0, 0, 0);
        _jpeg.close();
        return true;
    }

private:
    Stream *_input;
    uint8_t *_mjpeg_buf;
    JPEG_DRAW_CALLBACK *_pfnDraw;
    bool _useBigEndian;
    uint8_t *_read_buf = NULL;
    int32_t _mjpeg_buf_offset = 0;
    JPEGDEC _jpeg;
    int32_t _inputindex = 0;
    int32_t _buf_read;
    int32_t _remain = 0;
};

#endif // _MJPEGCLASS_H_
