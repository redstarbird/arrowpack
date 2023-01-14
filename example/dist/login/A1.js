function MakeSnakeCase(str) {
    str = str.replace(/ /g, '_');

    // Replace uppercase characters with lowercase versions preceded by an underscore
    str = str.replace(/[A-Z]/g, function (char) {
        return '_' + char.toLowerCase();
    });

    return str;

}
let JSMakeSnakeCase_ARROWPACK = {};JSMakeSnakeCase_ARROWPACK = { SnakeCase: MakeSnakeCase };
			const SnakeCaseModule = JSMakeSnakeCase_ARROWPACK;

			function UpdateH1() {
				var H1Element = document.getElementById("main-title");
				H1Element.textContent = SnakeCaseModule.SnakeCase(H1Element.textContent);
			}
	