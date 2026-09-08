# PixelForge AI

**PixelForge AI** é um editor profissional de sprites e animações de pixel art controlado por um agente autônomo de inteligência artificial construído em C++20 moderno.

A proposta fundamental do projeto:

> **«Você descreve o que quer. A IA planeja. O Sprite Engine desenha deterministicamente. A IA avalia o resultado e auto-corrige quando necessário.»**

A IA **não gera uma imagem diretamente** (não utiliza difusão ou redes neurais generativas visuais). Em vez disso, ela controla um **Sprite Engine nativo** através de chamadas a ferramentas estruturadas (*tool calling* / *function calling*).

O sistema opera **sem visão computacional**: o estado do sprite e dos frames é representado por dados estruturados e matrizes ASCII compactas que o modelo interpreta com alta fidelidade espacial e geométrica.

---

## ✨ Características

- 🧠 **Agente Autônomo com Autoavaliação:** Loop cognitivo com auto-crítica estruturada (detecção de ruído, violação de limites, excesso de cores e inconsistência de frames).
- 🛠️ **43 Ferramentas Estruturadas:** Primitivas de desenho geométrico, manipulação de cores, transformações e ferramentas semânticas de alto nível.
- 🏷️ **Regiões Semânticas:** Associação de partes do sprite por nome (`head`, `body`, `left_arm`, `weapon`, etc.), permitindo transformações espaciais como `move_region("left_leg", 2, 0)`.
- 🎨 **Paletas Controladas:** Suporte a presets retrô consagrados (Pico-8, GameBoy, DawnBringer 16/32, Endesga 32) e quantização automática de cores.
- 🎞️ **Animation Engine Multi-Frame:** Linha do tempo de frames com interpolação de regiões, comparação de consistência (*FrameDiffer*) e controle de duração em milissegundos.
- 🖼️ **Exportação Abrangente:** Sprites PNG individuais, sprite sheets organizadas em grade com metadados JSON e animações em GIF.
- 🤖 **Backend Intercambiável:**
  - `MockAgentBackend`: Planejador inteligente procedural/baseado em regras para testes, CI/CD e demonstrações offline (sem necessidade de API externa).
  - `HttpAgentBackend`: Compatível com qualquer modelo local via OpenAI/Ollama API (Gemma 4 E4B, Llama 3.2, Qwen 2.5, Mistral).
- ⚡ **C++20 de Alto Desempenho:** RAII, smart pointers, tipagem estrita, zero dependências externas pesadas (single-header vendored).

---

## 🏛️ Filosofia e Arquitetura

```
              ┌────────────────┐
              │    Usuário     │
              └───────┬────────┘
                      │ Prompt em Linguagem Natural
                      ↓
              ┌────────────────┐
              │    AI Agent    │◄─────────────────┐
              │  (IAgentLoop)  │                  │
              └───────┬────────┘                  │
                      │ Tool Calls (JSON)         │
                      ↓                           │
              ┌────────────────┐                  │
              │ Tool Registry  │                  │ Novo Estado /
              │  (Validação)   │                  │ Feedback Estruturado
              └───────┬────────┘                  │
                      │ Execução Determinística   │
                      ↓                           │
         ┌─────────────────────────┐              │
         │  Sprite/Animation Engine│──────────────┘
         │  - Pixels & Camadas     │
         │  - Regiões Semânticas   │
         │  - Paleta e Histórico   │
         └────────────┬────────────┘
                      │
                      ▼
               Exportação PNG/GIF
```

---

## 📁 Estrutura do Código

```
PixelForgeAI/
├── CMakeLists.txt              # Script de compilação CMake (C++20)
├── Makefile                    # Wrapper conveniente para build e testes
├── README.md                   # Documentação detalhada do projeto
├── LICENSE                     # Licença MIT
├── third_party/                # Dependências portáveis autocontidas
│   ├── nlohmann/json.hpp       # Manipulação de JSON para tool calling
│   ├── stb/                    # Leitura e escrita de PNG
│   └── gifenc/                 # Codificador de GIF animado em C
├── include/
│   ├── core/                   # Tipos primitivos (Point, Rect, Color, ValidationReport)
│   ├── sprite/                 # Sprite, Layer, Region, Palette, History, SpriteState
│   ├── tools/                  # ITool, ToolRegistry, PrimitiveTools, SemanticTools
│   ├── animation/              # Frame, Animation, FrameDiffer, SpriteSheet, AnimationTools
│   ├── io/                     # PngIO, GifIO, SpriteSheetIO
│   ├── agent/                  # IAgentBackend, MockAgentBackend, HttpAgentBackend, AgentLoop
│   └── cli/                    # CLI interativa e comandos
├── src/                        # Implementações (.cpp)
├── tests/                      # 33 testes unitários abrangentes
├── examples/                   # Exemplo de integração C++ como biblioteca
└── assets/                     # Sprites, animações e folhas de sprites geradas
```

---

## 💻 Compilação e Instalação

### Pré-requisitos
- Compilador C++ com suporte a **C++20** (GCC 11+, Clang 13+, MSVC 2022)
- **CMake** 3.20+ ou **Make**

### Compilando via CMake
```bash
cmake -B build
cmake --build build -j
```

### Compilando via Makefile
```bash
make
```

Os executáveis gerados estarão em:
- `build/pixel-forge` (aplicativo principal CLI / REPL)
- `build/pixel-forge-tests` (suite de testes unitários)
- `build/pixel-forge-example` (exemplo de embedding da biblioteca C++)

---

## 🧪 Executando os Testes

O Sprite Engine e todas as ferramentas podem ser testados de forma **100% offline e sem IA**:

```bash
make test
# ou:
./build/pixel-forge-tests
```

```
=======================================================
          PIXELFORGE AI - UNIT TEST SUITE             
=======================================================
[ RUN      ] SpriteTest.CreationAndDimensions
[       OK ] SpriteTest.CreationAndDimensions
...
[ RUN      ] AgentLoopTest.SlimeCreationEndToEnd
[       OK ] AgentLoopTest.SlimeCreationEndToEnd
[ RUN      ] AgentLoopTest.JumpAnimationEndToEnd
[       OK ] AgentLoopTest.JumpAnimationEndToEnd
-------------------------------------------------------
TOTAL: 33 | PASSED: 33 | FAILED: 0
=======================================================
```

---

## 🚀 Uso da Interface CLI

### 1. Modo Interativo (REPL)
Inicie sem argumentos para entrar na sessão iterativa com terminal colorido e pré-visualização ASCII:

```bash
./build/pixel-forge
```

```text
pixel-forge> Crie um slime verde 32x32 usando no máximo 8 cores.
pixel-forge> Faça esse slime pular em 6 frames.
pixel-forge> export slime_pulo.gif
pixel-forge> info
pixel-forge> undo
pixel-forge> exit
```

### 2. Comandos Diretos em Linha de Comando

**Criar novo sprite:**
```bash
./build/pixel-forge create 32 32 slime
```

**Gerar sprite por prompt:**
```bash
./build/pixel-forge prompt "Crie um cavaleiro medieval 32x32"
```

**Criar animação por prompt:**
```bash
./build/pixel-forge animate "Faça uma animação de pulo com 6 frames"
```

**Abrir sprite PNG existente e inspecionar:**
```bash
./build/pixel-forge open assets/knight.png
```

**Exportar arquivos:**
```bash
./build/pixel-forge export personagem.png
./build/pixel-forge export animacao.gif
./build/pixel-forge export folha_sheet.png
```

---

## 🤖 Integração com Modelos de IA Locais (Gemma, Llama, Qwen)

O PixelForge AI pode se conectar diretamente ao seu servidor de inferência local (Ollama, llama.cpp, vLLM ou LM Studio) sem alterar o Sprite Engine:

```bash
# Inicie seu modelo no Ollama (exemplo com Gemma):
ollama run gemma:4b

# Execute o PixelForge AI apontando para o endpoint local:
export PIXELFORGE_BACKEND=http
export PIXELFORGE_API_URL=http://localhost:11434/v1
export PIXELFORGE_MODEL=gemma:4b

./build/pixel-forge
```

---

## 🛠️ Catálogo de Ferramentas Estruturadas

### Ferramentas Primitivas
| Ferramenta | Descrição |
|---|---|
| `create_sprite` | Cria ou redefine o canvas do sprite com dimensões e cor de fundo. |
| `get_sprite_info` | Retorna resolução, paleta, caixas delimitadoras e matriz ASCII. |
| `get_pixel` / `set_pixel` / `clear_pixel` | Leitura e escrita atômica com coordenadas inteiras. |
| `draw_line` | Traçado com algoritmo de Bresenham. |
| `draw_rectangle` | Desenho de retângulos vazados ou preenchidos. |
| `draw_polygon` | Scanline rasterization para polígonos convexos ou côncavos. |
| `fill_region` | Flood fill (balde de tinta) 4-conectado. |
| `copy_region` / `move_region` / `erase_region` | Manipulação por deslocamento semântico de pixels. |
| `replace_color` | Substitui uma cor globalmente ou dentro de uma região delimitada. |
| `set_palette` / `add_color` / `remove_color` | Controle estrito de cores e paletas retro. |
| `flip_horizontal` / `flip_vertical` | Espelhamento no eixo horizontal ou vertical. |
| `undo` / `redo` | Desfazer e refazer transações no histórico do engine. |

### Ferramentas Semânticas de Alto Nível
| Ferramenta | Descrição |
|---|---|
| `create_outline` | Gera contorno limpo (4-way ou 8-way diagonal) ao redor do conteúdo. |
| `remove_outline` | Remove contorno periférico externo preservando a silhueta interna. |
| `add_shadow` | Projeta sombra direcional `(dx, dy)` nos pixels de fundo. |
| `add_highlight` | Adiciona brilho/iluminação nas bordas opostas da figura. |
| `quantize_palette` | Agrupa e reduz cores excedentes para o limite informado. |
| `symmetrize_region` | Torna regiões (rostos, escudos, corpos) simétricas ao longo de um eixo. |
| `clean_pixel_noise` | Elimina pixels órfãos (ruído isolado) sem vizinhos ortogonais. |
| `identify_color_clusters`| Agrupa blobs conectados e registra regiões semânticas automaticamente. |
| `create_region` / `rename_region` / `delete_region` | Gerenciamento de regiões semânticas. |

### Ferramentas de Animação
| Ferramenta | Descrição |
|---|---|
| `create_animation` | Inicia uma linha do tempo adicionando o sprite atual como Frame 0. |
| `add_frame` / `duplicate_frame` / `insert_frame` | Gerenciamento e duplicação de poses-chave. |
| `delete_frame` / `reorder_frame` | Reorganização de frames na linha do tempo. |
| `set_frame_duration` | Define a duração do frame em milissegundos. |
| `get_frame` / `update_frame` | Carrega e comita o frame ativo no workspace de edição. |
| `compare_frames` | Realiza diff estruturado entre frames (regiões alteradas/inalteradas). |
| `copy_region_between_frames` | Replica membros/acessórios entre frames consecutivos. |
| `interpolate_region` | Interpolação linear da posição de uma região entre keyframes. |

---

## 📜 Licença

Distribuído sob a licença [MIT](LICENSE).
