#include "tokenizer.hpp"
#include <iostream>

// Converts an input text string into a vector of llama tokens using the model's vocabulary.
// Parameters:
// - model: pointer to the loaded llama_model from which to retrieve the vocabulary.
// - input: the input text string to tokenize.
// - add_bos: whether to add a Beginning-Of-Sequence token if the model supports it.
//
// Returns:
// - A vector of llama_token representing the tokenized input text.
// - Returns an empty vector if tokenization fails.

std::vector<llama_token> tokenize_text(llama_model *model, const std::string &input, bool add_bos) {
    // Get the vocabulary associated with the loaded model.
    const llama_vocab *vocab = llama_model_get_vocab(model);

    // Pre-allocate a vector to hold up to 4096 tokens (arbitrary max buffer size).
    // This avoids resizing during tokenization.
    std::vector<llama_token> tokens(4096);

    // Tokenize the input text using llama_tokenize.
    int n_tokens = llama_tokenize(vocab, input.c_str(), -1, tokens.data(), tokens.size(), add_bos, false);

    // Check if tokenization failed (negative return value) and returns an empty vector {}.
    if (n_tokens < 0) {
        std::cerr << "Tokenization failed\n";
        return {};  
    }

    // Resize the vector to the actual number of tokens produced.
    tokens.resize(n_tokens);

    // Return the token vector.
    return tokens;
}


// Converts a sequence of llama tokens back into a human-readable string.
// Parameters:
// - model: pointer to the loaded llama_model used for mapping tokens to strings.
// - tokens: a vector of llama_token that you want to convert back to text.
//
// Returns:
// - A string containing the detokenized text.
std::string detokenize_text(const llama_model *model,
                            const std::vector<llama_token> &tokens,
                            bool remove_special = true,
                            bool unparse_special = false) {
    // Get the vocabulary from the model
    const llama_vocab *vocab = llama_model_get_vocab(model);

    // Estimate a buffer size (this can be tuned)
    constexpr int32_t buffer_size = 8192;
    char buffer[buffer_size];

    // Call the llama detokenizer
    int32_t n_chars = llama_detokenize(
        vocab,
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