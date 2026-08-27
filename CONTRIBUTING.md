# Contributing to ArrowPack

Thank you for considering contributing to ArrowPack!

---

## Ways of contributing
- **Reporting bugs**: To report a bug, open an issue detailing the bug. Check if the bug is already reported before opening. Include a clear title, details, steps to reproduce the bug, and your environment details.
- **Suggesting features/enhancements**: Open an issue explaining your proposed change or feature. Explain how it would work and how it would benefit ArrowPack.
- **Improving documentation**: Fixing typos, clarifying phrasing, and adding missing documentation/examples are great ways to improve the documentation of the project.
- **Writing code**: Finding an issue with the `Good first issue` or `Help wanted` tag to get started with writing code for arrowpack. Follow the pull request process below when contributing code to the project.
- **Writing tests**: Writing extra test cases helps to improve the reliability of the project and for future commits. When writing a test, make sure it is distinct and not too similar to an existing test. Use the pull request process below when contributing new test cases.

## Code contribution process
To write and submit your code for an issue, follow these steps:
### 1. **Fork/Clone the project**
Fork the project and clone it to your local machine.

### 2. Setup environment

#### Required components

- NPM
- Node.JS
- C compiler (GCC or Clang)
- Emscripten
- CMake
- Clang format

#### Install Node.js dependencies

Install Node dependencies by running `npm install`. This will install all of the Node. js-related dependencies for the JS side of the project.

#### Building

Once dependencies are installed, the project can be compiled in debug mode using:

```bash
npm run build
```

The code can also be compiled in release mode using the `build-release` npm command, this is not usually needed however unless you are testing release performance.

#### Testing

> [!NOTE]
> The formal testing setup is still in progress.

To run the tests, use:

```bash
npm run tests
```

### 3. **Create a branch**
Create a new branch for your feature/fix/test/etc using the following naming convention: 
`<type>/issue-<number>/<summary>`
Where `type` is one of the following:
- `feat`: A new feature/enhancement
- `fix`: A bug fix
- `docs`: Changes to only documentation
- `refactor`: A change that only refactors code without changing any functionality
- `test`: Adding/modifying tests
- `chore`: Miscellaneous changes made to the project itself, such as development tooling
- `build`: Changes to the build process

The `number` should be the issue number.

The `summary` should be around two or three words, briefly summarising the issue, e.g `css-parse-crash`.

An example of a full branch name: `fix/issue-87/css-parser-crash`.

### 4. Commit changes
Make your code changes and commit them. Preferably, use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) for your commit messages in the following format:
```
<type>(<scope>): <description>

<body>
```
#### Type
The type should be chosen according to the above branch typing guide.

#### Scope
The scope specifies which specific module/area has been modified. The scope should usually consist of `domain/sub-domain` such as `parser/css`.
The following scopes should be used for different parts of the project:
- `parser/language`
- `resolver/language`
- `cli`
- `test/area`, where `area` is a high-level module such as `parser`
- `bundler/language`

If the commit does not match any of these scopes, you should choose an appropriate scope yourself for the commit.

When your commit contains breaking changes, append an `!` after the type/scope, e.g: `feat(parser/css)! <description>`.

### 5. Open a pull request

Create a pull request on github for the branch. In the description for the pull request.
In the description of the pull request, write `Closes <issue number>` as well as a brief explanation of how the issue was solved or what design choices have been made.

### 6. Review
Your code will be reviewed by a maintainer. You may be requested to make some adjustments.
