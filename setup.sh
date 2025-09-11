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

if [ ! -f "models/Phi-3-mini-4k-instruct-q4.gguf" ]; then
    echo "Downloading Phi-3 Mini model..."
    wget -O "models/Phi-3-mini-4k-instruct-q4.gguf" "https://huggingface.co/microsoft/Phi-3-mini-4k-instruct-gguf/resolve/main/Phi-3-mini-4k-instruct-q4.gguf"
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
echo "Model location: $(pwd)/../models/Phi-3-mini-4k-instruct-q4.gguf"
echo ""
echo "To run CruiserChat:"
echo "  cd build"
echo "  ./CruiserChat"
echo ""
echo "Or run with custom model:"
echo "  ./CruiserChat /path/to/your/model.gguf"
echo ""
echo "Type '>>exit' in the chat to quit."