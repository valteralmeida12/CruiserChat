from llama_cpp import Llama

# Load your GGUF model
llm = Llama(model_path="../models/mistral-7b-v0.1.Q4_K_M.gguf")

# Prompt
prompt = "[INST] You are a concise, factual, direct and helpful assistant.\nWhat is the capital of Portugal? [/INST]"

output = llm(prompt, max_tokens=1000, echo=False, temperature=0.2)

print(output['choices'][0]['text'].strip())

