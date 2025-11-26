# Correlation power analysis

TraceExpert component *Correlation power analysis* provides a correlation coefficient computation for the CPA attack.

## Univariate arbitrary-order CPA

The component contains an analytical device for univariate CPA attack. It accepts power traces and leakage predictions. Then it computes the correlation coefficients between every sample of the power traces and every leakage prediction. It outputs correlation matrices.

The analytical device allows for incremental computation (see Actions). 

### Pre-initialization configuration

* **Trace length (in samples)**: The number of samples in each power trace.
* **Sample data type**: The data type of the samples (ranging from 8-32 bit signed or unsigned, or 32/64 bit float)
* **Number of predictions per trace**: The number of leakage predictions to be correlated with each sampling point in the trace. I.e., **number of key hypotheses**.
* **Prediction data type**: The data type of the predictions (same as for sample data type)
* **Maximum order**: The order of the attack up to which the correlations will be computed. If unsure, keep order 1. Has a direct impact on the computation time.

### Input

The input consists of two streams:

* **Traces**: Accepts arbitrary amount of power traces (consisting of previously configured number of samples).
* **Predictions**: For each power trace submitted, the previously configured number of predictions must be submitted. 

### Actions

1. **Compute correlation matrix (+ flush streams)** first *deletes all unread data from the output streams buffers* and then computes correlation coefficients between every sampling point and every key hypothesis.
The correlation coefficients are based on all previously submitted data (including data sent prior to previous computation).
The correlation matrices are then ready to be read from the output streams. 
The action fails when an invalid amount of data was previously submitted to the input stream (the number of submitted samples must be divisible by the trace length, i.e., the traces are complete, and the number of power predictions for each power trace must match the number of key hypotheses). 

2. **Reset (delete all data)** resets the state of the analytical device to the after-init state, i.e., it also deletes information about any previously submitted data.

### Output

After the **Compute correlation matrix (+ flush streams)** action finishes, the correlation coefficients can be read from the output streams:

1. 1-order correlation matrix
2. ...

Each output stream contains a correlation matrix N x M, where N is the number of samples in a trace, and M is the number of key hypotheses. The correlation coefficient is a 8 bytes long real number (double).

### Example usage

Attacker has captured 10 ciphertexts and 10 power traces from a hardware implementation of AES-128 encryption. She wishes to attack the 3rd byte of the key. She has used the **Leakage predictions** component to obtain 10\*256 bytes of leakage predictions, i.e., 256 byte-sized predictions (based on 256 key hypotheses) for each of her power traces. 

She then submits the power traces to the *Traces* stream, and the prediction to the *Predictions* stream, and runs the *Compute correlation matrix (+ flush streams)* action.

After that she reads the correlation matrix from the *1-order correlation matrix* stream.

If she then captures another 10 traces+ciphertexts, she can simply submit the data to the streams and run the *Compute correlation matrix (+ flush streams)* action again. The provided correlation matrix is now based on 20 traces and predictions. *Warning: the computation context of the analytical device (i.e., previously submitted data) is not saved along with the project.*

If she wishes to start the computation all over again, she rans the *Reset* action.

