#! /usr/bin/env python3
# -*- coding: UTF-8 -*-

import cv2
import queue
import threading

# 无缓存读取视频流类
class VideoCapture:

  def __init__(self, name):
    self.cap = cv2.VideoCapture(name)
    self.q = queue.Queue()
    t = threading.Thread(target=self._reader)
    t.daemon = True
    t.start()

  # 帧可用时立即读取帧，只保留最新的帧
  def _reader(self):
    while True:
      ret, frame = self.cap.read()
      if not ret:
        break
      if not self.q.empty():
        try:
          self.q.get_nowait()   # 删除上一个（未处理的）帧
        except queue.Empty:
          pass
      self.q.put(frame)

  def read(self):
    return self.q.get()

  def isOpened(self):
      return self.cap.isOpened()

capture = VideoCapture(10)

if capture.isOpened():
    print("camera open.")

    while(True):
        img = capture.read()

        cv2.imshow('camera', img)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    capture.release()
    cv2.destroyAllWindows()
