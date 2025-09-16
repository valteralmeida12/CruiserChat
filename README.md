# 🚀 CruiserChat

**CruiserChat** is a C++ AI inference chat application that allows you to run Large Language Models (LLMs) locally and offline with GPU acceleration support.

## ✨ Features

- **Local & Offline**: Run LLMs completely on your own hardware
- **Multi-GPU Support**: CUDA (NVIDIA), METAL (Apple Silicon), and ROCm (AMD) acceleration
- **Cross-Platform**: Works on Linux, macOS, and other Unix-like systems
- **Easy Setup**: Automated setup scripts for different platforms

## 🛠️ Prerequisites

- **C++ Compiler** (GCC/Clang)
- **CMake** (3.10 or higher)
- **Git**
- **Homebrew** (macOS) or **APT** (Linux) package manager

## 📦 Installation

**Linux:**
```bash
./setup.sh
```

**MacOS:**
```bash
./mac-setup.sh
```

## Usage

**To run the app with the default model:**
```bash
cd build
./CruiserChat
```

**Chat help:**
```bash
./CruiserChat --help
```

**To run the app with a different model:**
```bash
./CruiserChat --model (or --m) <model-path>
```

**Exiting the chat:**
```bash
You: >>exit
```

## GPU acceleration

**🟩CUDA (NVIDIA)**
```bash
cd build
rm -rf *
cmake .. -DCUDA=ON -GNinja 
ninja
```

**🔴ROCM (AMD)**
```bash
cd build
rm -rf *
cmake .. -DROCM=ON -GNinja 
ninja
```

**💻 METAL (Apple Silicon)**
```bash
cd build
rm -rf *
cmake .. -DMETAL=ON -GNinja 
ninja
```
