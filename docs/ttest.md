[Back to the top (index)](README.md)

# Welch's t-test leakage assessment

TraceExpert component *Welch's t-test leakage assessment* provides a t-value computation for the leakage assessment using Welch's t-test.

## Univariate Welch's t-test

The component contains an analytical device for t-test leakage assessment. It accepts power traces of different classes. Then it computes the Welch's t-test value between traces in every class, for every sampling point independently. It outputs t-values and degrees of freedom.

The analytical device allows for incremental computation (see Actions). 

### Pre-initialization configuration

* **Trace length (in samples)**: The number of samples in each power trace.
* **Number of classes**: The number of different classes to be compared. For non-specific t-test, choose 2 classes (random vs fixed).
* **Sample data type**: The data type of the samples (ranging from 8-32 bit signed or unsigned, or 32/64 bit float)
* **Maximum order**: The order of the leakage up to which the detection aims (preprocesses the traces on the go). If unsure, keep order 1. Has a direct impact on the computation time.
* **Input format** can be one of the following:
    - **Input stream per class**: A separate input stream will be created for every class. The power traces belonging to different classes are submitted to different streams.
    - **Label + traces streams**: Two streams are created. One stream accepts power traces. The other stream accepts labels (data type is set as a subparameter) of the traces. **The labels must be from range 0 to N-1, where N is the number of classes**. 

### Input

The input streams are arranged according to the previously configured *Input format*. 

The traces streams accept whole power traces (i.e., the previously configured number of samples).

The labels stream (if configured) accepts labels ranging from 0 to N-1, where N is the previously configured number of classes. The number of submitted power traces and labels must match. 

### Actions

1. **Compute t-values (+ flush streams)** first *deletes all unread data from the output streams buffers* and then computes t-test t-values between traces in each class. The t-value is computed in every sampling point independently. The t-values are based on all previously submitted data (including data sent prior to previous computation). The t-values and corresponding degrees of freedom are then ready to be read from the output streams. The action fails when an invalid amount of data was previously submitted to the input stream (the number of submitted samples must be divisible by the trace length, i.e., the traces are complete, and the number of labels must match the number of submitted power traces). 

2. **Reset (delete all data)** resets the state of the analytical device to the after-init state, i.e., it also deletes information about any previously submitted data.

### Output

After the **Compute t-values (+ flush streams)** action finishes, the t-values and corresponding degrees of freedom can be read from the output streams:

1. **1-order t-vals 0 vs 1** contains results for t-test between classes 0 and 1
2. ...

Each output stream contains N t-values followed by N degrees of freedom, where N is the number of samples in a trace. Both the t-values and the degrees of freedom are a 8 bytes long real numbers (double).

### Example usage

Attacker has captured 100,000 power traces, each with 1500 signed 16-bit samples, while encrypting either random or fixed plaintexts in an randomly interleaved fashion. She has 100,000 power traces and 100,000 byte-size labels (0x00 and 0x01).   

She wishes to compute a 1st order non-specific t-test. She configures:

* **Trace length (in samples)**: 1500
* **Number of classes**: 2
* **Sample data type**: Signed 16 bit
* **Maximum order**: 1
* **Input format**: Label + traces streams    
    - **Label data type**: Unsigned 8 bit

She then submits the power traces to the *Traces* stream, and the labels to the *Labels* stream. Then she runs the *Compute t-values (+ flush streams)* action.

After that she reads the 1500 t-values from the *1-order t-vals 0 vs 1* stream. If she wishes, she furthermore reads 1500 degrees of freedom.

If she then captures another 100,000 traces, she can simply submit the data to the streams and run the *Compute t-values (+ flush streams)* action again. The provided t-values are now based on 200,000 traces. *Warning: the computation context of the analytical device (i.e., previously submitted data) is not saved along with the project.*

If she wishes to start the computation all over again, she rans the *Reset* action.

