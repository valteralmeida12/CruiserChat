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
    cparams.n_ctx = 4096;
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

// Push to owned history
void chatbot::add_user(const std::string& text) {
     _history.emplace_back("user", text); 
}

void chatbot::add_assistant(const std::string& text) {
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
    
    std::string out;
    out.resize(8192);
    int n = llama_chat_apply_template(
        tmpl,
        msgs.data(), msgs.size(),
        append_assistant_prefix,
        out.data(), static_cast<int32_t>(out.size())
    );

    if (n < 0) {
        throw std::runtime_error("llama_chat_apply_template failed (buffer stage)");
    }

    if (n > static_cast<int>(out.size())) {
        out.resize(n);
        n = llama_chat_apply_template(
            tmpl,
            msgs.data(), msgs.size(),
            append_assistant_prefix,
            out.data(), static_cast<int32_t>(out.size())
        );
        if (n < 0){
            throw std::runtime_error("llama_chat_apply_template failed (resize stage)");
        }
    }
    out.resize(n);
    return out;
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

    if (n_tok < 0) throw std::runtime_error("tokenize failed");
    toks.resize(n_tok); 

    // 4) Feed prompt tokens safely, one token at a time
    for (int i = 0; i < n_tok; ++i) {
        
        llama_batch batch = llama_batch_init(1, 0, 1);
        
        // Set up arrays on stack
        llama_token token_array[1] = {toks[i]};
        llama_pos pos_array[1] = {_n_past};
        llama_seq_id seq_array[1] = {0};
        llama_seq_id* seq_ptrs_array[1] = {&seq_array[0]};
        int8_t logit_array[1] = {(i == n_tok - 1) ? (int8_t)1 : (int8_t)0};
        
        // Point batch to our stack arrays
        batch.n_tokens = 1;
        batch.token[0] = toks[i];
        batch.pos[0] = _n_past;
        batch.n_seq_id[0] = 1;        // Number of sequences for this token
        batch.seq_id[0][0] = 0;       // The actual sequence ID
        batch.logits[0] = (i == n_tok - 1) ? 1 : 0;

        if (llama_decode(_ctx.get(), batch) != 0) {
            llama_batch_free(batch);
            throw std::runtime_error("llama_decode failed at token " + std::to_string(i));
        }

        llama_batch_free(batch);
        _n_past++;
    }

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
            
            // Print the token as it's generated
            std::cout << token_text << std::flush;
        }

        
        llama_batch next = llama_batch_init(1, 0, 1);
        next.n_tokens = 1;
        next.token[0] = tok;
        next.pos[0] = _n_past;
        next.n_seq_id[0] = 1;        
        next.seq_id[0][0] = 0;       
        next.logits[0] = 1;          

        if (llama_decode(_ctx.get(), next) != 0) {
            llama_batch_free(next);
            break;
        }
        llama_batch_free(next);

        _n_past++;
    }

    add_assistant(reply);
    return reply;
}