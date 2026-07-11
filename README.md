<div align="center">
  <img src="nomad-llm/nomad.svg" alt="Nomad Logo" width="120"/>
</div>
<div align="center">

**Nomad LLM** — Local AI Desktop Application

*Offline · Privat · Embedded C++ Inference*

</div>

---

## ✨ Features

### 🧠 Fully Embedded C++ Inference
Nomad LLM runs Large Language Models (LLMs) **directly in the application** using [llama.cpp](https://github.com/ggerganov/llama.cpp)

### 🔍 Smart Hardware Detection
Automatically detects your system's **RAM, CPU, GPU (NVIDIA/AMD/Apple Metal)**, and available disk space. Recommends the optimal model and context size for your hardware to prevent crashes and maximize performance.

### 🎮 Multi-GPU Support
- **NVIDIA CUDA** — Automatic GPU offloading for maximum speed
- **AMD ROCm / Vulkan** — Cross-vendor GPU acceleration
- **Apple Metal** — Native Apple Silicon acceleration
- **CPU-only** — Runs great on any machine with sufficient RAM

### 🤖 Built-in Model Catalog
Pre-configured models optimized for different tasks, downloadable directly from the GUI:

| Model | Type | Min RAM | Description |
|-------|------|---------|-------------|
| TinyLlama 1.1B | Testing | 2 GB | Ultra-fast testing model |
| Phi-3 Mini | Text (Light) | 4 GB | Efficient for low-spec devices |
| Llama 3 8B | General | 8 GB | Advanced conversational AI |
| Llava 1.5 7B | Vision+Text | 8 GB | Image + text understanding |
| Qwen 2.5 Coder 7B | Coding | 8 GB | Expert programming assistant |
| DeepSeek R1 7B | Reasoning | 8 GB | Chain-of-thought reasoning |
| Qwen 2.5 Coder 14B | Coding (Large) | 16 GB | Advanced software development |

### ☁️ Custom Model Download
Use the GUI input box to paste any Hugging Face Repo ID and filename to seamlessly download community models!

### 📄 Local RAG (Document Chat)
Upload TXT, MD, CSV, or JSON files — they're automatically chunked and indexed using **SQLite FTS5** and native **Dense Vector Embeddings** for highly accurate semantic search.

### 🖼️ Vision & Image Generation
- **Vision Models (Llava):** Built-in multimodal support allowing the AI to see and analyze images using CLIP projectors.
- **Stable Diffusion:** Generate images locally and entirely offline through our embedded Stable Diffusion engine.

### 🧠 Advanced Context & Memory Management
Intelligent memory management ensures efficient prompt caching and context window rotation, keeping long conversations fast and coherent.

### 🔒 Privacy First (Offline-First)
All inference and data processing happen entirely on your local machine. **Zero telemetry**, zero cloud dependencies. Your data never leaves your device.

### 🎙️ Offline Voice Support
Integrated with `whisper.cpp`, allowing fully offline voice interactions using local transcription. **Benefit:** Users can give voice commands to the AI assistant securely without internet connection or third-party tracking.

### 🛠️ Plugin & Scripting Sandbox
A secure, multi-language execution runtime built directly into the application core:
- **Python Scripting:** Embedded via `pybind11` to enable power users to create custom scripts and plugins safely within a sandboxed environment. **Benefit:** Power users and developers can customize their workflow and build powerful extensions directly into the application.
- **JavaScript Runtime:** Powered natively via Qt6 `QJSEngine` providing a secure-by-default execution environment without requiring external dependencies like Node.js. **Benefit:** Developers can safely run dynamic script logic in an isolated environment with zero direct access to the operating system or file system.
- **WebAssembly Sandbox:** Integrated via a lightweight `wasm3` interpreter to execute low-level binary code safely within a strict sandbox. **Benefit:** When the AI generates code in languages like C, Rust, or Go, that code can be compiled to WebAssembly and executed locally instantly—completely eliminating the risk of compromising the user's operating system.

### 🎨 GUI
Built with **Qt6/QML**:
- Clean, responsive Light Mode

- Smooth animations and transitions
- Responsive layout (sidebar auto-collapses on small screens)
- Settings drawer with visual sliders
- Live status bar (model, RAM, tokens/sec)

---

## 🚀 Getting Started

### Prerequisites
- **CMake** 3.16+
- **Qt6** (Core, Gui, Qml, Quick, Network, Sql, Widgets)
- **C++17** compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **Git** (for FetchContent)

> **Note**: llama.cpp is automatically downloaded and built by CMake. No manual setup needed.

### Build

```bash
# Clone
git clone https://github.com/ittoqs/nomad.llm.git
cd nomad.llm

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run
./build/nomad-llm/nomad_llm
```

### GPU Acceleration

GPU support is auto-detected during build:
- **NVIDIA**: Install CUDA Toolkit → CMake enables CUDA automatically
- **AMD/Intel**: Install Vulkan SDK → CMake enables Vulkan automatically
- **macOS**: Metal is enabled by default on Apple Silicon

---

## 🏗️ Architecture

```
nomad.llm/
├── CMakeLists.txt              # Root build
├── tests/                      # Automated QtTest suite
├── nomad-llm/
│   ├── CMakeLists.txt          # GUI build + llama.cpp FetchContent
│   ├── config.json             # API base URLs, system prompts, configurations
│   ├── src/
│   │   ├── main.cpp            # Application entry + component wiring
│   │   ├── inference_engine.*  # llama.cpp wrapper (model loading, generation)
│   │   ├── ILlmBackend.h       # Interface abstraction for LLM backend
│   │   ├── LlamaBackend.*      # llama.cpp specific backend implementation
│   │   ├── VisionBackend.*     # Vision/Llava model implementation with CLIP projector
│   │   ├── sd_engine.*         # Stable Diffusion image generation engine
│   │   ├── embedding_engine.*  # Native dense vector embedding computation
│   │   ├── memory_manager.*    # Chat history and context window management
│   │   ├── database_manager.*  # SQLite WAL + FTS5 (sessions, messages, RAG)
│   │   ├── model_manager.*     # HuggingFace model download + management
│   │   ├── settings_manager.*  # User prefs, i18n, config persistence
│   │   ├── hardware_detector.* # CPU/RAM/GPU/Disk detection
│   │   ├── document_processor.*# Document → FTS5/Embeddings indexing for RAG
│   │   ├── voice_manager.*     # whisper.cpp integration for offline voice
│   │   ├── script_engine.*     # pybind11 Python scripting integration
│   │   ├── js_engine.*         # Qt QJSEngine integration for JavaScript
│   │   └── wasm_engine.*       # wasm3 integration for WebAssembly
│   └── qml/
│       └── main.qml            # Complete GUI
```

**Key Design Decisions:**
- **Fully embedded** — All inference runs in-process via llama.cpp C API
- **SQLite FTS5** — Replaced Weaviate (~300MB RAM) with lightweight full-text search
- **Qt Signals/Slots** — Thread-safe communication between inference thread and GUI
- **Cross-platform** — Windows, macOS, Linux from a single codebase

---
