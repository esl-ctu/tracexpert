# TraceXpert 1.0 User Guide

**An integrated environment for side-channel data acquisition and analysis**

## Introduction

TraceXpert 1.0 is a desktop application designed to facilitate both the measurement and statistical analysis of side-channel data. It provides a unified framework for conducting correlation power analysis (CPA), classification-based attacks, and leakage assessments, supporting researchers and practitioners in the field of hardware security evaluation. While the software assumes familiarity with the principles of side-channel analysis -- the study of information leakage through physical emissions such as power consumption -- it offers a streamlined and user-friendly environment for managing complex experimental workflows.

TraceXpert integrates seamlessly with a variety of **measurement** and **communication** devices, including serial ports, smartcards, ChipWhisperer boards, and Picoscope 6-series oscilloscopes. It also supports arbitrary **protocol communication**, including parsing of pre-defined messages. Direct interaction with connected hardware components is possible through separate widgets. 

TraceXpert's modular architecture includes virtual **analytical devices** capable of generating power predictions, performing CPA on cryptographic algorithms such as AES, enumerating internal variables, and applying Welch's t-test for leakage detection. Data can be stored at any point in the workflow using TraceXpert's custom binary trace format, ensuring reproducibility and efficient storage.

A key feature of TraceXpert is its **scenario system**, which enables users to visually construct analysis pipelines through interconnected graphical blocks. This allows rapid prototyping and automation of measurement, acquisition, and analysis procedures without manual reconfiguration. More complex or unsupported operations may be further defined using Python scripts inside the scenario. 

TraceXpert 1.0 supports both Windows and Linux platforms and is distributed as open-source software, intended primarily for academic and laboratory research applications.

## Index

- [Title Page, Index](README.md)
- 1 [Getting Started](gettingstarted.md)
- 2 [Project Manager](projects.md)
- 3 [Components and Devices](components-devices.md)
- 4 [I/O Devices](iodevices.md)
    - 4.1 [File](file.md)
    - 4.2 [Random number generator](rng.md)
    - 4.3 [Serial port](serialport.md)
    - 4.4 [Smart card](smartcard.md)
- 5 [Protocols](protocols.md)
- 6 [Oscilloscopes](oscilloscopes.md)
    - 6.1 [NewAE (ChipWhisperer)](newae.md)
    - 6.2 [Picoscope 6000 A/B/C/D/E series](picoscope6000.md)
- 7 [Analytical Devices](analytical.md)
    - 7.1 [Ciphers](ciphers.md)
    - 7.2 [Leakage predictions](predictions.md)
    - 7.3 [Correlation power analysis](cpa.md)
    - 7.4 [Welch's t-test leakage assessment](ttest.md)
- 8 [Scenarios](scenarios.md)
- 9 [Example use-case](examples.md)
    - 9.1 [Correlation power analysis](example-cpa.md)
    - 9.2 [Non-specific t-test](example-nsttest.md)
    - 9.3 [Specific t-test](example-sttest.md)
- [Build instructions](build.md)

![TraceXpert logo](images/tracexpert256.png)

