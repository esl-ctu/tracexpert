# TraceXpert 1.0

**An integrated environment for side-channel data acquisition and analysis**

## Introduction

TraceXpert 1.0 is a desktop application designed to facilitate both the measurement and statistical analysis of side-channel data. It provides a unified framework for conducting correlation power analysis (CPA), classification-based attacks, and leakage assessments, supporting researchers and practitioners in the field of hardware security evaluation. While the software assumes familiarity with the principles of side-channel analysis -- the study of information leakage through physical emissions such as power consumption -- it offers a streamlined and user-friendly environment for managing complex experimental workflows.

TraceXpert integrates seamlessly with a variety of **measurement** and **communication** devices, including serial ports, smartcards, ChipWhisperer boards, and Picoscope 6-series oscilloscopes. It also supports arbitrary **protocol communication**, including parsing of pre-defined messages. Direct interaction with connected hardware components is possible through separate widgets. 

TraceXpert's modular architecture includes virtual **analytical devices** capable of generating power predictions, performing CPA on cryptographic algorithms such as AES, enumerating internal variables, and applying Welch's t-test for leakage detection. Data can be stored at any point in the workflow using TraceXpert's custom binary trace format, ensuring reproducibility and efficient storage.

A key feature of TraceXpert is its **scenario system**, which enables users to visually construct analysis pipelines through interconnected graphical blocks. This allows rapid prototyping and automation of measurement, acquisition, and analysis procedures without manual reconfiguration. More complex or unsupported operations may be further defined using Python scripts inside the scenario. 

TraceXpert 1.0 supports both Windows and Linux platforms and is distributed as open-source software, intended primarily for academic and laboratory research applications.

## Index

* Getting Started
* I/O Devices
    - [Protocols](protocols.md)
* Oscilloscopes
* Analytical devices
    - [Power predictions (CPA)](predictions.md)
* [Scenarios](scenarios.md)
* Build instructions

## Contributors (alphabetically)

Czech Technical University in Prague, Embedded Security Lab, https://crypto.fit.cvut.cz/

* Vojtěch Miškovský
* Martin Novotný
* David Pokorný
* Tomáš Přeučil
* Petr Socha
* Adam Švehla

## Acknowledgment

This work is supported by Ministry of the Interior of the Czech Republic under Grant VJ02010010.

