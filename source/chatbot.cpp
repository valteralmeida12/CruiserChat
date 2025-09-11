#include "chatBot.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

chatbot::chatbot(const std::string& model_path, float temperature, float top_p)
: _temp(temperature), _top_p(top_p) {

    // Load model with GPU support
    llama_model_params mparams = llama_model_default_params();
    mparams.n_gpu_layers = 32;
    _model.reset(llama_model_load_from_file(model_path.c_str(), mparams));
    if (!_model) {
        throw std::runtime_error("Failed to load model: " + model_path);
    }

    _vocab = llama_model_get_vocab(_model.get());
    if (!_vocab) throw std::runtime_error("Failed to get vocab");

    // Context
    llama_context_params cparams = llama_context_default_params();
    cparams.type_k = GGML_TYPE_F16;
    cparams.type_v = GGML_TYPE_F16;
    cparams.n_ctx = 8192;
    _ctx.reset(llama_init_from_model(_model.get(), cparams));

    if (!_ctx) {
        throw std::runtime_error("Failed to init context");
    }

    // Sampler: chain(top_p, temp, seed-dist)
    llama_sampler_chain_params chain = llama_sampler_chain_default_params();
    _sampler.reset(llama_sampler_chain_init(chain));
    llama_sampler_chain_add(_sampler.get(), llama_sampler_init_top_p(_top_p, 1));
    llama_sampler_chain_add(_sampler.get(), llama_sampler_init_temp(_temp));
    llama_sampler_chain_add(_sampler.get(), llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
}

void chatbot::reset_context() {
    _n_past = 0;

    // Clear context memory
    llama_memory_clear(llama_get_memory(_ctx.get()), true);
}

// Push to owned history
void chatbot::add_user(const std::string& text) {
    if( _history.size() > max_messages * 2 ) {
        // Remove oldest pair (user+assistant)
        _history.erase(_history.begin(), _history.begin() + 2);
        // reset context
        reset_context();
    }

    _history.emplace_back("user", text);
}

void chatbot::add_assistant(const std::string& text) {
    if( _history.size() > max_messages * 2 ) {
        // Remove oldest pair (user+assistant)
        _history.erase(_history.begin(), _history.begin() + 2);
        // reset context
        reset_context();
    }

    _history.emplace_back("assistant", text);
}

// Build prompt using chat template
std::string chatbot::apply_chat_template(bool append_assistant_prefix) {

    std::vector<llama_chat_message> msgs;
    msgs.reserve(_history.size());
    
    for (auto& rc : _history) {
        msgs.push_back({ rc.first.c_str(), rc.second.c_str() });
    }
    
    const char* tmpl = llama_model_chat_template(_model.get(), nullptr);
    
    if (!tmpl) {
        std::string s;
        for (auto& rc : _history) {
            s += rc.first; s += ": "; s += rc.second; s += "\n";
        }
        if (append_assistant_prefix) s += "assistant: ";
        return s;
    }
    
    // Output buffer
    std::string outputBuffer;
    outputBuffer.resize(8192);
    int n = llama_chat_apply_template(
        tmpl,
        msgs.data(), msgs.size(),
        append_assistant_prefix,
        outputBuffer.data(), static_cast<int32_t>(outputBuffer.size())
    );

    if (n < 0) {
        throw std::runtime_error("llama_chat_apply_template failed (buffer stage)");
    }

    if (n > static_cast<int>(outputBuffer.size())) {
        outputBuffer.resize(n);
        n = llama_chat_apply_template(
            tmpl,
            msgs.data(), msgs.size(),
            append_assistant_prefix,
            outputBuffer.data(), static_cast<int32_t>(outputBuffer.size())
        );
        if (n < 0){
            throw std::runtime_error("llama_chat_apply_template failed (resize stage)");
        }
    }
    outputBuffer.resize(n);
    return outputBuffer;
}

// Generate response
std::string chatbot::get_response(const std::string& user_input) {
    // 1) Update history
    add_user(user_input);

    // 2) Build prompt with assistant prefix
    std::string prompt = apply_chat_template(true);
    
    // 3) Tokenize prompt
    std::vector<llama_token> toks(prompt.size() + 32);
    int32_t n_tok = llama_tokenize(
        _vocab,
        prompt.c_str(), static_cast<int32_t>(prompt.size()),
        toks.data(), static_cast<int32_t>(toks.size()),
        true, true
    );
        
    if (n_tok < 0) throw std::runtime_error("tokenize failed");
    toks.resize(n_tok);

    // 4) Feed prompt tokens in a single batch
    llama_batch batch = llama_batch_init(n_tok, 0, 1);
    
    for (int i = 0; i < n_tok; ++i) {
        batch.token[i] = toks[i];
        batch.pos[i] = _n_past + i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tok - 1) ? 1 : 0;
    }
    
    batch.n_tokens = n_tok;

    if (llama_decode(_ctx.get(), batch) != 0) {
        llama_batch_free(batch);
        throw std::runtime_error("llama_decode failed for prompt batch");
    }
    
    _n_past += n_tok;
    llama_batch_free(batch);

    // 5) Generate new tokens
    std::string reply;
    const int max_new_tokens = 512;

    for (int step = 0; step < max_new_tokens; ++step) {
        llama_token tok = llama_sampler_sample(_sampler.get(), _ctx.get(), -1);
        llama_sampler_accept(_sampler.get(), tok);

        if (llama_vocab_is_eog(_vocab, tok)) {
            break;
        }

        char piece[256];
        int np = llama_token_to_piece(_vocab, tok, piece, sizeof(piece), 0, true);
        if (np > 0) {
            std::string token_text(piece, np);
            reply += token_text;
            std::cout << token_text << std::flush;
        }

        // Create batch for the new token
        llama_batch next_batch = llama_batch_init(1, 0, 1);
        next_batch.n_tokens = 1;
        next_batch.token[0] = tok;
        next_batch.pos[0] = _n_past;
        next_batch.n_seq_id[0] = 1;
        next_batch.seq_id[0][0] = 0;
        next_batch.logits[0] = 1;

        if (llama_decode(_ctx.get(), next_batch) != 0) {
            llama_batch_free(next_batch);
            break;
        }
        
        _n_past++;
        llama_batch_free(next_batch);
    }

    add_assistant(reply);
    return reply;
}