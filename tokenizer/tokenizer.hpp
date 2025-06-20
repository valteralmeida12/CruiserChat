#pragma once
#include <string>
#include <vector>
#include "../llama.cpp/include/llama.h"

// Tokenizes input text using the provided model
std::vector<llama_token> tokenize_text(llama_model *model, const std::string &input, bool add_bos);

// Detokenizes tokens back into a string using the model's vocabulary
std::string detokenize_text(const llama_model *model,
                            const std::vector<llama_token> &tokens,
                            bool remove_special = true,
                            bool unparse_special = false);
