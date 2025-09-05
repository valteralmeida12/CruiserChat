#ifndef CHATBOT_H
#define CHATBOT_H

#include <string>
#include <vector>
#include "common.h"
#include "llama.h"

class chatbot {
private:
    // custom deleters for llama.cpp types
    struct ModelDeleter {
        void operator()(llama_model* m) const { if (m) llama_free(m); }
    };
    struct ContextDeleter {
        void operator()(llama_context* c) const { if (c) llama_free(c); }
    };
    struct SamplerDeleter {
        void operator()(llama_sampler* s) const { if (s) llama_sampler_free(s); }
    };

    std::unique_ptr<llama_context> _ctx;
    std::unique_ptr<llama_model> _model;
    std::unique_ptr<llama_sampler> _sampler;

    llama_batch _batch;
    llama_token _currToken;

    //vector to store the chat messages
    std::vector<std::string> _messages;

public:
    chatbot(const std::string& model_path, float floatingP, float temperature);

    void addMessage(const std::string& message);

    std::string get_response(const std::string& input);
    
    std::string format_prompt();

    ~chatbot();
}

#endif