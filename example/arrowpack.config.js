module.exports = {
	entry: "example/src",
	exit: "example/dist",
	autoClear: false,
	largeProject: false,
	bundleCSSInHTML: true,
	productionMode: false,
	addBaseTag: true,
	faviconPath: "favicon.ico",
	transformers: {
		"*.html": ["extensions/transformer.js"]
	},
	validators: {
		"*.js": ["@arrowpack/jshint"]
	},
	postProcessors: {
		"*": ["./extensions/minifyHTML.js"]
	},
	resolvers: []
}
