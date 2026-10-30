// Ubica las dependencias (playwright, esbuild) y la carpeta del proyecto, estén donde estén.
// Primero busca en node_modules del proyecto (npm install); si no, en las herramientas globales del entorno.
import { createRequire } from 'node:module';import fs from 'node:fs';import path from 'node:path';import { fileURLToPath } from 'node:url';
export const ROOT=path.resolve(path.dirname(fileURLToPath(import.meta.url)),'..');
export const OUT=path.join(ROOT,'.salidas');try{fs.mkdirSync(OUT,{recursive:true})}catch(e){}
export function need(name){for(const base of [ROOT+'/','/opt/npm-tools/',process.cwd()+'/']){try{return createRequire(base)(name)}catch(e){}}
 throw new Error('Falta «'+name+'»: ejecutá «npm install» en la carpeta del proyecto.')}
