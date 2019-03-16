# Minicomputer

[![Build Status](https://semaphoreci.com/api/v1/jpcima/minicomputer/branches/master/badge.svg)](https://semaphoreci.com/jpcima/minicomputer)

:fr: __Synthétiseur numérique pour Linux de qualité industrielle__

Ce programme est une version améliorée de [Minicomputer 1.41](http://minicomputer.sourceforge.net/).

- fonctionnement en processus et client unique
- pas de préconfiguration manuelle
- gestion du MIDI par interface JACK
- amélioration de la robustesse générale du code

[Page de documentation sur linuxmao.org](http://linuxmao.org/minicomputer)

## Étapes de construction

1. Installer les dépendances requises

- Debian : `git build-essential cmake libfltk1.3-dev libxft-dev libxinerama-dev libjack-jackd2-dev`

2. Récupérer le code source

- `git clone --recursive https://github.com/linuxmao-org/Minicomputer.git`

3. Configurer et compiler

- `mkdir Minicomputer/build`
- `cd Minicomputer/build`
- `cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..`
- `cmake --build .`

4. Installer (facultatif)

- `sudo cmake --build . --target install`

----

:us: __Industrial grade digital synthesizer for Linux__

This program is an improved fork of [Minicomputer 1.41](http://minicomputer.sourceforge.net/).

- single process, single client operation
- no manual setup required
- MIDI support through JACK interface
- improved general code robustness

[Documentation page on linuxmao.org](http://linuxmao.org/minicomputer) :fr: in French

## Build steps

1. Install required dependencies

- Debian : `git build-essential cmake libfltk1.3-dev libxft-dev libxinerama-dev libjack-jackd2-dev`

2. Retrieve source code

- `git clone --recursive https://github.com/linuxmao-org/Minicomputer.git`

3. Configure and compile

- `mkdir Minicomputer/build`
- `cd Minicomputer/build`
- `cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..`
- `cmake --build .`

4. Install (optional)

- `sudo cmake --build . --target install`
