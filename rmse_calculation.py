import numpy as np
import struct
import matplotlib.pyplot as plt

def read_float_binary_file(filename):
    """Read a binary file and return a numpy array of floats."""
    data = []
    with open(filename, 'rb') as f:
        while True:
            bytes_read = f.read(4)
            if not bytes_read:
                break
            value = struct.unpack('f', bytes_read)[0]
            data.append(value)
    return np.array(data)

def calculate_rmse_db(file1, file2):
    """Calculate the RMSE in dB between two binary files containing float values."""
    # Read the binary files
    data1 = read_float_binary_file(file1)
    data2 = read_float_binary_file(file2)

    # Ensure both files have the same length
    if len(data1) != len(data2):
        raise ValueError("The two files do not have the same length.")
    
    # Calculate the error
    error = data1 - data2
    
    # Calculate the RMSE
    rmse = np.sqrt(np.mean(error**2))
    
    # Convert RMSE to dB
    rmse_db = 20 * np.log10(rmse)
    
    return data1, data2, error, rmse_db

def plot_signals(data1, data2, error):
    """Plot data1, data2, and error in a single plot with different colors."""
    plt.figure(figsize=(14, 8))

    plt.plot(data1, label='Data 1', color='b')
    plt.plot(data2, label='Data 2', color='g')
    plt.plot(error, label='Error', color='r')
    
    plt.title('Data 1, Data 2, and Error')
    plt.xlabel('Sample Index')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.grid(True)

    plt.show()

# Example usage
file1 = 'filtered_signal.bin'
file2 = 'filtered_msvc_fxd.bin'

data1, data2, error, rmse_db = calculate_rmse_db(file1, file2)
print(f"RMSE in dB: {rmse_db:.2f} dB")

# Plot the signals
plot_signals(data1, data2, error)
