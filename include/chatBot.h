#ifndef CHATBOT_H
#define CHATBOT_H

#include "llama.h"
#include <memory>
#include <string>
#include <vector>
#include <utility>

// RAII deleters (current API)
struct ModelDeleter {
    void operator()(llama_model* m) const { if (m) llama_model_free(m); }
};
struct ContextDeleter {
    void operator()(llama_context* c) const { if (c) llama_free(c); }
};
struct SamplerDeleter {
    void operator()(llama_sampler* s) const { if (s) llama_sampler_free(s); }
};

class chatbot {
public:
    chatbot(const std::string& model_path, float temperature = 0.7f, float top_p = 0.95f);
    ~chatbot() = default; // unique_ptr deleters handle cleanup
    
    std::string get_response(const std::string& user_input);
    
private:    
    std::string apply_chat_template(bool append_assistant_prefix);
    void add_user(const std::string& text);
    void add_assistant(const std::string& text);
    void reset_context();

    std::unique_ptr<llama_model,   ModelDeleter>   _model;
    std::unique_ptr<llama_context, ContextDeleter> _ctx;
    std::unique_ptr<llama_sampler, SamplerDeleter> _sampler;
    const llama_vocab* _vocab = nullptr;
    
    // {role, content}
    std::vector<std::pair<std::string,std::string>> _history; 
    const size_t max_messages = 2;

    float _temp  = 0.7f;
    float _top_p = 0.95f;
    int _n_past = 0;
};

#endif // CHATBOT_H
