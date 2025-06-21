#pragma once
#include <string>
#include <vector>
#include "../../llama.cpp/include/llama.h"


class LlamaTokenizer {
public:
    explicit LlamaTokenizer(const llama_model *model)
        : vocab_(llama_model_get_vocab(model)) {}

    // Tokenizes input text into llama_token vector.
    std::vector<llama_token> tokenize(const std::string &input, bool add_bos = true) const;

    // Converts tokens back into a string.
    std::string detokenize(const std::vector<llama_token> &tokens,
                           bool remove_special = true,
                           bool unparse_special = false) const;
                           
    // Returns the vocabulary used by this tokenizer.
    const llama_vocab* get_vocab() const {
        return vocab_;
    }

private:
    const llama_vocab *vocab_;
};

