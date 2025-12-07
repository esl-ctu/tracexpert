![TraceXpert logo](icons/tracexpert256.png)

# TraceXpert

**An integrated environment for side-channel data acquisition and analysis**

## User Guide and Contributors

[User Guide and Contributors can be found in docs/.](docs/README.md)

## Compilation

The project is written in C++/Qt6, compiles using CMake. 

Some plug-ins have their requirements (e.g., third-party libraries) to compile properly: Picoscope SDK (ps6000, ps6000a), MSVC compiler (smartcard), OpenMP (analytical plug-ins)...

On Windows, MSVC C++ compiler is used. 

The User Manual compilation requires Pandoc.

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
along with this program.  If not, see <https://www.gnu.org/licenses/>.

