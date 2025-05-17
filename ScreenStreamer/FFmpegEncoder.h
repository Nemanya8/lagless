#pragma once

#include <d3d11.h>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

class FFmpegEncoder {
public:
    FFmpegEncoder();
    ~FFmpegEncoder();

    bool Initialize(ID3D11Device* device, int width, int height, int bitrate);
    bool EncodeFrame(ID3D11Texture2D* frame, std::vector<uint8_t>& encoded);
    void Shutdown();

private:
    ID3D11Device* m_device;
    int m_width;
    int m_height;
    int m_bitrate;
    bool m_initialized;

    AVCodecContext* m_codecContext;
    AVFrame* m_frame;
    AVPacket* m_packet;
    SwsContext* m_swsContext;

    bool InitCodec(int width, int height, int bitrate);
    bool InitFrame(int width, int height);
    void FreeResources();
}; 