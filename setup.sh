#!/bin/bash

# CruiserChat setup script

set -e

echo "Starting CruiserChat setup..."

# Update package lists
sudo apt-get update

# Install dependencies
sudo apt-get install cmake gcc -y ninja-build libreadline-dev

# Initialize and update git submodules
git submodule update --init --recursive

# Download default models if not present
if [ ! -d "models" ]; then
    echo "Creating models directory..."
    mkdir models
fi

if [ ! -f "models/Llama-3.2-3B-Instruct-Q6_K.gguf" ]; then
    echo "Downloading Llama 3.2 3B..."
    wget -O "models/Llama-3.2-3B-Instruct-Q6_K.gguf" "https://huggingface.co/bartowski/Llama-3.2-3B-Instruct-GGUF/resolve/main/Llama-3.2-3B-Instruct-Q6_K.gguf?download=true"
else
    echo "Model already exists, skipping download..."
fi

# Build app
mkdir -p build
cd build
cmake .. -GNinja
ninja

echo ""
echo " CruiserChat setup complete!"
echo ""
echo "Model location: $(pwd)/../models/Llama-3.2-3B-Instruct-Q8_0.gguf"
echo ""
echo "To run (./CruiserChat --help) to see how to run the application"
echo ""
echo "NOTE: The application was built without GPU support. For GPU support, please refer to the README.md"\n
echo "and choose the software compatible with your GPU."
echo ""