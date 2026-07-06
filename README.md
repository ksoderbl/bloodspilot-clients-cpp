# Blood's Pilot Clients

This repository contains X11-based XPilot client variants, including:

- X11 client
- XPilot 4.5.4-style client
- Replay tool

The project uses **CMake only** (no autotools required).

---

## Requirements (Debian / Ubuntu)

Install the required development packages:

```bash
sudo apt update
sudo apt install git build-essential cmake \
    libx11-dev libxext-dev \
    zlib1g-dev
```

### Optional (sound support)

```bash
sudo apt install libopenal-dev libalut-dev
```

---

## Build

### Build all targets

```bash
./cmake-build.sh
```

This builds:
- `bloodspilot-client-x11`
- `bloodspilot-client-454`
- `bloodspilot-replay`

---

### Build only X11 client

```bash
./build-x11.sh
```

---

### Build only 4.5.4 client

```bash
./build-454.sh
```

---

## Run

Binaries are located under the build directory:

```bash
build-cmake/src/client/x11/bloodspilot-client-x11
build-cmake/src/client/454/bloodspilot-client-454
build-cmake/src/replay/bloodspilot-replay
```

---

## Install (optional)

```bash
sudo ./cmake-install.sh
```

Or manually:

```bash
cmake -S . -B build-cmake
cmake --build build-cmake
sudo cmake --install build-cmake
```

---

## Raspberry Pi Notes

On Raspberry Pi (Debian-based systems like Raspberry Pi OS), install:

```bash
sudo apt update
sudo apt install build-essential cmake \
    libx11-dev libxext-dev \
    zlib1g-dev
```

Optional:

```bash
sudo apt install libopenal-dev libalut-dev
```

---

## Notes

- Runtime data (textures, ship shapes) is loaded from the `share/` directory.
- Make sure you run binaries from the project root or install the data files.
- CMake builds are kept in separate directories (`build-cmake-*`) for flexibility.
- The project was migrated from autotools to CMake for simplicity and portability.
