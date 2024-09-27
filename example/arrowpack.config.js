module.exports = {
	entry: "example/src",
	exit: "example/dist",
	autoClear: false,
	largeProject: false,
	bundleCSSInHTML: true,
	productionMode: false,
	addBaseTag: true,
	bundleJS: true,
	faviconPath: "favicon.ico",
	transformers: {
		"*.html": ["extensions/transformer.js"]
	},
	postProcessors: {
		//"*": ["./extensions/minifyHTML.js"]
	},
	resolvers: []
}
