# TeleMídia Multiplexer
This is a transport stream multiplexer provider.

## Main features
  * Multiple services
  * DSM-CC encoder
  * IIP and TMCC
  * UDP and file output
  * Multi-platform (Linux and Windows)

## Downloading

    $ git clone https://github.com/TeleMidia/tmm.git
    $ cd tmm
    $ git submodule init
    $ git submodule update

### Linux
You can compile it by entering in the tmm root directory and typing:

    $ mkdir build && cd build
    $ cmake ../
    $ make

### Windows
  * TODO

## Authors and contributors
  * Felippe Nagato <fnagato@telemidia.puc-rio.br>
  * Rafael Diniz <rafaeldiniz@telemidia.puc-rio.br>
  * Roberto Azevedo <robertogerson@telemidia.puc-rio.br>

## License

TeleMídia Multiplexer
Copyright (C) 2016-2018 TeleMídia Lab/PUC-Rio

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
for more details.
You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
