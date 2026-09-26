# High-level architecture (WIP)

This is the new planned WIP arrowpack architecture.

## Bundling stages

### Dependency graph building

Generating the dependency graph is an iterative process that takes input files, transforms them, and finds their dependencies. This is done iteratively until the graph is fully built.
The dependency graph build process consists of the following steps:
- Find all of the entry/input files in the input directory according to the rules specified in `arrowpack.config.js`.
- Each file path will be found using a built-in or plugin-provided resolver.
- Read the file data for the current file from the local file system
- Transformers are run on the files based on their file type, if applicable. This will be either using a hardcoded transformer or a plugin transformer for the specific file type.
- An AST is generated from the transformed code, and it and its source map are stored for later
- Dependencies are then extracted from the AST
- The process is then recursively continued for any newly found dependencies until all transformations are complete and all dependencies have been resolved and added to the dependency graph.

#### Concurrency
All modules are independent to each other until the graph is fully resolved so multi-threading can be used to speed up the building the dependency graph. 

### Chunk creation/identification

A chunk is created for every explicitly defined entry point. In a project with an entry directory and no explicitly defined entry points, this will instead be done via using HTML files as entry points. 

During the dependency graph building stage, DFS is used on the chunk's dependencies and they are added to the chunk if they are synchronous dependencies. When a dynamic (async) import is found, a seperate chunk will be created for it and it will be pushed to a queue to process.

#### Code/chunk splitting
Deduplicate chunks by looking for modules that exist in lots of different chunks and splitting them into their own chunks. Each module stores a reference count to see how many different chunks depend upon it. Modules that are imported into multiple different distinct chunks.

### Dependency sorting

The dependency graph then needs to be used to find the order in which the dependencies have to be built. This is done by performing a DFS [topological sort](https://en.wikipedia.org/wiki/Topological_sorting) on the dependency graph to find the order in which the files are built. The exact order is not guaranteed to be the same on every run as unrelated files may be built in different orders.

### Combine chunk ASTs

The ASTs for every file/module in a chunk are combined into a single AST.

#### Name collision resolution

- To prevent naming collisions, a symbol dictionary is constructed which contains every global declaration for each module.
- A new name is generated for every symbol in the dictionary, which is the original name prepended with the name of the module.
- A check is performed to make sure that there are no naming collisions with any of the renamed modules. If any collisions are found, a hash will then be appended to the name. 
- All occurrences of the name are changed in all chunk ASTs to the new name. This is done carefully to ensure that shadowed local variables are not replaced.

#### AST Concatenation

- The ASTs for dependencies are then inserted into their dependent module AST. They are inserted at the top of the AST.
- Require/include statements, for modules that are part of the chunk, are then removed from the AST.
- Most of the chunk-wide code transformations happen here as it is the finalised AST form.

### Tree shaking

Tree shaking is performed on the final AST to reduce dead code.

### Code generation

- Concatenated ASTs are then converted back into code
- A plugin hook is then called for plugins to modify the initially generated code
- Optimisers are then used on the final code to reduce bundle size or to increase the execution speed
- The chunks are written to files in the exit directory

### Source map generation
Source maps for the concatenated modules are concatenated into a "final" source map.

## Plugin hooks
A variety of plugin hooks will be available to allow for custom logic to be run during the bundling process.
- `resolveId` is used for making custom resolvers that take a `source` string containing the path as written in the import statement and an `importer` string containing the path of the dependent file. Resolvers should return the absolute path for the dependency.
- `load` is called when a dependency needs to be loaded from its absolute path. Plugins using this hook can register for specific file types and are provided a `absolutePath` containing the absolute path of the dependency. Loaders should return the content of the dependency.
- `transform` is used for transforming a module's source code before it is parsed into an AST. This is mainly used for transpiling languages such as TypeScript or Sass. Transform plugins take a `code` string containing the raw source code, and a `path` string containing the absolute file path. The transformed string should be returned.
- `renderChunk` is called when the final concatenated AST of a chunk has been generated into code. These plugins are provided with `code` and `path` parameters.
- `generateBundle` is called just before the final generated code for each chunk is written to the disk. These plugins are provided with the exit file path, `path`, and the final source code: `code`. These plugins run after the transformations on the final source code during the `onGenerated` hook so these should ideally be for final optimisations or emitting new assets/files.

## JS Wrapper
JavaScript interacts with the compiled C/C++ code via the Node.JS Node-API.
### Arrowpack JS API
The `src/arrowpack.js` file contains the public JavaScript API for interacting with arrowpack. 
The main way to interact with arrowpack is via the `build` function, which can be provided with an arrowpack config object as well as optionally a list of entry files/modules for the bundler to use. This allows for other applications that use arrowpack internally to use the build function for a wide variety of uses such as general bundling, bundling specific files, bundling a specific file in a custom dev server.

#### Dev server
An arrowpack `DevServer` object can be created to allow the user to programmatically run a dev server that will cache the modules and dependency graph throughout its lifetime. The dev server can be used either in on-demand mode or watch mode. Watch mode will build the entire project initially and will watch for changes to any source files. When a change occurs, the modified file will be rebuilt so that the build directory always contains an up-to-date build while the dev server is running. On-demand mode will not build the project initially but will instead wait for the browser to request a route/file. When this route is requested, that file and its dependency tree will be built on demand and sent to the browser.

### CLI
The primary way to use arrowpack for bundling applications is via the command line. This is done by running the `arrowpack` command, which uses `src/cli/cli.js` as its entry point. `cli.js` is responsible for handling any CLI-specific functionality. An external, extremely lightweight arg library, [arrowargs](https://github.com/redstarbird/arrowargs), is used for managing and parsing the command line arguments.

#### CLI Parameters
- `-dev`: runs the development server. This keeps running until the user manually stops it using <kbd>Ctrl</kbd> + <kbd>C</kbd>.
- `-c`: used to specify a directory containing a config file rather than automatically using the current directory. Expects a string value after it representing the config directory.
- `-v`: prints the current version of arrowpack to the terminal and then stops the program.

### JS/C communication
Transferring data across the JS/C boundary is an extreme bottleneck and needs to be minimised and optimised as much as possible.
#### Optimisation techniques
- **Batching data**: Data is batched before being transferred from C to JS. This allows for a large amount of data to be transferred at once to minimise overhead.
- **Filtering plugins**: Plugins can provide a regex filter string that tells arrowpack what files they should run for. This can be computed on the C side so that JS plugins are only invoked if they are actually needed for a specific module/file.
