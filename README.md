[Português](#pixelforge-ai) | [English](#pixelforge-ai-en)

---

# PixelForge AI

O PixelForge AI é um editor e animador de pixel art feito em C++20 que você controla conversando com uma inteligência artificial.

A maioria dos geradores de imagem atuais (como Midjourney ou DALL-E) tem dificuldade com pixel art porque eles tentam "adivinhar" a imagem inteira de uma vez. O resultado costuma ter pixels borrados, cores fora da paleta e personagens que mudam de forma a cada frame de uma animação.

O PixelForge AI funciona de um jeito diferente: ele usa o modelo de linguagem apenas como um **diretor de arte**. Você diz o que quer, a IA escolhe os comandos certos (desenhar uma linha, preencher uma cor, criar um contorno ou mover uma parte do corpo) e o motor em C++ desenha cada pixel no lugar exato da grade.

---

## Sumário

- [Como Funciona](#como-funciona)
- [Vantagens](#vantagens)
- [Economia de Contexto para Modelos Locais](#economia-de-contexto-para-modelos-locais)
- [Como a IA Enxerga o Desenho](#como-a-ia-enxerga-o-desenho)
- [Como Compilar e Rodar](#como-compilar-e-rodar)
- [Configurando o Modelo (pixelforge.json)](#configurando-o-modelo-pixelforgejson)
- [Exemplos de Uso](#exemplos-de-uso)
- [Lista de Ferramentas](#lista-de-ferramentas)
- [Estrutura de Pastas](#estrutura-de-pastas)
- [English Version](#pixelforge-ai-en)

---

## Como Funciona

1. **Você pede:** digite uma instrução simples, como *"crie um mago 32x32 com capa azul e cajado"*.
2. **A IA planeja:** o modelo escolhe quais ferramentas usar e com quais medidas (retângulos, linhas, cores e contornos).
3. **O motor desenha:** o código em C++ executa cada ação diretamente nos pixels, sem anti-aliasing e sem borrar nada.
4. **Auto-correção:** o sistema confere se há pixels isolados ou partes fora do desenho e a própria IA pode fazer pequenos ajustes.
5. **Você salva:** exporte o resultado final como imagem PNG, folha de sprites ou GIF animado.

---

## Vantagens

- **Pixels limpos e nítidos:** cada pixel é 100% sólido ou 100% transparente. Nada de borrões ou semi-transparências indesejadas.
- **Paletas de cores reais:** você pode usar paletas retrô consagradas (como Pico-8, GameBoy e DawnBringer) com limites reais de 8, 16 ou 32 cores.
- **Edições fáceis:** como partes do desenho podem receber nomes (`head`, `arm`, `weapon`), você pode pedir alterações pontuais como *"mude a cor da capa para vermelho"* ou *"mova a espada 2 pixels para cima"*.
- **Animações consistentes:** o personagem não muda de rosto nem se deforma entre os frames, pois os mesmos membros são reaproveitados e movimentados na linha do tempo.
- **Roda leve na CPU:** não exige placas de vídeo pesadas (VRAM) e funciona perfeitamente com modelos locais compactos (de 1B a 8B de parâmetros).

---

## Economia de Contexto para Modelos Locais

Se você rodar modelos locais no seu computador (via Jan, LM Studio ou Ollama), o tempo de resposta depende muito de quanto texto o modelo precisa ler a cada turno. Para deixar tudo mais rápido, o PixelForge AI inclui:

- **Filtro de ferramentas:** em vez de mandar as 43 ferramentas de uma vez só (o que consumiria mais de 3.000 tokens em cada mensagem), o programa escolhe apenas as 10 a 14 ferramentas necessárias para o que você pediu. Isso acelera o processamento em até 85%.
- **Histórico enxuto:** em conversas longas no terminal, os comandos de pixels antigos que já foram desenhados são resumidos. A IA se lembra do que você pediu, mas sem carregar centenas de linhas de histórico desnecessárias.
- **Respostas curtas:** quando uma ferramenta funciona sem erros, o programa devolve uma confirmação simples de uma linha, evitando gastar texto à toa.

---

## Como a IA Enxerga o Desenho

Como o motor não precisa de visão computacional nem de modelos pesados de imagem, ele mostra o estado do sprite para a IA usando texto simples e um mapa ASCII com coordenadas:

```text
=== SPRITE STATE ===
Resolucao: 16x16
Cores: 3 | Transparente: sim
BoundingBox: x=4..11 y=4..11 (tamanho 8x8)

Regioes:
  body: (4,4)-(11,11)
  eyes: (5,6)-(10,7)

Previa em ASCII:
    0    5    10   15
    0123456789012345
   +----------------+
04 |    .######.    |
05 |   .########.   |
06 |   .#@@##@@#.   |
07 |   .########.   |
08 |   .########.   |
09 |    .######.    |
   +----------------+
Legenda:
  . = Transparente
  # = #42B842 (corpo)
  @ = #FFFFFF (olhos)
```

---

## Como Compilar e Rodar

### O que você precisa
- Um compilador C++ moderno compatível com **C++20** (GCC 11+, Clang 13+ ou MSVC)
- **CMake** (versão 3.20 ou superior) ou **Make**

### Compilando pelo Make
```bash
git clone https://github.com/1g0rX/pixelforgeAI.git
cd pixelforgeAI
make -j$(nproc)
```

### Compilando pelo CMake
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

O executável principal será gerado em `build/pixel-forge` (e uma cópia é colocada na pasta raiz).

---

## Configurando o Modelo (pixelforge.json)

Você pode trocar o modelo ou o programa que roda a IA abrindo o arquivo [`pixelforge.json`](pixelforge.json). Não precisa recompilar o código para mudar as configurações:

```jsonc
{
  // Escolha qual perfil usar: "jan", "lmstudio", "ollama", "groq" ou "mock"
  "active_profile": "jan",

  "profiles": {
    "jan": {
      "type": "http",
      "api_url": "http://localhost:1337/v1",
      "model": "auto"           // "auto" detecta o modelo que ja esta aberto no Jan!
    },
    "lmstudio": {
      "type": "http",
      "api_url": "http://localhost:1234/v1",
      "model": "auto"           // "auto" detecta o modelo aberto no LM Studio!
    },
    "ollama": {
      "type": "http",
      "api_url": "http://localhost:11434/v1",
      "model": "llama3.2"
    },
    "mock": {
      "type": "mock",
      "model": "procedural-mock" // Funciona offline sem IA instalada (otimo para testar o motor)
    }
  }
}
```

---

## Exemplos de Uso

### Modo Conversa no Terminal (REPL)
Basta abrir o programa sem argumentos:
```bash
./build/pixel-forge
```

Você pode digitar pedidos normais:
```text
pixel-forge> Crie um pinguim 32x64 com uma panela na cabeca como chapeu
pixel-forge> Troque a cor do cachecol para vermelho escuro
pixel-forge> export pinguim.png
pixel-forge> exit
```

### Comandos Diretos no Terminal
```bash
# Criar uma tela vazia
./build/pixel-forge create 32 32 heroi

# Gerar sprite direto por texto
./build/pixel-forge prompt "Crie um cavaleiro medieval 32x32 com escudo"

# Gerar uma animacao
./build/pixel-forge animate "Crie um pulo em 6 quadros"

# Exportar nos formatos desejados
./build/pixel-forge export assets/heroi.png
./build/pixel-forge export assets/heroi_sheet.png
./build/pixel-forge export assets/heroi_animado.gif
```

---

## Lista de Ferramentas

O motor conta com ferramentas divididas em três categorias principais:

### 1. Desenho e Cores
- `create_sprite`: cria ou redefine o tamanho da tela.
- `set_pixel` / `clear_pixel`: pinta ou apaga um pixel em uma coordenada específica.
- `draw_line`: desenha uma linha reta entre dois pontos.
- `draw_rectangle`: cria retângulos cheios ou vazados.
- `draw_polygon`: desenha formas livres e polígonos.
- `fill_region`: balde de tinta para preencher áreas da mesma cor.
- `replace_color`: troca uma cor por outra em todo o desenho ou só em uma área.
- `set_palette`: aplica paletas prontas (Pico-8, GameBoy, DawnBringer).
- `flip_horizontal` / `flip_vertical`: espelha o desenho para a esquerda/direita ou cima/baixo.
- `undo` / `redo`: desfaz e refaz alterações.

### 2. Ferramentas Inteligentes
- `create_outline`: cria um contorno limpo ao redor do personagem.
- `add_shadow`: projeta uma sombra no chão ou no fundo.
- `add_highlight`: adiciona pontos de brilho e luz nas bordas.
- `clean_pixel_noise`: limpa pontinhos soltos que tenham ficado no desenho.
- `symmetrize_region`: deixa uma área (como um rosto ou escudo) perfeitamente simétrica.
- `create_region` / `move_region`: dá nome a partes do corpo e permite movê-las facilmente.

### 3. Animação
- `create_animation`: inicia uma animação com taxa de quadros (FPS) definida.
- `add_frame` / `duplicate_frame`: cria novos quadros ou duplica a pose anterior.
- `set_frame_duration`: define quanto tempo cada quadro fica na tela.
- `compare_frames`: confere o que mudou de um frame para o outro.
- `interpolate_region`: calcula o movimento suave de uma parte do corpo entre duas poses.

---

## Estrutura de Pastas

```
pixelforgeAI/
├── CMakeLists.txt        # Instrucoes de compilacao do CMake
├── Makefile              # Atalhos para compilar e limpar o projeto
├── README.md             # Este guia do projeto
├── LICENSE               # Licenca aberta MIT
├── pixelforge.json       # Configuracoes de modelos e servidores de IA
├── third_party/          # Bibliotecas auxiliares leves (JSON, PNG, GIF)
├── include/              # Cabecalhos e definicoes das classes (.hpp)
├── src/                  # Codigo-fonte do motor e ferramentas (.cpp)
└── assets/               # Sprites e imagens geradas
```

---

<a name="pixelforge-ai-en"></a>

# PixelForge AI (English)

PixelForge AI is a pixel art and animation tool written in C++20 that you control by chatting with an AI model.

Traditional image generation models (like Midjourney or DALL-E) often struggle with pixel art. They blur pixels, ignore fixed color palettes, and distort characters when making multi-frame animations.

PixelForge AI takes a different route: the AI acts as an **art director**. You describe what you want in plain words, the model selects structured drawing tools (lines, fills, outlines, palettes, region moves), and our native C++ engine renders each pixel onto the grid with mathematical accuracy.

---

## Why Tool Calling Over Image Diffusion?

- **Crisp pixels:** Every pixel is either fully solid or fully transparent. Zero unwanted anti-aliasing or blurry edges.
- **Strict palettes:** Supports authentic retro color palettes (Pico-8, GameBoy, DawnBringer) with exact 8, 16, or 32 color limits.
- **Targeted edits:** Named regions (`head`, `arm`, `weapon`) make it easy to request adjustments like *"change the cape to red"* or *"move the sword up by 2 pixels"*.
- **Consistent animations:** Characters do not morph between frames because the same body parts are manipulated across the timeline.
- **Runs on CPU:** No high-end GPU or VRAM needed; works smoothly with local models (from 1B to 8B parameters).

---

## Context Optimization for Local Models

Running local LLMs (via Jan, LM Studio, or Ollama) on consumer CPUs requires keeping prompts compact. PixelForge AI includes:

- **Smart Tool Selection:** Instead of sending all 43 tool descriptions every turn (~22 KB / ~3,400 tokens), the engine only passes the 10 to 14 tools relevant to your prompt. This cuts prompt evaluation time by up to 85%.
- **Sliding History Compaction:** In long interactive sessions, older pixel-by-pixel commands are summarized, keeping conversation memory light and preventing context exhaustion.
- **Compact Tool Feedback:** Successful operations return a single status line (`[SUCCESS] ...`), sending full validation reports only when errors or noise are found.

---

## Building and Running

### Requirements
- A modern C++ compiler supporting **C++20** (GCC 11+, Clang 13+, or MSVC)
- **CMake** (3.20+) or **Make**

### Build with Make
```bash
git clone https://github.com/1g0rX/pixelforgeAI.git
cd pixelforgeAI
make -j$(nproc)
```

### Build with CMake
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

The output binary is located at `build/pixel-forge` (and copied to the project root).

---

## Configuration (`pixelforge.json`)

You can switch models and providers without recompiling by editing [`pixelforge.json`](pixelforge.json):

```jsonc
{
  "active_profile": "jan",

  "profiles": {
    "jan": {
      "type": "http",
      "api_url": "http://localhost:1337/v1",
      "model": "auto"           // Automatically uses whichever model is active in Jan!
    },
    "lmstudio": {
      "type": "http",
      "api_url": "http://localhost:1234/v1",
      "model": "auto"           // Automatically uses the active model in LM Studio!
    },
    "ollama": {
      "type": "http",
      "api_url": "http://localhost:11434/v1",
      "model": "llama3.2"
    },
    "mock": {
      "type": "mock",
      "model": "procedural-mock" // Runs offline with zero external AI dependencies
    }
  }
}
```

---

## Usage Examples

### Interactive Terminal (REPL)
Launch the tool directly:
```bash
./build/pixel-forge
```

Type natural commands:
```text
pixel-forge> Create a 32x64 penguin wearing a cooking pot as a hat
pixel-forge> Change the scarf color to dark red
pixel-forge> export penguin.png
pixel-forge> exit
```

### Command Line Mode
```bash
# Create an empty sprite canvas
./build/pixel-forge create 32 32 hero

# Generate a sprite from prompt
./build/pixel-forge prompt "Create a medieval knight 32x32 with a sword"

# Generate an animation
./build/pixel-forge animate "Make a 6-frame jump animation"

# Export
./build/pixel-forge export assets/hero.png
./build/pixel-forge export assets/hero_sheet.png
./build/pixel-forge export assets/hero.gif
```

---

## License

Distributed under the [MIT License](LICENSE).
