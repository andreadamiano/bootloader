import os 

with open("build/main_app.bin", "rb") as f:
    data = f.read()
    print("Hex dump:")
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ' '.join(f"{b:02x}" for b in chunk)
        print(f"{i:08x}: {hex_str:<48}")

    print(f"File size: {os.path.getsize("build/main_app.bin")}")