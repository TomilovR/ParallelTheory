import argparse
import time
import queue
import threading
import multiprocessing as mp
import os
import cv2
from ultralytics import YOLO

class VideoProcessor:    
    def __init__(self, video_path, output_path, model_path="yolov8s-pose.pt", num_workers=4):
        self.video_path = video_path
        self.output_path = output_path
        self.model_path = model_path
        self.num_workers = num_workers
        
        cap = cv2.VideoCapture(video_path)
        self.frame_width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
        self.frame_height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
        self.fps = cap.get(cv2.CAP_PROP_FPS)
        self.total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        cap.release()
        
        self.video_writer = cv2.VideoWriter(
            output_path,
            cv2.VideoWriter_fourcc(*'mp4v'),
            self.fps,
            (self.frame_width, self.frame_height)
        )
        
        self.input_queue = queue.Queue()
        
        self.processed_frames = {}
        self.next_frame_to_write = 0
        
        self.lock = threading.Lock()
        
    def __del__(self):
        if hasattr(self, 'video_writer'):
            self.video_writer.release()
        
    def worker(self):
        model = YOLO(self.model_path)
    
        while True:
            try:
                frame_number, frame = self.input_queue.get(timeout=5)
                if frame is None:
                    break
            
                results = model(frame)
                processed_frame = results[0].plot()
            
                with self.lock:
                    self.processed_frames[frame_number] = processed_frame
                    while self.next_frame_to_write in self.processed_frames:
                        self.video_writer.write(self.processed_frames[self.next_frame_to_write])
                        del self.processed_frames[self.next_frame_to_write]
                        self.next_frame_to_write += 1
            
                self.input_queue.task_done()
            
            except queue.Empty:
                break
    
    def process_single_thread(self):
        model = YOLO(self.model_path)
        cap = cv2.VideoCapture(self.video_path)
        frame_count = 0
        
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break
            
            results = model(frame)
            processed_frame = results[0].plot()
            
            self.video_writer.write(processed_frame)
            
            frame_count += 1
            if frame_count % 10 == 0:
                print(f"Обработано {frame_count}/{self.total_frames} кадров")
                
        cap.release()
    
    def process_multi_thread(self):
        threads = []
        for _ in range(self.num_workers):
            thread = threading.Thread(target=self.worker)
            thread.daemon = True
            threads.append(thread)
            thread.start()
    
        cap = cv2.VideoCapture(self.video_path)
        frame_number = 0
    
        while cap.isOpened():
            ret, frame = cap.read()
            if not ret:
                break
            
            self.input_queue.put((frame_number, frame))
            frame_number += 1
        
            if frame_number % 10 == 0:
                print(f"Отправлено на обработку {frame_number}/{self.total_frames} кадров")
    
        cap.release()
    
        self.input_queue.join()
    
        for _ in range(self.num_workers):
            self.input_queue.put((None, None))
    
        for thread in threads:
            thread.join()
    
        with self.lock:
            while self.next_frame_to_write in self.processed_frames:
                self.video_writer.write(self.processed_frames[self.next_frame_to_write])
                del self.processed_frames[self.next_frame_to_write]
                self.next_frame_to_write += 1
    
    def run(self, multi_thread=False):
        start_time = time.time()
        
        if multi_thread:
            print(f"Запуск в многопоточном режиме ({self.num_workers} потоков)")
            self.process_multi_thread()
        else:
            print("Запуск в однопоточном режиме")
            self.process_single_thread()
        
        end_time = time.time()
        processing_time = end_time - start_time
        
        self.video_writer.release()
        
        print(f"Обработка завершена за {processing_time:.2f} секунд")
        print(f"Обработанное видео сохранено: {self.output_path}")
        
        return processing_time

def main():
    parser = argparse.ArgumentParser(description="Обработка видео с использованием модели YOLOv8s-pose")
    parser.add_argument("video_path", help="Путь к входному видео")
    parser.add_argument("mode", choices=["single", "multi"])
    parser.add_argument("output_path", help="Путь для сохранения обработанного видео")
    parser.add_argument("--workers", type=int, default=4, help="Количество рабочих потоков (для многопоточного режима)")
    
    args = parser.parse_args()
    
    processor = VideoProcessor(
        args.video_path, 
        args.output_path, 
        num_workers=args.workers
    )
    
    processor.run(multi_thread=(args.mode == "multi"))

if __name__ == "__main__":
    main()
