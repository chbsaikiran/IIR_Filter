import numpy as np
from scipy.signal import iirfilter, sosfilt, sosfreqz
import matplotlib.pyplot as plt
import struct

# Parameters
order = 4
cutoff = 0.3  # Normalized frequency (0 to 1, where 1 is the Nyquist frequency)
btype = 'low'  # Filter type: 'low', 'high', 'band', 'stop'

# Set a random seed for reproducibility
np.random.seed(0)

# Design the IIR filter in second-order sections (biquads)
sos = iirfilter(order, cutoff, btype=btype, ftype='butter', output='sos')

# Generate a sample signal (e.g., a noisy sine wave)
fs = 1000  # Sampling frequency in Hz
t = np.linspace(0, 1, fs, endpoint=False)
signal = np.sin(2 * np.pi * 50 * t) + 0.5 * np.random.randn(t.size)
signal = signal/max(abs(signal))

# Apply the IIR filter to the signal using second-order sections
filtered_signal = sosfilt(sos, signal)

# Plot the frequency response of the filter
w, h = sosfreqz(sos, worN=2000)
plt.figure()
plt.plot(0.5 * fs * w / np.pi, np.abs(h), 'b')
plt.plot(cutoff * fs, 0.5 * np.sqrt(2), 'ko')
plt.axvline(cutoff * fs, color='k')
plt.xlim(0, 0.5 * fs)
plt.title("Frequency Response")
plt.xlabel('Frequency (Hz)')
plt.ylabel('Gain')
plt.grid()

# Plot the original and filtered signals
plt.figure()
plt.plot(t, signal, 'b', alpha=0.75, label='Original Signal')
plt.plot(t, filtered_signal, 'r', alpha=0.75, label='Filtered Signal')
plt.legend()
plt.xlabel('Time [seconds]')
plt.grid()

plt.show()

# Write the filtered signal to a binary file
with open('filtered_signal.bin', 'wb') as f:
    for value in filtered_signal:
        f.write(struct.pack('f', value))

print("Filtered signal has been written to 'filtered_signal.bin'.")


# Write the filtered signal to a binary file
with open('signal.bin', 'wb') as f:
    for value in signal:
        f.write(struct.pack('f', value))

print("signal has been written to 'signal.bin'.")
