#!/usr/bin/env python3
from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
import subprocess, os
import threading

app = Flask(__name__)
CORS(app)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(BASE_DIR)
IMAGE_DIR = os.path.join(PROJECT_ROOT, "Target_Detect", "img", "true")
LOG_DIR = os.path.join(BASE_DIR, "logs")
os.makedirs(LOG_DIR, exist_ok=True)

SCRIPTS = [
    os.path.join(BASE_DIR, "1_slam.sh"),
    os.path.join(BASE_DIR, "2_path_plan.sh"),
    os.path.join(BASE_DIR, "4_target_detect.sh"),
    os.path.join(BASE_DIR, "3_path_run.sh"),
]

script_status = ["idle"] * len(SCRIPTS)
script_procs = [None] * len(SCRIPTS)
status_lock = threading.Lock()


def run_script(idx):
    path = SCRIPTS[idx]
    os.chmod(path, 0o755)
    log_path = os.path.join(LOG_DIR, f"script_{idx + 1}.log")

    with status_lock:
        script_status[idx] = "running"

    def target():
        try:
            with open(log_path, "w") as log:
                log.write(f"Starting: {path}\n")
                log.flush()
                # 不再使用 gnome-terminal，直接后台执行，避免 Docker/无桌面时卡住
                proc = subprocess.Popen(
                    ["bash", path],
                    stdout=log,
                    stderr=subprocess.STDOUT,
                    cwd=BASE_DIR,
                    env=os.environ.copy(),
                )
                with status_lock:
                    script_procs[idx] = proc
                proc.wait()
                log.write(f"\nExit code: {proc.returncode}\n")
        except Exception as e:
            with open(log_path, "a") as log:
                log.write(f"\nERROR: {e}\n")
        finally:
            with status_lock:
                script_status[idx] = "idle"
                script_procs[idx] = None

    threading.Thread(target=target, daemon=True).start()


@app.route("/")
def index():
    return send_from_directory(BASE_DIR, "dashboard.html")


@app.route("/dashboard.html")
def dashboard():
    return send_from_directory(BASE_DIR, "dashboard.html")


@app.route("/start/<int:idx>", methods=["POST"])
def start(idx):
    if 1 <= idx <= 4:
        with status_lock:
            if script_status[idx - 1] == "running":
                return jsonify({"status": "error", "msg": "脚本已在运行"}), 400
        run_script(idx - 1)
        return jsonify({"status": "ok", "msg": f"脚本 {idx} 已启动，日志: logs/script_{idx}.log"})
    return jsonify({"status": "error", "msg": "无效脚本编号"}), 400


@app.route("/status")
def status():
    with status_lock:
        return jsonify(script_status)


@app.route("/reset", methods=["POST"])
def reset():
    """手动把卡住的状态清回 idle，并尝试结束仍在运行的进程。"""
    with status_lock:
        for i, proc in enumerate(script_procs):
            if proc is not None and proc.poll() is None:
                proc.terminate()
            script_procs[i] = None
            script_status[i] = "idle"
    return jsonify({"status": "ok", "msg": "状态已重置"})


@app.route("/images")
def list_images():
    if not os.path.exists(IMAGE_DIR):
        return jsonify([])
    try:
        files = sorted(
            [f for f in os.listdir(IMAGE_DIR) if f.lower().endswith((".png", ".jpg", ".jpeg", ".gif"))],
            key=lambda f: os.path.getmtime(os.path.join(IMAGE_DIR, f)),
            reverse=True,
        )
        return jsonify(files)
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@app.route("/get_image/<path:filename>")
def get_image(filename):
    return send_from_directory(IMAGE_DIR, filename)


if __name__ == "__main__":
    print(f"Project root: {PROJECT_ROOT}")
    print(f"Dashboard: http://127.0.0.1:5000/")
    print(f"Reset status: curl -X POST http://127.0.0.1:5000/reset")
    app.run(host="0.0.0.0", port=5000)
