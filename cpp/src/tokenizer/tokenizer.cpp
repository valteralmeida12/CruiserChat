#include "tokenizer.hpp"
#include <iostream>


std::vector<llama_token> LlamaTokenizer::tokenize(const std::string &input, bool add_bos) const {
    std::vector<llama_token> tokens(4096); // arbitrary buffer size

    int lenght = static_cast<int>(input.size());
    
    int n_tokens = llama_tokenize(
        vocab_,
        input.c_str(),
        lenght,  // automatically infer text length
        tokens.data(),
        tokens.size(),
        add_bos,
        false  // parse_special = false
    );

    if (n_tokens < 0) {
        std::cerr << "Tokenization failed\n";
        return {};
    }

    tokens.resize(n_tokens);
    return tokens;
}

std::string LlamaTokenizer::detokenize(const std::vector<llama_token> &tokens,
                                       bool remove_special,
                                       bool unparse_special) const {
    constexpr int32_t buffer_size = 8192;
    char buffer[buffer_size];

    int32_t n_chars = llama_detokenize(
        vocab_,
        tokens.data(),
        static_cast<int32_t>(tokens.size()),
        buffer,
        buffer_size,
        remove_special,
        unparse_special
    );

    if (n_chars < 0) {
        std::cerr << "Detokenization failed\n";
        return {};
    }

    return std::string(buffer, n_chars);
}
