import gzip

# 1. Read the elop.cpp file you uploaded
with open('elop.cpp', 'r') as f:
    content = f.read()

# 2. Extract the numbers located between the curly braces { }
start = content.find('{')
end = content.find('}')
array_content = content[start+1:end]

# 3. Clean up the text and convert to a list of integers
byte_values = [int(x.strip()) for x in array_content.split(',') if x.strip().isdigit()]

# 4. Convert the integers into a byte array
compressed_data = bytearray(byte_values)

# 5. Decompress the gzip data and save it as an HTML file
try:
    html_data = gzip.decompress(compressed_data)
    with open('elegant_decoded.html', 'wb') as f:
        f.write(html_data)
    print("Success! The file has been decoded to 'elegant_decoded.html'.")
except Exception as e:
    print(f"Error during decompression: {e}")
