# P2P Video Streaming Protocol

This is a simple P2P video streaming implementation using UDP protocol and FFmpeg for video encoding/decoding. The implementation consists of a sender and receiver component that can be run on different machines to stream video over a network.

## Requirements

- Python 3.7+
- FFmpeg installed on your system
- Required Python packages (install using `pip install -r requirements.txt`):
  - opencv-python
  - numpy
  - ffmpeg-python

## Installation

1. Clone this repository
2. Install the required packages:
   ```bash
   pip install -r requirements.txt
   ```
3. Make sure FFmpeg is installed on your system

## Usage

### Running the Receiver

1. Start the receiver first:
   ```bash
   python receiver.py
   ```
   The receiver will listen on port 9998 by default.

### Running the Sender

1. Start the sender:
   ```bash
   python sender.py
   ```
   The sender will capture video from your default camera and stream it to the receiver.

2. To change the target address, modify the `target_address` in `sender.py` to match your receiver's IP address.

## Features

- Real-time video streaming over UDP
- Frame buffering to handle network latency
- Automatic frame dropping when buffer is full
- JPEG compression for efficient bandwidth usage
- Multi-threaded implementation for smooth performance

## Customization

You can modify the following parameters in both sender.py and receiver.py:
- Host address
- Port number
- Camera ID (for sender)
- Frame quality
- Buffer size

## Notes

- The default implementation uses JPEG compression for simplicity and efficiency
- The streaming is unidirectional (sender to receiver)
- Press 'q' in the receiver window to quit
- Use Ctrl+C in the terminal to stop either the sender or receiver 