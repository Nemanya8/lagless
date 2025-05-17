import cv2
import numpy as np
import socket
import struct
import time
import threading
import queue
import ffmpeg

class VideoStreamer:
    def __init__(self, host='0.0.0.0', port=9999, camera_id=0):
        self.host = host
        self.port = port
        self.camera_id = camera_id
        self.frame_queue = queue.Queue(maxsize=30)
        self.running = False
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        
    def start_capture(self):
        """Start capturing frames from the camera"""
        self.running = True
        cap = cv2.VideoCapture(self.camera_id)
        
        while self.running:
            ret, frame = cap.read()
            if not ret:
                continue
                
            if not self.frame_queue.full():
                self.frame_queue.put(frame)
            else:
                try:
                    self.frame_queue.get_nowait()
                    self.frame_queue.put(frame)
                except queue.Empty:
                    pass
                    
        cap.release()
        
    def encode_frame(self, frame):
        """Encode frame using FFmpeg"""
        try:
            # Convert frame to bytes
            _, buffer = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
            return buffer.tobytes()
        except Exception as e:
            print(f"Error encoding frame: {e}")
            return None
            
    def stream_frames(self, target_address):
        """Stream frames to the target address"""
        while self.running:
            try:
                frame = self.frame_queue.get(timeout=1)
                encoded_frame = self.encode_frame(frame)
                
                if encoded_frame:
                    # Send frame size first
                    size = len(encoded_frame)
                    self.sock.sendto(struct.pack("!I", size), target_address)
                    # Send frame data
                    self.sock.sendto(encoded_frame, target_address)
                    
            except queue.Empty:
                continue
            except Exception as e:
                print(f"Error streaming frame: {e}")
                continue
                
    def start(self, target_address):
        """Start the streaming process"""
        capture_thread = threading.Thread(target=self.start_capture)
        stream_thread = threading.Thread(target=self.stream_frames, args=(target_address,))
        
        capture_thread.start()
        stream_thread.start()
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            self.stop()
            
    def stop(self):
        """Stop the streaming process"""
        self.running = False
        self.sock.close()

if __name__ == "__main__":
    # Example usage
    streamer = VideoStreamer(port=9999)
    target_address = ('127.0.0.1', 9998)  # Change this to the receiver's address
    streamer.start(target_address) 