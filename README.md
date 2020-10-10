[![forthebadge](https://forthebadge.com/images/badges/made-with-c.svg)](https://forthebadge.com)
[![forthebadge](https://forthebadge.com/images/badges/powered-by-coders-sweat.svg)](https://forthebadge.com)


# 📡 STM32RF24

## 📜 Índice

- [🎉 Introdução](#-introdução)
- [➕ Requisitos](#-requisitos)
- [📦 Submódulos com Git](#-submódulos-com-git)
    - [💥 Adicionando um submódulo](#-adicionando-um-submódulo)
    - [✅ Inicializando um submódulo já existente](#-inicializando-um-submódulo-já-existente)
- [📁 Estrutura do Repositório](#-estrutura-do-repositório)
- [📝 Documentação](#-documentação)
- [🏗️ Estrutura do Código](#-estrutura-do-código)
- [🔌 Configuração de Hardware](#-configuração-de-hardware)
- [📚 Usando a biblioteca](#-usando-a-biblioteca)
- [📝 Contribuindo](#-contribuindo)
- [✨ Contribuidores](#-contribuidores)

## 🎉 Introdução

Esse repositório contém uma biblioteca para lidar com o módulo de rádiofrequência nRF24L01 da Nordic Semiconductor, cujo datasheet pode ser visto [aqui](docs/Nordic_Semiconductor-NRF24L01-datasheet.pdf).

Essa biblioteca foi feita para ser utilizada como submódulo no [STM32ProjectTemplate](https://github.com/ThundeRatz/STM32ProjectTemplate).


## ➕ Requisitos

Essa biblioteca não requer, para funcionar, nenhum requisito extra, além dos já listados [requisitos do STM32ProjectTemplate](https://github.com/ThundeRatz/STM32ProjectTemplate#requisitos).

Porém, caso se deseje gerar a documentação, como descrito na seção [📝 Documentação](#-documentação), é necessário instalar o Doxygen. No Ubuntu é possível instalá-lo com o seguinte comando:

```bash
sudo apt install doxygen
```

Para outros sistemas operacionais é possível ver opções de download na [página oficial do Doxygen](https://www.doxygen.nl/download.html).


## 📦 Submódulos com Git

Como dito, essa biblioteca funciona como um submódulo. Dessa forma se facilita escolher a versão da biblioteca que será utilizada no projeto, permitindo também o desenvolvimento em paralelo da biblioteca.

### 💥 Adicionando um submódulo

Crie um diretório chamado `lib`, caso não exista:

```bash
mkdir lib
```

E adicione o submódulo fazendo:

* Com HTTPS:
```bash
git submodule add --name STM32RF24 https://github.com/ThundeRatz/STM32RF24.git lib/STM32RF24
```

* Com SSH:
```bash
git submodule add --name STM32RF24 git@github.com:ThundeRatz/STM32RF24.git lib/STM32RF24
```

### ✅ Inicializando um submódulo já existente

Ao clonar um repositório que já tenha submódulos, é necessário clonar os repositórios desse submódulo. Isso pode ser feito de duas formas, clonando junto com o repositório do projeto ou depois de já ter clonado.

Exemplo:

Para se clonar junto, rode o seguinte comando, trocando para o link do repositório do seu projeto:

```bash
git clone --recurse-submodules git@github.com:ThundeRatz/STM32ProjectTemplate.git
```

Para se clonar depois de já ter clonado o repositório do projeto, dentro dele, deve-se fazer:

```bash
git submodule update --init
```


## 📁 Estrutura do Repositório

O repositório contém as seguintes pastas:

- **docs/** → Arquivos de documentação
- **inc/** → Arquivos de cabeçalho
- **src/** → Arquivos de código fonte

Na raiz do repositório, além dos arquivos que contém o código de conduta, diretrizes de contribuição, README e licensa, há o arquivo `sources.mk`, que é responsável por possibilitar com que os arquivos da biblioteca sejam encontrados quando se compila o código. Também há um `Doxyfile` para gerar a documentação.


## 📝 Documentação

Essa biblioteca é documentada usando Doxygen. Para gera a documentação, rode o seguinte comando estando na raiz do seu repositório:

```bash
doxygen
```

As configurações estão no arquivo [Doxyfile](./Doxyfile).


## 🏗️ Estrutura do Código

O código está estruturado da seguinte forma:

- `nrf24l01_registers.h` → tipos e constantes relacionados aos registradores do módulo.
- `rf24_platform.c/.h` → tipos e funções de mais baixo nível que utilizam o HAL.
- `rf24.c/.h` → tipos e funções de mais alto nível para utilização do usuário.
- `rf24_debug.c/.h` → funções úteis para se validar o funcionamento do módulo.


## 🔌 Configuração de Hardware


## 📚 Usando a biblioteca


## 📝 Contribuindo

Toda a ajuda no desenvolvimento da robótica é bem-vinda, nós lhe encorajamos a contribuir para o projeto! Para saber como fazer, veja as diretrizes de contribuição [aqui](CONTRIBUTING.pt-br.md).

## ✨ Contribuidores



---------------------

Equipe ThundeRatz de Robótica
