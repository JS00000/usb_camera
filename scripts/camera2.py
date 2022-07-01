#! /usr/bin/env python3
# -*- coding: UTF-8 -*-

# command to check camera
# cheese
# guvcview
# v4l2-ctl -d /dev/video0 --list-formats-ext

import cv2
cap0 = cv2.VideoCapture(0)
cap0.set(3, 640)
cap0.set(4, 480)
cap1 = cv2.VideoCapture(2)
cap1.set(3, 640)
cap1.set(4, 480)
ret0, frame0 = cap0.read()
assert ret0
ret1, frame1 = cap1.read()
assert ret1

while True:
    ret0, frame0 = cap0.read()
    ret1, frame1 = cap1.read()
    if not ret0:
        print("ERROR cap0")
        break
    if not ret1:
        print("ERROR cap1")
        break
    cv2.imshow('frame0', frame0)
    cv2.imshow('frame1', frame1)
    if cv2.waitKey(1) == ord('q'):
        break
cap0.release()
cap1.release()
cv2.destoryAllWindows()