text = "😊"
bytes_obj = text.encode('utf-8')  # b'\xf0\x9f\x98\x8a'
print(bytes_obj)      # b'\xf0\x9f\x98\x8a'
print(bytes_obj[0])   # 240 (first byte: \xf0)
print(bytes_obj[1])   # 159 (second byte: \x9f)
print(bytes_obj[2])   # 152 (second byte: \x98)
print(bytes_obj[3])   # 138 (third byte: \x8a)