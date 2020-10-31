# Contribuindo para o STM32RF24 🚀

Primeiramente, obrigado pelo interesse! Nós agradecemos muito pela participação da comunidade nesse projeto! ✨

Esse documento descreve um conjunto de diretrizes para contribuir com esse projeto. Não são regras estritas, então sempre vale o bom senso, e, em caso de dúvidas, nos contate em comp@thunderatz.org.

Por favor, note que este projeto é liberado com um [Código de Conduta para Colaboradores](./CODE_OF_CONDUCT.pt-br.md). Ao participar deste projeto, você concorda em cumprir seus termos.

## Como eu posso contribuir?

### Issues

A maior parte das contribuições é feita por meio de [GitHub Issues](https://guides.github.com/features/issues/). Elas serão usadas principalmente para

1. Identificar bugs - `bug`
2. Sugerir melhorias e recursos - `enhancement`
3. Melhorar a documentação - `documentation`

Para cada um desses itens, existe um [label específico](https://docs.github.com/en/enterprise/2.17/user/github/managing-your-work-on-github/applying-labels-to-issues-and-pull-requests). Nós recomendamos fortemente que todas as issues criadas contenham pelo menos um desses labels descritos a cima.

Mas, antes de criar uma nova issue, é importante primeiro checar se já não existe uma com o mesmo assunto. Você pode filtrar por labels, por exemplo, [aqui](https://github.com/ThundeRatz/STM32RF24/labels/bug) serão mostradas apenas as issues ativas com o label `bug`.

#### Identificando bugs 🐛

- Use títulos claros
- Especifique a versão da biblioteca
- Especifique o SO, MV (se aplicável), o microcontrolador do projeto, e outras configurações que possam ser úteis
- Descreva os passos para reproduzir o bug encontrado
- Descreva o comportamento observado e o esperado
- Inclua screenshots, gifs e outros tipos de referências que podem ajudar a explicar o problema

#### Sugerir melhorias e recursos ✨

- Use títulos claros
- Descreva a sugestão passo-a-passo
- Descreva o comportamento esperado após implementar a ideia
- Explique por que esse novo recurso ou atualização pode ser útil

#### Melhorar a documentação 📝

- Use títulos claros
- Especifique os arquivos que precisam ser documentados
- Explique suas sugestões e por que seriam melhores ou mais claras

### Pull Requests

Se você quiser contribuir com código para o projeto, procure uma issue e comece a desenvolver sua solução! Quando você estiver pronto, abra uma Pull Request e nós vamos revisar.

Algumas recomendações:

- Descreva exatamente o que você fez e porque, sendo sempre o mais claro possível
- Adicione o link com a issue correspondente ao seu Pull Request (se não houver nenhuma, por favor crie uma nova)
- Confira se você está seguindo o [Syleguide 💄](#styleguide-)

## Styleguide 💄

### Formatação do código

O código é formatado seguindo regras específicas, para mais informações de como garantir que seu código esteja formatado da forma correta, veja a seção [🎨 Formatação](./README.pt-br.md#-formatação) do README.

### Código em C

Existem algumas diretrizes de organização de código em C que devem seguidas.

> "Programas precisam serem escritos para as pessoas lerem,
> e somente incidentalmente para as máquinas executarem."

― Harold Abelson, Structure and Interpretation of Computer Programs

#### Escopos

Reduza sempre os escopos das variáveis ao menor possível, evitando variáveis globais, externas e estáticas, dando preferência para variáveis locais. O mesmo é válido para funções, não crie funções públicas se elas so forem ser usadas dentro de um arquivo, deixe elas privadas.

#### Nomenclatura

> "Você deveria nomear uma variável com o mesmo cuidado
> com que nomearia uma criança recém-nascido."

― Robert C. Martin, Clean Code: A Handbook of Agile Software Craftsmanship

Primeiramente, para manter o padrão, deixe todos os nomes das variáveis, funções, constantes, arquivos, entre outros, em inglês.

Escolha nomes com significado, os nomes devem mostrar para que a variável, por exemplo, é utilizada no código, portanto, nunca utilize variáveis com somente uma letra, além de não dizer nada sobre o que ela é, também se dificulta a localização da variável no código.

Para variáveis ou constantes, por exemplo, que tenham unidade de medida, inclua essa unidade de medida no nome, isso facilita a compreensão do seu signifcado.

Para indicar que uma variável é um ponteiro, adicione `p_` no início do nome da variável, caso ela seja um ponteiro de um ponteiro, por exemplo, adcione `pp_`.

Variáveis globais de um arquivo e varáveis externas devem ser evitadas, principalmente as externas, porém caso sejam realmente necessárias, devem ser identificadas para se facilitar o entendimento de partes isoladas do código. Adicione `m_` no início do nome de uma variável para indicar que ela é uma variável global do arquivo. E para indicar que uma variável é global externa, adicione `g_` no início do nome da mesma. Caso se tivesse um ponteiro externo, por exemplo, se utilizaria `gp_`.

Para mais dicas de nomenclatura, veja o artigo ["How To Create Meaningful Names In Code"](https://medium.com/better-programming/how-to-create-meaningful-names-in-code-20d7476537d4).

#### Tipos de variáveis

Faça uma escolha coerente para os tipos das variáveis e tipos de retorno das funções, além de seus parâmetros, por exemplo, não utilize `float` se sua variável será um contador unitário.

Além disso, para variáveis inteiras faça distinção entre `signed` e `unsigned` e especifique seu tamanho em bits, isso garante mais controle sobre os valores que cada variável pode ter, como também otimiza a ocupação da memória. Para isso utilize a biblioteca `stdint.h`, podendo assim, por exemplo, se utilizar os tipos `uint8_t` para inteiros de 8 bits sem sinal e `int16_t` para inteiros de 16 bits com sinal.

#### Números mágicos/Constantes

Evite [números mágicos](https://pt.wikipedia.org/wiki/N%C3%BAmero_m%C3%A1gico_(programa%C3%A7%C3%A3o_de_sistemas)) no meio do código, além de eles obscurecerem o entendimento do mesmo, também dificultam a manutenção. Para contornar isso, utilize constantes definidas com `#define` no ínicio do seu código.

### Menssagens de commit

- As menssagens devem ser escritas preferencialmente em inglês.
- Use o tempo presente ("Add feature" não "Added feature")
- Use o modo imperativo ("Move cursor to..." não "Moves cursor to...")
- Nós recomendamos fortemente iniciar a menssagem de commit com um emoji relacionado
  - 📝 `:memo:` para documentação
  - 🐛 `:bug:` para bugs
  - 🚑 `:ambulance:` para correções críticas
  - 🎨 `:art:` para melhorias na estrutura
  - ✨ `:sparkles:` para novos recursos

  Para mais exemplos, veja [aqui](https://gitmoji.carloscuesta.me/).

### Fluxo de trabalho

O fluxo de trabalho é baseado no [Git Flow](https://nvie.com/posts/a-successful-git-branching-model/).

### Documentação

A documentação é gerada com [Doxygen](https://www.doxygen.nl/index.html) e deve seguir o seu manual de documentação. Um código bem documentado é crucial para seu entendimento.