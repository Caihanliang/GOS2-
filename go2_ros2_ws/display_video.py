#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
通过 WebRTC 拉取 Go2 前置摄像头画面：
- 窗口实时显示
- 大约每秒保存一帧到 Target_Detect/img/pred，供目标检测监控使用
注意：这是 WebRTC 通道，不是 unitree_ros2 话题。
"""
import cv2
import numpy as np
import os
import time
import asyncio
import logging
import threading
from queue import Queue
from go2_webrtc_driver.webrtc_driver import Go2WebRTCConnection, WebRTCConnectionMethod
from aiortc import MediaStreamTrack

logging.basicConfig(level=logging.FATAL)

# 先创建黑窗，避免后续无帧时窗口卡住
height, width = 720, 1280
img = np.zeros((height, width, 3), dtype=np.uint8)
cv2.imshow("Video", img)
cv2.waitKey(1)

# 检测输入目录（相对本仓库 Target_Detect）
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SAVE_DIR = os.path.join(ROOT, "Target_Detect", "img", "pred")
os.makedirs(SAVE_DIR, exist_ok=True)
last_save_ts = 0


def main():
    frame_queue = Queue()

    # 按实际机器人 IP 修改
    conn = Go2WebRTCConnection(WebRTCConnectionMethod.LocalSTA, ip="192.168.123.161")

    async def recv_camera_stream(track: MediaStreamTrack):
        """异步接收视频帧，放入队列供主线程显示/保存"""
        while True:
            frame = await track.recv()
            img_bgr = frame.to_ndarray(format="bgr24")
            frame_queue.put(img_bgr)

    def run_asyncio_loop(loop):
        """在独立线程中运行 asyncio 事件循环"""
        asyncio.set_event_loop(loop)

        async def setup():
            await conn.connect()
            conn.video.switchVideoChannel(True)
            conn.video.add_track_callback(recv_camera_stream)

        loop.run_until_complete(setup())
        loop.run_forever()

    loop = asyncio.new_event_loop()
    asyncio_thread = threading.Thread(target=run_asyncio_loop, args=(loop,))
    asyncio_thread.start()

    # 主循环：显示画面 + 每秒存一帧
    try:
        while True:
            if not frame_queue.empty():
                frame = frame_queue.get()

                now = int(time.time())
                global last_save_ts
                if now != last_save_ts:
                    last_save_ts = now
                    cv2.imwrite(os.path.join(SAVE_DIR, f"{now}.jpg"), frame)

                cv2.imshow("Video", frame)
                if cv2.waitKey(1) & 0xFF == ord("q"):
                    break
            else:
                time.sleep(0.01)
    finally:
        cv2.destroyAllWindows()
        loop.call_soon_threadsafe(loop.stop)
        asyncio_thread.join()


if __name__ == "__main__":
    main()
