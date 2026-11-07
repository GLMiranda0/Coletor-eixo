// uso: node eda.mjs arquivo.js  -> executa o corpo da funcao no EasyEDA e imprime o resultado
import { readFileSync } from 'node:fs';
const code = readFileSync(process.argv[2], 'utf8');
const r = await fetch('http://127.0.0.1:49620/execute', {
  method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ code }),
});
const j = await r.json();
if (!j.success) { console.error('ERRO:', j.error); process.exit(1); }
console.log(typeof j.result === 'string' ? j.result : JSON.stringify(j.result, null, 1));
