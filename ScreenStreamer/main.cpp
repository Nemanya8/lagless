#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

#include "CaptureManager.h"
#include "FFmpegEncoder.h"
#include "UdpSender.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

void PrintUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  -i, --ip <address>    Target IP address (default: 255.255.255.255 for broadcast)\n"
              << "  -p, --port <port>     UDP port (default: 12345)\n"
              << "  -b, --bitrate <rate>  Bitrate in kbps (default: 5000)\n"
              << "  -h, --help           Show this help message\n";
}

int main(int argc, char* argv[]) {
    std::string ip = "255.255.255.255";  // Broadcast address
    int port = 12345;
    int bitrate = 5000000;  // 5 Mbps

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            PrintUsage(argv[0]);
            return 0;
        }
        else if (arg == "-i" || arg == "--ip") {
            if (i + 1 < argc) ip = argv[++i];
        }
        else if (arg == "-p" || arg == "--port") {
            if (i + 1 < argc) port = std::stoi(argv[++i]);
        }
        else if (arg == "-b" || arg == "--bitrate") {
            if (i + 1 < argc) bitrate = std::stoi(argv[++i]) * 1000;  // Convert kbps to bps
        }
    }

    try {
        std::cout << "Starting screen capture stream to " << ip << ":" << port << std::endl;
        std::cout << "Bitrate: " << (bitrate / 1000) << " kbps" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;

        std::cout << "Initializing capture manager..." << std::endl;
        CaptureManager capture;
        if (!capture.Initialize()) {
            std::cerr << "Failed to initialize capture" << std::endl;
            return -1;
        }
        std::cout << "Capture initialized. Screen size: " << capture.GetWidth() << "x" << capture.GetHeight() << std::endl;

        std::cout << "Initializing encoder..." << std::endl;
        FFmpegEncoder encoder;
        if (!encoder.Initialize(capture.GetDevice(), capture.GetWidth(), capture.GetHeight(), bitrate)) {
            std::cerr << "Failed to initialize encoder" << std::endl;
            return -1;
        }
        std::cout << "Encoder initialized" << std::endl;

        std::cout << "Initializing UDP sender..." << std::endl;
        UdpSender sender(ip, port);
        if (!sender.Initialize()) {
            std::cerr << "Failed to initialize UDP sender" << std::endl;
            return -1;
        }
        std::cout << "UDP sender initialized" << std::endl;

        std::cout << "Starting capture loop..." << std::endl;
        int frameCount = 0;
        auto startTime = std::chrono::steady_clock::now();
        auto lastStatsTime = startTime;

        while (true) {
            ID3D11Texture2D* frame = capture.CaptureFrame();
            if (!frame) {
                std::cerr << "Failed to capture frame" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            std::vector<uint8_t> encoded;
            if (encoder.EncodeFrame(frame, encoded)) {
                if (!encoded.empty()) {
                    if (!sender.Send(encoded)) {
                        std::cerr << "Failed to send frame" << std::endl;
                    }
                    frameCount++;
                }
            }

            // Print stats every second
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count();
            if (elapsed >= 1) {
                auto totalElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
                float fps = static_cast<float>(frameCount) / elapsed;
                std::cout << "FPS: " << fps << " | Total frames: " << frameCount 
                         << " | Running time: " << totalElapsed << "s" << std::endl;
                frameCount = 0;
                lastStatsTime = now;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
