import os
import cv2

FRAME_DIR = "frame"
OUTPUT_FILE = "frame.txt"
WIDTH, HEIGHT = 128, 64  # verifikasi ukuran

def list_frame_files():
    files = [f for f in os.listdir(FRAME_DIR) if f.lower().endswith(".png")]
    return sorted(files)

def frame_to_bits(path: str) -> str:
    img = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        return ""
    if img.shape != (HEIGHT, WIDTH):
        # optional: paksa resize jika tidak sesuai
        img = cv2.resize(img, (WIDTH, HEIGHT), interpolation=cv2.INTER_NEAREST)
    # Pastikan biner: ubah semua >0 menjadi 1
    flat = (img.flatten() > 0).astype('uint8')
    # Convert to OLED format (pack 8 pixels per byte, MSB first)
    num_bytes = (len(flat) + 7) // 8
    image = bytearray(num_bytes)
    for i in range(len(flat)):
        if flat[i] != 0:
            byte_index = i // 8
            bit_index = 7 - (i % 8)
            image[byte_index] |= (1 << bit_index)
    # Convert bytes to string representation
    return "".join(f"{b:02x}" for b in image)

def generate_frame_txt():
    files = list_frame_files()
    if not files:
        print("Tidak ada file frame PNG.")
        return
    with open(OUTPUT_FILE, "w", encoding="utf-8") as out:
        for i, fname in enumerate(files):
            bits = frame_to_bits(os.path.join(FRAME_DIR, fname))
            if not bits:
                continue
            out.write(bits + "\n")
            if (i + 1) % 500 == 0:
                print(f"Progress: {i+1} frame")
    print(f"Selesai. Total frame tertulis: {len(files)} -> {OUTPUT_FILE}")

if __name__ == "__main__":
    generate_frame_txt()