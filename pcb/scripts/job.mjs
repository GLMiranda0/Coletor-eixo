// uso: node job.mjs arquivo.js -> roda o corpo como tarefa em segundo plano no EasyEDA e espera o fim (consulta a cada 3 s)
import { readFileSync } from 'node:fs';
const body = readFileSync(process.argv[2], 'utf8');
const post = async (code) => (await (await fetch('http://127.0.0.1:49620/execute', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ code }) })).json());
const start = `globalThis.__job = { done: false, log: [] }; const log = (m) => globalThis.__job.log.push(m);
(async () => { ${body} })().then(r => { globalThis.__job.result = r; globalThis.__job.done = true; }, e => { globalThis.__job.error = String(e && (e.stack || e.message) || e); globalThis.__job.done = true; });
return 'started';`;
let r = await post(start); if (!r.success) { console.error('ERRO ao iniciar:', r.error); process.exit(1); }
const t0 = Date.now();
while (true) {
  await new Promise(s => setTimeout(s, 3000));
  r = await post('const j = globalThis.__job; return j ? { done: j.done, n: j.log.length, last: j.log.slice(-1)[0] } : null;');
  if (!r.success) { console.error('consulta:', r.error); continue; }
  if (!r.result) { console.error('estado perdido entre chamadas'); process.exit(1); }
  if (r.result.done) break;
  process.stderr.write(`  ${((Date.now()-t0)/1000).toFixed(0)}s ${r.result.n} passos ${r.result.last ?? ''}\n`);
}
r = await post('const j = globalThis.__job; return { result: j.result, error: j.error, log: j.log };');
if (r.result.error) { console.error('ERRO:', r.result.error); console.log(r.result.log.join('\n')); process.exit(1); }
console.log(JSON.stringify(r.result.result, null, 1));
