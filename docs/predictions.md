# Power predictions

TraceExpert plugin *predict* creates power predictions suitable for CPA attack.

## AES-128

The plugin contains an analytical device for AES-128. The device can be configured to work with two different power models:

1. Hamming weight or Identity of the S-box output in the first round (suitable for attacking SW implementations)
2. Hamming distance or Identity on the working register in the last round (suitable for attacking HW implementations)

It accepts plaintexts on an input stream. 

When the "Compute predictions" actions is launched, the device enumerates working values during the encryption (as mentioned) for every submitted plaintext and every possible value of the key (each key byte is independent in this scenario).

The device creates 16 output streams, one for each byte of the key. Each output stream then returns 256 bytes for every submitted plaintext. Every byte contains either Hamming weight/distance or Identity, based on the settings.

