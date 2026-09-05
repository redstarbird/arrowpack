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

### Dependency sorting

The dependency graph then needs to be used to find the order in which the dependencies have to be built. This is done by performing a DFS [topological sort](https://en.wikipedia.org/wiki/Topological_sorting) on the dependency graph to find the order in which the files are built. The exact order is not guaranteed to be the same on every run as unrelated files may be built in different orders.

### Chunking

- Create a new chunk for each entry point
- Use a DFS on the chunk's dependencies and add them to the chunk if they are synchronous dependencies
- Create a new chunk for async dependencies which are set as a dependency for the current chunk. This chunk is then pushed to a queue to be processed
- Deduplicate chunks by looking for modules that exist in lots of different chunks and splitting them into their own chunks

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
- A plugin hook is then called to allow plugins to work with and modify the final AST.

### Code generation

- Concatenated ASTs are then converted back into code
- A plugin hook is then called for plugins to modify the initially generated code
- Optimisers are then used on the final code to reduce bundle size or to increase the execution speed
- The chunks are written to files in the exit directory

## Plugin hooks
A variety of plugin hooks will be available to allow for custom logic to be run during the bundling process.
- `onResolve` is used for making custom resolvers that take a `source` string containing the path as written in the import statement and an `importer` string containing the path of the dependent file. Resolvers should return the absolute path for the dependency.
- `onLoad` is called when a dependency needs to be loaded from its absolute path. Plugins using this hook can register for specific file types and are provided a `absolutePath` containing the absolute path of the dependency. Loaders should return the content of the dependency.
- `onTransform` is used for transforming a module's source code before it is parsed into an AST. This is mainly used for transpiling languages such as TypeScript or Sass. Transform plugins take a `code` string containing the raw source code, and a `path` string containing the absolute file path. The transformed string should be returned.
- `onParse` is used for modify a module after it has been transformed but before it gets converted into an AST. Plugins are provided with a `code` string representing the transformed source code, and a `path` string containing the absolute file path of the module.
- `onModuleParsed` is called after the module has been parsed into an AST. An AST interface is provided to any attached plugins.
- `onConcatenate` is used to modify/read the combined AST of an entire chunk. These plugins are provided with an interface for the final AST.
- `onGenerated` is called when the final concatenated AST of a chunk has been generated into code. These plugins are provided with `code` and `path` parameters.
- `onOptimize` is strictly for optimising the final generated code for each chunk. Optimiser plugins are provided with the exit file path, `path`, and the final source code: `code`. These plugins run after the transformations on the final source code during the `onGenerated` hook so these should strictly be for final optimisations. 
