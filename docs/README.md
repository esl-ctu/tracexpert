# TraceXpert 1.0 User Guide

**An integrated environment for side-channel data acquisition and analysis**

## Introduction

TraceXpert 1.0 is a desktop application designed to facilitate both the measurement and statistical analysis of side-channel data. It provides a unified framework for conducting correlation power analysis (CPA), classification-based attacks, and leakage assessments, supporting researchers and practitioners in the field of hardware security evaluation. While the software assumes familiarity with the principles of side-channel analysis -- the study of information leakage through physical emissions such as power consumption -- it offers a streamlined and user-friendly environment for managing complex experimental workflows.

TraceXpert integrates seamlessly with a variety of **measurement** and **communication** devices, including serial ports, smartcards, ChipWhisperer boards, and Picoscope 6-series oscilloscopes. It also supports arbitrary **protocol communication**, including parsing of pre-defined messages. Direct interaction with connected hardware components is possible through separate widgets. 

TraceXpert's modular architecture includes virtual **analytical devices** capable of generating power predictions, performing CPA on cryptographic algorithms such as AES, enumerating internal variables, and applying Welch's t-test for leakage detection. Data can be stored at any point in the workflow using TraceXpert's custom binary trace format, ensuring reproducibility and efficient storage.

A key feature of TraceXpert is its **scenario system**, which enables users to visually construct analysis pipelines through interconnected graphical blocks. This allows rapid prototyping and automation of measurement, acquisition, and analysis procedures without manual reconfiguration. More complex or unsupported operations may be further defined using Python scripts inside the scenario. 

TraceXpert 1.0 supports both Windows and Linux platforms and is distributed as open-source software, intended primarily for academic and laboratory research applications.

## Index

* 1 Getting Started
* 2 Project Manager and Components
* 3 [I/O Devices](iodevices.md)
    - 3.1 [File](file.md)
    - 3.2 [Random number generator](rng.md)
    - 3.3 [Serial port](serialport.md)
    - 3.4 [Smart card](smartcard.md)
* 4 [Protocols](protocols.md)
* 5 [Oscilloscopes](oscilloscopes.md)
    - 5.1 NewAE (ChipWhisperer)
    - 5.2 [Picoscope 6000 A/B/C/D/E series](picoscope6000.md)
* 6 [Analytical Devices](analytical.md)
    - 6.1 [Ciphers](ciphers.md)
    - 6.2 [Leakage predictions](predictions.md)
    - 6.3 [Correlation power analysis](cpa.md)
    - 6.4 [Welch's t-test leakage assessment](ttest.md)
* 7 [Scenarios](scenarios.md)
* 8 Example use-case
    - 9.1 Correlation power analysis
    - 9.2 Non-specific t-test
    - 9.3 Specific t-test
* Build instructions

## Contributors (alphabetically)

Embedded Security Lab, CTU in Prague, https://crypto.fit.cvut.cz/

* Vojtech Miškovský
* Martin Novotný
* David Pokorný
* Tomáš Preucil
* Petr Socha
* Adam Švehla

## Acknowledgment

This work is supported by Ministry of the Interior of the Czech Republic under Grant VJ02010010 (2022-2025).

## License

TraceXpert

Copyright (C) 2022-present Embedded Security Lab, CTU in Prague, and contributors.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

![TraceXpert logo](images/tracexpert256.png)

