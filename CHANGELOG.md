# Changelog

Any features, notable changes and bug fixes will be found in the file.
Based of off [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)

# [0.2.2] - Unreleased

### Added

-   Config
    -   Allow config files with .cjs file extension

# [0.2.1] - 2023-04-01

### Fixed

-   Core
    -   Fixed major crash when no command line arguments were present

# [0.2.0] - 2023-03-23

### Added

-   Core
    -   Replaced yargs with an ultra-lightweight argument handler [Details](https://github.com/redstarbird/arrowpack/commit/a4bbd1d30fd28ad7d8c3fc45d4b5a375ed1e1e29)
    -   Allow an unlimited number of dependencies for dev mode rebuild [Details](https://github.com/redstarbird/arrowpack/commit/74e18c11e15d0a7c484893ef3a442a148b470c47)
    -   Track time taken to bundle files [Details](https://github.com/redstarbird/arrowpack/commit/12d66962ee9971af53184084577df63160e28569)
-   Javascript
    -   EMJS variable bundling
    -   Remove export token when bundled into another file
-   HTML
    -   Added global favicon inserts [Details](https://github.com/redstarbird/arrowpack/commit/8ace2ddc09a78d8cbc28d93ef009bda94828c3c7)
    -   Add base tag inserts [Details](https://github.com/redstarbird/arrowpack/commit/8ace2ddc09a78d8cbc28d93ef009bda94828c3c7)

### Fixed

-   Javascript
    -   EMJS alias names being detected incorrectly

# [0.1.0] - 2023-03-02

### Added

-   Core

    -   Improved file type handling [Details](https://github.com/redstarbird/arrowpack/commit/d57d5b47690fff276c8c511b9490843ea5a89bb9)
    -   Support relative paths using ../ [Details](https://github.com/redstarbird/arrowpack/commit/e51013459316e7c51c7d379944d1abc3ced814d1)
    -   More consistent are useful errors [Details](https://github.com/redstarbird/arrowpack/commit/4baa8790a976eed50c361e27169944552bbb50b1)
    -   Ignore dependencies inside of comments [Details](https://github.com/redstarbird/arrowpack/commit/d422d3d9fbea29c78fe8de2a3e34e6ab9b5f742e)
    -   Added live realoading development server [Details](https://github.com/redstarbird/arrowpack/commit/c426bbafe79df9c241845782c5c350c2b56201b8)

-   Config

    -   Option to bundle CSS in HTML [Details](https://github.com/redstarbird/arrowpack/commit/4eb6a186bda3b00d5944697e3040c0c3c1df06c2)

-   Dev Server

    -   Made reloading faster
    -   Maintain user scroll position when reloading

-   HTML
    -   Bundle CSS into HTML files
    -   Bundle inline Javascript [Details](https://github.com/redstarbird/arrowpack/commit/660844d6a6e3d46d5f27137dc24c99f2663fb2f2)
-   CSS
    -   CSS imports in CSS [Details](https://github.com/redstarbird/arrowpack/commit/4d9abd6078ce3dd97109b583f0de4b8b0d3e5f01)
-   Javascript
    -   CommonJS module bundling [Details](https://github.com/redstarbird/arrowpack/commit/c275b41be12fcbb10e3b25a4919dfdfdc713ce78)
    -   Maintain line numbers when bundling JS files in dev mode [Details](https://github.com/redstarbird/arrowpack/commit/7e1124de3df96bc9a8d3ab4635680796f6e74c4c)
    -   EM module bundling

### Fixed

-   Core

    -   Optimised dependency graph sorting [Details](https://github.com/redstarbird/arrowpack/commit/6f336adcf38b87de6394f06bcc59a3c664aff3aa)
    -   Turned dependency tree into graph structure [Details](https://github.com/redstarbird/arrowpack/commit/aa731c88b126e20c7f5d1481b9e737195094b7f8)
    -   Fixed bug with not creating enough space for dependency graph [Details](https://github.com/redstarbird/arrowpack/commit/0ab2fc5181a1870cdbab32d7e0147f667b62abe1)
    -   Fixed strings being shifted incorrectly when inserting previous string [Details](https://github.com/redstarbird/arrowpack/commit/9526c0e4200731cfa1f5b2f4015b9dacad567a48)
    -   Fixed exiting method when an error occurs [Details](https://github.com/redstarbird/arrowpack/commit/f8d6f8614058f3e3a354ddd3ce6cb216485c01ea)
    -   Fixed bug with detecting comments in files [Details](https://github.com/redstarbird/arrowpack/commit/48ecc54b796e51f909e3d664d0270a65dbf68ed4)
    -   Fixed bug with trying to bundle URL dependencies [Details](https://github.com/redstarbird/arrowpack/commit/63fe274925518e2e903cf2703c04bc7828ddbce7)
    -   Fixed bug with finding full path for relative dependencies [Details](https://github.com/redstarbird/arrowpack/commit/928a5ae78353876e793db400c58a31ded878b216)
    -   Support windows path seperators [Details](https://github.com/redstarbird/arrowpack/commit/00573c0acde9c4acdfc2a9380cde2e2a9b5ff516)

-   Javascript

    -   Remove name collisions from imported files [Details](https://github.com/redstarbird/arrowpack/commit/deb1f4acb7380c75e9a9726e5f88fa89372c5bee)

-   HTML
    -   Fixed dependencies being inserted in the wrong location [Details](https://github.com/redstarbird/arrowpack/commit/f70b97990e4059bc6923633fb1a7f267844b8840)
-   Dev Server
    -   Stopped page shifting when reloading
    -   Fixed dev server not finding the correct page when query parameters are present
    -   Fixed dev server not detecting script tags
