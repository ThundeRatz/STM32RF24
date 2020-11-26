# Contributing to STM32RF24 🚀

Para a versão em PT-BR 🇧🇷 desse documento, [veja aqui](CONTRIBUTING.pt-br.md).

First of all, thanks for your interest! We really appreciate the participation of the community! ✨

Here we describe a set of guidelines when contributing to STM32RF24. These are not strict rules, so use your best judgment, and in case you have questions, contact us on comp@thunderatz.org.

Please note that this project is released with a [Contributor Code of Conduct](./CODE_OF_CONDUCT.md). By participating in this project you agree to abide by its terms.

## How can I contribute?

### Issues

Most contributions are made with [GitHub Issues](https://guides.github.com/features/issues/). They will be primarily used for

1. Tracking bugs - `bug`
2. Suggesting Enhancements - `enhancement`
3. Improving documentation - `documentation`

For each of these, there is a specific [label](https://docs.github.com/en/enterprise/2.17/user/github/managing-your-work-on-github/applying-labels-to-issues-and-pull-requests). We strongly recommend that every issue created has at least one of the three labels described above.

But before creating an issue, first check if there is already one with the same subject. You can search and filter by labels, for example, [here](https://github.com/ThundeRatz/STM32RF24/labels/bug) we only see the issues that are active and have the `bug` label.

#### Reporting Bugs 🐛

- Use a clear title
- Specify the version of the library
- Specify OS, VM (if applicable), project's microcontroller, and other configurations that may be useful
- Describe the steps to reproduce the bug
- Describe the observed and the expected behavior
- Include screenshots, gifs, any type of reference that helps to explain the problem

#### Suggesting Enhancements ✨

- Use a clear title
- Describe the suggestion step-by-step
- Describe the expected behavior after implementing this idea
- Explain why this new feature or update can be usefull

#### Improving documentation 📝

- Use a clear title
- Specify the files that need to be documented
- Explain your suggestion and why would it be better/clearer

### Pull Requests

If you want to contribute with code to the project, search for an open issue and start developing. When you are ready, open a Pull Request and we will review it.

A few recommendations:

- Describe exactly what you've done, always as clear as possible
- Link the applicable issue in your Pull Request (if there is none, please, create a new one)
- Make sure that you are following the [Syleguide 💄](#styleguide-)

## Styleguide 💄

### Code formatting

The code is formatted following specific rules, for more information on how to ensure that your code is formatted correctly, see the [🎨 Formatting](./README.md#-formatting) section on the README.

### Coding in C

There are some guidelines for organizing C code that should be followed.

> "Programs must be written for people to read,
> and only incidentally for machines to execute."

― Harold Abelson, Structure and Interpretation of Computer Programs

#### Variables and functions scopes

Always reduce the scopes of the variables to the minimum possible, avoiding global, external and static, giving preference to local variables. The same is true for functions, do not create public functions if they are only to be used within a file, leave them private.

#### Nomenclature

> "You should name a variable using the same care
> with which you name a first-born child."

― Robert C. Martin, Clean Code: A Handbook of Agile Software Craftsmanship

First, to maintain the standard, leave all the names of variables, functions, constants, files, among others, in English.

Choose meaningful names, the names should show what the variable, for example, is used in the code, so never use variables with only one letter, besides saying nothing about what it is, it is also difficult to find the variable in the code.

For variables or constants, for example, that have a unit of measure, include this unit of measure in the name, this makes it easier to understand its meaning.

In order to indicate that a variable is a pointer, add `p_` to the beginning of the variable name, if it is a pointer to a pointer, for example, add `pp_`.

Global variables in a file and external variables should be avoided, especially external ones, but if they are really necessary, they should be identified to facilitate the understanding of isolated parts of the code. Add `m_` to the beginning of a variable name to indicate that it is a global variable in a file. And to indicate that a variable is a global external, add `g_` at the beginning of its name. If you had an external pointer, for example, you would use `gp_`.

For more naming tips, see the article ["How To Create Meaningful Names In Code"](https://medium.com/better-programming/how-to-create-meaningful-names-in-code-20d7476537d4).

#### Variable types

Make coherent choices for your variable types and your functions return and parameters types. For examples, do not use `float` if your variable is a unit counter.

Besides that, for integer variables, distinguish between `signed` and `unsigned` and specify their size in bits, this ensures more control over the values that each variable can have, as well as optimizes the memory occupation. To do this, use the `stdint.h` library, than, for example, you can use the types `uint8_t` for unsigned 8-bit integers and `int16_t` for signed 16-bit integers.

#### Magic numbers/Constants

Avoid [magic numbers](https://en.wikipedia.org/wiki/Magic_number_(programming)) in your code. In addition to decreasing readability, they also make maintenance difficult. To solve this, use constants defined with `#define` at the beginning of your code.

### Git commit messages

- Use the present tense ("Add feature" not "Added feature")
- Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
- It is strongly recommended to start a commit message with a related emoji
  - 📝 `:memo:` for documentation
  - 🐛 `:bug:` for bug issues
  - 🚑 `:ambulance:` for critical fixes
  - 🎨 `:art:` for improving structure
  - ✨ `:sparkles:` for new features

  For more examples, see [this reference](https://gitmoji.carloscuesta.me/).

### Git workflow

The project workflow is based on [Git Flow](https://nvie.com/posts/a-successful-git-branching-model/).

### Documentation

The documentation is generated with [Doxygen](https://www.doxygen.nl/index.html) and should follow its documenting manual. A well-documented code is crucial for understanding it.
