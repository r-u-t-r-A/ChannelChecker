import gzip

# Configuration - change these filenames if needed
input_filename = 'elegant_decoded.html'
output_filename = 'elop_edited.cpp'
array_name = 'ELEGANT_HTML'

def encode_html_to_cpp():
    print(f"Reading '{input_filename}'...")
    try:
        with open(input_filename, 'rb') as f:
            html_data = f.read()
    except FileNotFoundError:
        print(f"Error: Could not find '{input_filename}'.")
        return

    print("Compressing data...")
    # Compress the HTML data
    compressed_data = gzip.compress(html_data)
    data_length = len(compressed_data)

    print(f"Formatting {data_length} bytes into a C++ array...")
    
    # Start building the C++ file string
    cpp_output = '#include "elop.h"\n\n'
    cpp_output += f'const uint8_t {array_name}[{data_length}] PROGMEM = {{\n  '

    # Convert the bytes to a list of decimal strings
    byte_strings = [str(b) for b in compressed_data]
    
    # Chunk the array into rows of 20 numbers for neat formatting
    for i in range(0, data_length, 20):
        chunk = byte_strings[i:i+20]
        cpp_output += ', '.join(chunk)
        
        # Add a comma and newline if this isn't the last chunk
        if i + 20 < data_length:
            cpp_output += ',\n  '

    # Close the array
    cpp_output += '\n};\n'

    # Write to the new .cpp file
    with open(output_filename, 'w') as f:
        f.write(cpp_output)

    print(f"Success! Your encoded C++ array is ready in '{output_filename}'.")

if __name__ == "__main__":
    encode_html_to_cpp()
