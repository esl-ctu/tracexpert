# Leakage predictions

TraceExpert plugin *Leakage predictions* creates leakage predictions suitable for a CPA attack.

## AES-128

The plugin contains an analytical device for AES-128, which provides leakage predictions that can be correlated to actually measured power consumption or electromagnetic radiation.

### Pre-initialization configuration

The only **Key length**, which can be set at the moment, is 128 bits.

The **Operation** is one of the following:

1. **Encryption, first round, Hamming weight**
2. **Encryption, first round, Identity**
3. **Encryption, last round, Hamming distance**
4. **Encryption, last round, Identity**

The first two options make predictions about the leakage at the output of the S-box in the first round, using either Hamming weight or Identity leakage function. The device then accepts plaintext blocks. 

The latter two make predictions about leakage depicted below, either the Hamming distance, or the Identity of STATE 9. The device then accepts ciphertext blocks.

A typical choice for CPA on software AES is the option #1, while for CPA on hardware AES the option #2. 

![Hamming distance diagram](images/aes-hd.png)

### Input

The stream **Plaintext**/**Ciphertext** accepts full blocks of AES, i.e. 16 bytes of plaintext/ciphertext per block. 

### Actions

1. **Compute predictions (+ flush streams)** first *deletes all unread data from the output streams buffers* and then processes the AES input blocks from the input stream.
The leakage predictions are then ready to be read from the output streams. 
The action fails when an invalid amount of data was previously submitted to the input stream (the number of bytes must be divisible by 16). 

2. **Reset (delete all data)** resets the state of the analytical device to the after-init state.

### Output

After the **Compute predictions (+ flush streams)** action finishes, the leakage predictions can be read from the output streams:

1. Byte 0 Leakage Predictions
2. Byte 1 Leakage Predictions
3. ...
4. Byte 15 Leakage Predictions

Each output stream contains leakage predictions for one of the 16 bytes of the attacked AES-128 round key (leftmost byte being byte #0).

#### Single leakage predictions stream

For each submitted plain/cipher text block, the device produces 256 leakage predictions for every possible value of the byte in the round key (0x00 - 0xFF). Every leakage prediction is 1 byte. 

### Example usage

Attacker has captured 10 ciphertexts and 10 power traces from a hardware implementation of AES-128 encryption. She wishes to attack the 3rd byte of the key.

She chooses **Encryption, last round, Hamming distance**. Then she submits the 10 captured ciphertexts to the **Ciphertext** input stream, and launches the **Compute predictions (+ flush streams)** action. 

She then reads 10\*256 bytes of leakage predictions from the **Byte 3 Leakage Predictions** output stream. The first 256 bytes contain 256 leakage predictions that belong to the 1st measured power trace and can be correlated against it. The second 256 bytes contain 256 leakage predictions that belong to the 2st measured power trace. Overall she has read 10\*256 leakage predictions for based on 10 ciphertexts.

The correlations between these leakage predictions and power traces can be computed using the **Correlation power analysis** component.

If she then wishes to attack 4th byte of the key, she obtains the data from the **Byte 4 Leakage Predictions** output stream.

