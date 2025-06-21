#include "tokenizer/tokenizer.hpp"
#include <iostream>
#include <string>
#include <vector>

int main() {
    // Load model parameters and model
    llama_model_params model_params = llama_model_default_params();
    llama_model *model = llama_load_model_from_file("../models/mistral-7b-v0.1.Q4_K_M.gguf", model_params);
    if (!model) {
        std::cerr << "Failed to load model.\n";
        return 1;
    }
    std::cout << "Model loaded successfully.\n";

    // Initialize context
    llama_context_params ctx_params = llama_context_default_params();
    llama_context *ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        std::cerr << "Failed to initialize context.\n";
        llama_free_model(model);
        return 1;
    }
    std::cout << "Context initialized successfully.\n";

    // Initialize sampler
    llama_sampler_chain_params sampler_params = llama_sampler_chain_default_params();
    llama_sampler *sampler = llama_sampler_chain_init(sampler_params);
    if (!sampler) {
        std::cerr << "Failed to initialize sampler.\n";
        llama_free(ctx);
        llama_free_model(model);
        return 1;
    }
    std::cout << "Sampler initialized successfully.\n";

    // Tokenize input
    LlamaTokenizer tokenizer(model);
    std::string input = "How are you?";
    std::vector<llama_token> tokens = tokenizer.tokenize(input);

    if (tokens.empty()) {
        std::cerr << "Tokenization returned no tokens.\n";
        // Cleanup
        llama_sampler_free(sampler); 
        llama_free(ctx);
        llama_free_model(model);
        return 1;
    }
    std::cout << "Input tokenized: " << tokens.size() << " tokens.\n";

    // Decode initial input tokens
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (llama_decode(ctx, batch) != 0) {  
        std::cerr << "Initial decode failed.\n";
        // Cleanup
        llama_sampler_free(sampler);
        llama_free(ctx);
        llama_free_model(model);
        return 1;
    }

    std::string response;
    auto vocab = tokenizer.get_vocab();
    if (!vocab) {
        std::cerr << "Failed to get vocabulary from tokenizer.\n";
        llama_sampler_free(sampler);
        llama_free(ctx);
        llama_free_model(model);
        return 1;
    }

    // Sampling loop
    while (true) {
        llama_token token = llama_sampler_sample(sampler, ctx, -1);
        if (token < 0) {
            std::cerr << "Sampler returned invalid token\n";
            break;
        }
        
        if (llama_vocab_is_eog(vocab, token)) break;

        char buf[256];
        if (llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true) <= 0) {
            std::cerr << "Failed to convert token to piece.\n";
            break;
        }
        response += buf;

        batch = llama_batch_get_one(&token, 1);
        if (llama_decode(ctx, batch) != 0) {
            std::cerr << "Decode failed during sampling.\n";
            break;
        }
    }

    std::cout << "Model response: " << response << "\n";

    // Cleanup all allocated resources
    llama_sampler_free(sampler);  
    llama_free(ctx);       
    llama_free_model(model);

    return 0;
}
