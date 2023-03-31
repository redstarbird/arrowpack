import { Validator } from "../../plugin/index.mjs";
export default new Validator(
    function (filecontents, filename) {
        const Presence = /<\\s*head\\s*>/.test(filecontents)
        this.AddResult(Presence);
        if (Presence === false) { this.AddWarning("Could not find head element for file " + filename); } // Colour and formatting could also be added to the text using modules such as chalk
        return this.Result; // Not required, but recommended to have
    }, "htmlHeadPresenceCheck"
)