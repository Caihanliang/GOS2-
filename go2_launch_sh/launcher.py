#!/usr/bin/env python3
import tkinter as tk
import subprocess
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))

SCRIPTS = [
    os.path.join(BASE_DIR, "1_slam.sh"),
    os.path.join(BASE_DIR, "2_path_plan.sh"),
    os.path.join(BASE_DIR, "3_path_run.sh"),
    os.path.join(BASE_DIR, "4_target_detect.sh"),
]

LABELS = [
    "1. 启动建图",
    "2. 规划路线",
    "3. 执行路径",
    "4. 启动识别",
]


def run_script(script):
    os.chmod(script, 0o755)
    log_dir = os.path.join(BASE_DIR, "logs")
    os.makedirs(log_dir, exist_ok=True)
    name = os.path.basename(script)
    log_path = os.path.join(log_dir, f"launcher_{name}.log")
    with open(log_path, "w") as log:
        subprocess.Popen(
            ["bash", script],
            stdout=log,
            stderr=subprocess.STDOUT,
            cwd=BASE_DIR,
            env=os.environ.copy(),
        )


root = tk.Tk()
root.title("Go2 脚本启动器")
root.geometry("300x280")

for label, script in zip(LABELS, SCRIPTS):
    btn = tk.Button(
        root,
        text=label,
        font=("Arial", 14),
        command=lambda s=script: run_script(s),
    )
    btn.pack(fill="both", expand=True, padx=20, pady=10)

root.mainloop()
