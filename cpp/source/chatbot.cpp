#include "chatBot.h"
#include <iostream>
#include <stdexcept>

ChatBot::ChatBot(const std::string& model_path, float floatingP, float temperature)
    : _model(nullptr), _ctx(nullptr), _sampler(nullptr) {

    // Load model
    llama_model_params params = llama_model_default_params();
    _model.reset(llama_model_load_from_file(model_path.c_str(), params));
    if (!_model) {
        throw std::runtime_error("Failed to load model from " + model_path);
    }

    // Create context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.f16_kv = floatingP > 0.0f;
    _ctx.reset(llama_new_context_with_model(_model.get(), ctx_params));
    if (!_ctx) {
        throw std::runtime_error("Failed to create llama context");
    }

    // Initialize sampler chain
    llama_sampler_chain_params sampler_params = llama_sampler_chain_default_params();
    _sampler.reset(llama_sampler_chain_init(sampler_params));
    if (!_sampler) {
        throw std::runtime_error("Failed to create sampler");
    }

    llama_sampler_chain_add(_sampler.get(), llama_sampler_init_min_p(floatingP, 1));
    llama_sampler_chain_add(_sampler.get(), llama_sampler_init_temp(temperature));
    llama_sampler_chain_add(_sampler.get(), llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
}

void ChatBot::addMessage(const std::string& message) {
    _messages.push_back(message);
}

std::string ChatBot::get_response(const std::string& input) {
    // TODO: Implement inference!!!
    return "";
}

ChatBot::~ChatBot() = default;
