#include "FFmpegEncoder.h"
#include <iostream>

FFmpegEncoder::FFmpegEncoder()
    : m_device(nullptr)
    , m_width(0)
    , m_height(0)
    , m_bitrate(0)
    , m_initialized(false)
    , m_codecContext(nullptr)
    , m_frame(nullptr)
    , m_packet(nullptr)
    , m_swsContext(nullptr)
{
}

FFmpegEncoder::~FFmpegEncoder() {
    Shutdown();
}

bool FFmpegEncoder::Initialize(ID3D11Device* device, int width, int height, int bitrate) {
    if (m_initialized) return true;

    m_device = device;
    m_width = width;
    m_height = height;
    m_bitrate = bitrate;

    // Initialize FFmpeg
    avformat_network_init();

    // Find H.264 encoder
    const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!codec) {
        std::cerr << "Could not find H.264 encoder" << std::endl;
        return false;
    }

    // Create codec context
    m_codecContext = avcodec_alloc_context3(codec);
    if (!m_codecContext) {
        std::cerr << "Could not allocate codec context" << std::endl;
        return false;
    }

    // Set codec parameters
    m_codecContext->width = width;
    m_codecContext->height = height;
    m_codecContext->time_base = { 1, 60 };  // 60 FPS
    m_codecContext->framerate = { 60, 1 };
    m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    m_codecContext->bit_rate = bitrate;
    m_codecContext->gop_size = 10;
    m_codecContext->max_b_frames = 1;

    // Open codec
    if (avcodec_open2(m_codecContext, codec, nullptr) < 0) {
        std::cerr << "Could not open codec" << std::endl;
        return false;
    }

    // Allocate frame
    m_frame = av_frame_alloc();
    if (!m_frame) {
        std::cerr << "Could not allocate frame" << std::endl;
        return false;
    }

    m_frame->format = m_codecContext->pix_fmt;
    m_frame->width = width;
    m_frame->height = height;

    if (av_frame_get_buffer(m_frame, 0) < 0) {
        std::cerr << "Could not allocate frame buffer" << std::endl;
        return false;
    }

    // Allocate packet
    m_packet = av_packet_alloc();
    if (!m_packet) {
        std::cerr << "Could not allocate packet" << std::endl;
        return false;
    }

    m_initialized = true;
    return true;
}

bool FFmpegEncoder::EncodeFrame(ID3D11Texture2D* frame, std::vector<uint8_t>& encoded) {
    if (!m_initialized || !frame) return false;

    // Get device context
    ID3D11DeviceContext* context = nullptr;
    m_device->GetImmediateContext(&context);
    if (!context) {
        std::cerr << "Failed to get device context" << std::endl;
        return false;
    }

    // Map the texture
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = context->Map(frame, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        std::cerr << "Failed to map texture" << std::endl;
        return false;
    }

    // Convert RGBA to YUV420P
    if (!m_swsContext) {
        m_swsContext = sws_getContext(
            m_width, m_height, AV_PIX_FMT_RGBA,
            m_width, m_height, AV_PIX_FMT_YUV420P,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }

    uint8_t* srcSlice[] = { static_cast<uint8_t*>(mapped.pData) };
    int srcStride[] = { static_cast<int>(mapped.RowPitch) };

    sws_scale(m_swsContext, srcSlice, srcStride, 0, m_height,
              m_frame->data, m_frame->linesize);

    context->Unmap(frame, 0);
    context->Release();

    // Encode frame
    if (avcodec_send_frame(m_codecContext, m_frame) < 0) {
        std::cerr << "Error sending frame to encoder" << std::endl;
        return false;
    }

    encoded.clear();
    while (true) {
        int ret = avcodec_receive_packet(m_codecContext, m_packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        else if (ret < 0) {
            std::cerr << "Error receiving packet from encoder" << std::endl;
            return false;
        }

        encoded.insert(encoded.end(), m_packet->data, m_packet->data + m_packet->size);
        av_packet_unref(m_packet);
    }

    return !encoded.empty();
}

void FFmpegEncoder::Shutdown() {
    if (m_swsContext) {
        sws_freeContext(m_swsContext);
        m_swsContext = nullptr;
    }

    if (m_frame) {
        av_frame_free(&m_frame);
    }

    if (m_packet) {
        av_packet_free(&m_packet);
    }

    if (m_codecContext) {
        avcodec_free_context(&m_codecContext);
    }

    m_initialized = false;
} 