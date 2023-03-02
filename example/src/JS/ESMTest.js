export default function TestESMImports() {
    console.log("Ecmascript imports are working");
}
export const GRAVITY = 9.8;
export function TestGRAVITY() {
    console.log("Gravity is: " + GRAVITY + "!");
    return GRAVITY === 9.8;
}