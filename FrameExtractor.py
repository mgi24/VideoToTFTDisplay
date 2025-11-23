import cv2
import os

INPUT_VIDEO = "badapple.mp4" #edit sesuai file video
OUTPUT_DIR = "frame"
WIDTH, HEIGHT = 128, 64
THRESH_VALUE = 127  # dapat diubah jika perlu

def ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)

def process_video():
    cap = cv2.VideoCapture(INPUT_VIDEO)
    if not cap.isOpened():
        print(f"Gagal membuka video: {INPUT_VIDEO}")
        return

    ensure_dir(OUTPUT_DIR)
    frame_idx = 0

    while True:
        ok, frame = cap.read()
        if not ok:
            break

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        resized = cv2.resize(gray, (WIDTH, HEIGHT), interpolation=cv2.INTER_AREA)
        _, binary = cv2.threshold(resized, THRESH_VALUE, 255, cv2.THRESH_BINARY)

        filename = f"{frame_idx:05d}.png"
        out_path = os.path.join(OUTPUT_DIR, filename)
        cv2.imwrite(out_path, binary)

        frame_idx += 1

    cap.release()
    print(f"Selesai. Total frame: {frame_idx}")

if __name__ == "__main__":
    process_video()