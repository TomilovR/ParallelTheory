import cv2
import time
import threading
import queue
import argparse
import logging
import os

LOG_DIR = os.path.join(os.getcwd(), "log")
os.makedirs(LOG_DIR, exist_ok=True)
logging.basicConfig(
    filename=os.path.join(LOG_DIR, "app.log"),
    level=logging.INFO,
    format='%(asctime)s:%(levelname)s:%(message)s'
)

class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")

class SensorX(Sensor):
    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data

class SensorCam(Sensor):
    def __init__(self, cam_name: str, resolution: str):
        try:
            width, height = map(int, resolution.split("x"))
        except Exception as e:
            logging.error("Invalid resolution format: %s", resolution)
            raise

        self.cap = cv2.VideoCapture(int(cam_name))
        if not self.cap.isOpened():
            logging.error("Camera with name %s not found or cannot be opened.", cam_name)
            raise RuntimeError(f"Camera {cam_name} not found")
        self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
        self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

    def get(self):
        ret, frame = self.cap.read()
        if not ret or frame is None:
            logging.error("Error reading from camera")
            return None
        return frame

    def __del__(self):
        if hasattr(self, 'cap') and self.cap is not None:
            self.cap.release()

class WindowImage:
    def __init__(self, display_rate: float):
        self.display_rate = display_rate
        self.window_name = "Sensor Display"
        cv2.namedWindow(self.window_name, cv2.WINDOW_NORMAL)

    def show(self, img):
        cv2.imshow(self.window_name, img)
        cv2.waitKey(1)

    def __del__(self):
        cv2.destroyAllWindows()

def sensor_worker(sensor, sensor_queue, stop_event):
    while not stop_event.is_set():
        try:
            value = sensor.get()
            sensor_queue.put(value)
        except Exception as e:
            logging.error("Error reading from sensor: %s", e)

def camera_worker(sensor_cam, frame_queue, stop_event):
    while not stop_event.is_set():
        try:
            frame = sensor_cam.get()
            if frame is not None:
                while not frame_queue.empty():
                    try:
                        frame_queue.get_nowait()
                    except queue.Empty:
                        break
                frame_queue.put(frame)
        except Exception as e:
            logging.error("Error in camera worker: %s", e)

def main():
    parser = argparse.ArgumentParser(description="Run sensor reading and data display")
    parser.add_argument("--cam", type=str, required=True, help="Camera name in the system")
    parser.add_argument("--res", type=str, required=True, help="Desired camera resolution, e.g., 1280x720")
    parser.add_argument("--display_rate", type=float, required=True, help="Display window refresh rate")
    args = parser.parse_args()

    try:
        sensor_cam = SensorCam(args.cam, args.res)
    except Exception as e:
        logging.error("Error initializing SensorCam: %s", e)
        return

    sensor0 = SensorX(0.01)
    sensor1 = SensorX(0.1)
    sensor2 = SensorX(1)

    queue0 = queue.Queue()
    queue1 = queue.Queue()
    queue2 = queue.Queue()
    frame_queue = queue.Queue(maxsize=1)

    stop_event = threading.Event()

    threads = []
    for sensor, q in zip([sensor0, sensor1, sensor2], [queue0, queue1, queue2]):
        t = threading.Thread(target=sensor_worker, args=(sensor, q, stop_event))
        t.daemon = True
        t.start()
        threads.append(t)

    camera_thread = threading.Thread(target=camera_worker, args=(sensor_cam, frame_queue, stop_event))
    camera_thread.daemon = True
    camera_thread.start()
    threads.append(camera_thread)

    window = WindowImage(args.display_rate)

    last_val0 = last_val1 = last_val2 = None
    last_frame = None

try:
    while True:
        try:
            while True:
                last_frame = frame_queue.get_nowait()
        except queue.Empty:
            pass

        try:
            while True:
                last_val0 = queue0.get_nowait()
        except queue.Empty:
            pass
        try:
            while True:
                last_val1 = queue1.get_nowait()
        except queue.Empty:
            pass
        try:
            while True:
                last_val2 = queue2.get_nowait()
        except queue.Empty:
            pass

        if last_frame is not None:
            text = f"SensorX0: {last_val0} | SensorX1: {last_val1} | SensorX2: {last_val2}"
            cv2.putText(last_frame, text, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 
                        0.8, (0, 255, 0), 2)
            window.show(last_frame)

        if cv2.waitKey(int(1000/args.display_rate)) & 0xFF == ord('q'):
            break
except Exception as e:
    logging.error("Error in main loop: %s", e)
finally:
    stop_event.set()
    for t in threads:
        t.join(timeout=1)
    del window

if __name__ == '__main__':
    main()
