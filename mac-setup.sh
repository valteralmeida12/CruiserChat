#!/bin/zsh

# CruiserChat setup script

set -e

echo "Starting CruiserChat setup..."

# Update package lists
brew update

# Install dependencies
brew install cmake gcc readline ninja

# Initialize and update git submodules
git submodule update --init --recursive

# Download default models if not present
if [ ! -d "models" ]; then
    echo "Creating models directory..."
    mkdir models
fi

if [ ! -f "models/Llama-3.2-3B-Instruct-Q6_K.gguf" ]; then
    echo "Downloading Phi-3 Mini model..."
    curl -L -o "models/Llama-3.2-3B-Instruct-Q6_K.gguf" "https://huggingface.co/bartowski/Llama-3.2-3B-Instruct-GGUF/resolve/main/Llama-3.2-3B-Instruct-Q6_K.gguf?download=true"
    echo "Model already exists, skipping download..."
fi

# Build app
mkdir -p build
cd build
cmake .. -DMETAL=ON -GNinja
ninja

echo ""
echo " CruiserChat setup complete!"
echo ""
echo "Model location: $(pwd)/../models/Llama-3.2-3B-Instruct-Q6_K.gguf"
echo ""
echo "To run CruiserChat:"
echo "  cd build"
echo "  ./CruiserChat"
echo ""
echo "Or run with custom model:"
echo "  ./CruiserChat /path/to/your/model.gguf"
echo ""
echo "Type '>>exit' in the chat to quit."
```