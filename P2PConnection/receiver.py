import cv2
import numpy as np
import socket
import struct
import threading
import queue
import time

class VideoReceiver:
    def __init__(self, host='0.0.0.0', port=9998):
        self.host = host
        self.port = port
        self.frame_queue = queue.Queue(maxsize=30)
        self.running = False
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.host, self.port))
        
    def receive_frames(self):
        """Receive frames from the network"""
        self.running = True
        while self.running:
            try:
                # Receive frame size
                data, addr = self.sock.recvfrom(4)
                size = struct.unpack("!I", data)[0]
                
                # Receive frame data
                data, addr = self.sock.recvfrom(size)
                
                # Decode frame
                frame = cv2.imdecode(np.frombuffer(data, dtype=np.uint8), cv2.IMREAD_COLOR)
                
                if frame is not None:
                    if not self.frame_queue.full():
                        self.frame_queue.put(frame)
                    else:
                        try:
                            self.frame_queue.get_nowait()
                            self.frame_queue.put(frame)
                        except queue.Empty:
                            pass
                            
            except Exception as e:
                print(f"Error receiving frame: {e}")
                continue
                
    def display_frames(self):
        """Display received frames"""
        while self.running:
            try:
                frame = self.frame_queue.get(timeout=1)
                cv2.imshow('Received Stream', frame)
                
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
                    
            except queue.Empty:
                continue
            except Exception as e:
                print(f"Error displaying frame: {e}")
                continue
                
    def start(self):
        """Start the receiving process"""
        receive_thread = threading.Thread(target=self.receive_frames)
        display_thread = threading.Thread(target=self.display_frames)
        
        receive_thread.start()
        display_thread.start()
        
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            self.stop()
            
    def stop(self):
        """Stop the receiving process"""
        self.running = False
        cv2.destroyAllWindows()
        self.sock.close()

if __name__ == "__main__":
    # Example usage
    receiver = VideoReceiver(port=9998)
    receiver.start() 