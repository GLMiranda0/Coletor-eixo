#pragma once

// Interface web servida em http://NOME.local, com a identidade visual da plataforma Eixo
// (tokens, fontes e componentes de GLMiranda0/eixo-tcc, app/src/styles.css).
// As fontes vem do Google Fonts sem bloquear a pagina; sem internet, cai nas fontes do sistema.
const char WEBUI_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Coletor · Eixo</title>
<script>try{var t=localStorage.getItem('eixo-theme');if(t&&t!=='sys')document.documentElement.setAttribute('data-theme',t)}catch(e){}</script>
<link rel="stylesheet" media="print" onload="this.media='all'"
  href="https://fonts.googleapis.com/css2?family=Sora:wght@500;600;700;800&family=IBM+Plex+Sans:wght@400;500;600&family=IBM+Plex+Mono:wght@400;500&display=swap">
<style>
:root{
  --bg:#F3F6F9;--surface:#FFFFFF;--surface-2:#EAF0F5;--fg:#0F1A28;--muted:#5A6B7E;--border:#D7DFE8;
  --accent:#0891B2;--accent-2:#2563EB;--accent-ink:#FFFFFF;--accent-soft:#DDF4FA;--glow:rgba(8,145,178,.35);
  --ok:#15803D;--ok-soft:#DCFCE7;--warn:#B45309;--warn-soft:#FEF3C7;--crit:#B91C1C;--crit-soft:#FEE2E2;
  --s-temp:#EA580C;--s-corr:#0891B2;--s-vib:#DB2777;
  --grid:#E3E9F0;--shadow:0 10px 30px rgba(15,26,40,.08);
  --display:"Sora",system-ui,sans-serif;--body:"IBM Plex Sans",system-ui,sans-serif;--mono:"IBM Plex Mono",ui-monospace,Consolas,monospace;
  color-scheme:light;
}
:root[data-theme="dark"]{
  --bg:#0A1018;--surface:#111A25;--surface-2:#182434;--fg:#E7EEF5;--muted:#8C9DB2;--border:#223044;
  --accent:#22D3EE;--accent-2:#60A5FA;--accent-ink:#06121A;--accent-soft:#0E2F3A;--glow:rgba(34,211,238,.35);
  --ok:#4ADE80;--ok-soft:#0E2E1C;--warn:#FBBF24;--warn-soft:#3A2A05;--crit:#F87171;--crit-soft:#3E1414;
  --grid:#1C2838;--shadow:0 10px 30px rgba(0,0,0,.35);color-scheme:dark;
}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]):not([data-theme="dark"]){
  --bg:#0A1018;--surface:#111A25;--surface-2:#182434;--fg:#E7EEF5;--muted:#8C9DB2;--border:#223044;
  --accent:#22D3EE;--accent-2:#60A5FA;--accent-ink:#06121A;--accent-soft:#0E2F3A;--glow:rgba(34,211,238,.35);
  --ok:#4ADE80;--ok-soft:#0E2E1C;--warn:#FBBF24;--warn-soft:#3A2A05;--crit:#F87171;--crit-soft:#3E1414;
  --grid:#1C2838;--shadow:0 10px 30px rgba(0,0,0,.35);color-scheme:dark;}}

*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--fg);font-family:var(--body);font-size:14px;line-height:1.5;transition:background .3s,color .3s}
h1,h2,h3{font-family:var(--display);margin:0;letter-spacing:-.01em}
h1{font-size:28px;font-weight:700} h3{font-size:15px;font-weight:600}
p{margin:0} a{color:var(--accent)}
code,.mono{font-family:var(--mono)} .num{font-variant-numeric:tabular-nums} .muted{color:var(--muted)}
.small{font-size:12px}
.eyebrow{font-size:11px;letter-spacing:.12em;text-transform:uppercase;color:var(--muted);font-weight:600}
button{font:inherit;color:inherit;cursor:pointer}
input{font:inherit;color:var(--fg);background:var(--surface);border:1px solid var(--border);border-radius:8px;padding:9px 12px;width:100%}
input:focus,button:focus-visible{outline:2px solid var(--accent);outline-offset:2px}
input[type=range]{accent-color:var(--accent);padding:0;border:0;background:transparent}

/* botoes */
.btn{display:inline-flex;align-items:center;justify-content:center;gap:8px;border-radius:10px;padding:10px 16px;font-weight:600;border:1px solid transparent;transition:transform .15s,box-shadow .2s,background .2s,border-color .2s;position:relative;white-space:nowrap}
.btn:active{transform:translateY(1px)}
.btn-primary{background:linear-gradient(135deg,var(--accent),var(--accent-2));color:var(--accent-ink);box-shadow:0 6px 18px var(--glow);overflow:hidden}
.btn-primary:hover{transform:translateY(-1px);box-shadow:0 10px 26px var(--glow)}
.btn-primary::after{content:"";position:absolute;inset:0;background:linear-gradient(110deg,transparent 30%,rgba(255,255,255,.35) 50%,transparent 70%);transform:translateX(-120%);animation:shine 3.2s ease-in-out infinite}
.btn-secondary{background:var(--surface);border-color:var(--border);color:var(--fg)}
.btn-secondary:hover{border-color:var(--accent);color:var(--accent)}
.btn-ghost{background:transparent;color:var(--muted)} .btn-ghost:hover{background:var(--surface-2);color:var(--fg)}
.btn-danger{background:var(--crit-soft);color:var(--crit);border-color:color-mix(in srgb,var(--crit) 30%,transparent)}
.btn-sm{padding:6px 10px;font-size:12px;border-radius:8px}
.btn[disabled]{opacity:.5;cursor:not-allowed;transform:none}
@keyframes shine{0%,55%{transform:translateX(-120%)}85%,100%{transform:translateX(120%)}}
@keyframes blink{0%,100%{opacity:1}50%{opacity:.35}}
@keyframes spin{to{transform:rotate(360deg)}}
@keyframes pop{0%{transform:translateY(-8px) scale(.98);opacity:0}100%{transform:none;opacity:1}}
@media (prefers-reduced-motion:reduce){*{animation:none!important;transition:none!important}}

/* topo */
.top{display:flex;align-items:center;gap:12px;padding:12px 24px;background:var(--surface);border-bottom:1px solid var(--border);position:sticky;top:0;z-index:10}
.brand{display:flex;align-items:center;gap:10px}
.brand .word{font-family:var(--display);font-weight:800;font-size:20px;letter-spacing:-.03em}
.brand .tag{font-size:11px;letter-spacing:.12em;text-transform:uppercase;color:var(--muted);font-weight:600;border-left:1px solid var(--border);padding-left:10px}
.grow{flex:1;min-width:0}
.pill{display:inline-flex;align-items:center;gap:6px;border-radius:999px;padding:4px 10px;font-size:12px;font-weight:600;border:1px solid var(--border);background:var(--surface);white-space:nowrap}
.pill .dot{width:8px;height:8px;border-radius:50%;background:var(--muted)}
.pill.ok{background:var(--ok-soft);color:var(--ok);border-color:transparent} .pill.ok .dot{background:var(--ok);animation:blink 1.6s infinite}
.pill.warn{background:var(--warn-soft);color:var(--warn);border-color:transparent} .pill.warn .dot{background:var(--warn)}
.pill.crit{background:var(--crit-soft);color:var(--crit);border-color:transparent} .pill.crit .dot{background:var(--crit)}
.theme-toggle{display:inline-flex;border:1px solid var(--border);border-radius:999px;background:var(--surface);padding:3px}
.theme-toggle button{border:0;background:transparent;border-radius:999px;padding:5px 10px;font-size:12px;font-weight:600;color:var(--muted)}
.theme-toggle button.on{background:var(--accent);color:var(--accent-ink)}

/* pagina */
main{max-width:1080px;margin:0 auto;padding:20px 24px 48px;display:flex;flex-direction:column;gap:14px}
.topbar{display:flex;align-items:flex-end;gap:12px;flex-wrap:wrap;margin-bottom:6px}
.grid{display:grid;gap:14px}
.g2{grid-template-columns:repeat(2,minmax(0,1fr))} .g3{grid-template-columns:repeat(3,minmax(0,1fr))} .g4{grid-template-columns:repeat(4,minmax(0,1fr))}
.card{background:var(--surface);border:1px solid var(--border);border-radius:14px;padding:16px;min-width:0}
.card-h{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:12px;flex-wrap:wrap}
.sub{margin:14px 0 8px}
.tile{background:var(--surface-2);border-radius:10px;padding:10px 12px;min-width:0}
.tile .l{font-size:12px;color:var(--muted);font-weight:500}
.tile .v{font-family:var(--display);font-size:20px;font-weight:700;letter-spacing:-.02em}
.tile .u{font-size:12px;color:var(--muted);font-weight:500;margin-left:3px}
.tile.sm .v{font-size:16px}
.tile.err .v{color:var(--crit)}
.empty{padding:14px;text-align:center;color:var(--muted);border:1px dashed var(--border);border-radius:10px;font-size:13px}
.field{display:flex;flex-direction:column;gap:5px;margin-bottom:12px}
.field label{font-size:12px;font-weight:600;color:var(--muted)}
.hint{font-size:12px;color:var(--muted);line-height:1.4}
.row{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
.row .grow input{width:100%}
.actions{display:flex;gap:8px;flex-wrap:wrap;margin-top:4px}
.info{display:flex;justify-content:space-between;gap:12px;padding:9px 0;border-bottom:1px solid var(--border);font-size:13px}
.info:last-child{border-bottom:0}
.info .v{font-weight:600;text-align:right;word-break:break-all}
hr{border:0;border-top:1px solid var(--border);margin:14px 0}
.alpha-num{width:76px;text-align:center}
.spin{display:inline-block;width:13px;height:13px;border:2px solid var(--border);border-top-color:var(--accent);border-radius:50%;animation:spin .6s linear infinite}

/* gauges (mesmo desenho do Eixo) */
.gauge{position:relative;display:flex;flex-direction:column;align-items:center;gap:2px}
.gauge svg{width:100%;max-width:150px;height:auto}
.gauge .val{font-family:var(--display);font-size:22px;font-weight:700;margin-top:-30px}
.gauge .lbl{font-size:12px;color:var(--muted);font-weight:500}
.gauge .arc{transition:stroke-dashoffset .6s cubic-bezier(.2,.8,.2,1)}
.gauge .pill{margin-top:6px}

/* aviso */
.toast{position:fixed;right:16px;bottom:16px;z-index:50;background:var(--surface);border:1px solid var(--ok);border-left-width:5px;border-radius:12px;padding:12px 14px;box-shadow:var(--shadow);max-width:360px;font-weight:500;display:none}
.toast.show{display:block;animation:pop .35s ease-out}
.toast.err{border-color:var(--crit)}

@media (max-width:900px){.g4,.g3{grid-template-columns:repeat(2,minmax(0,1fr))}}
@media (max-width:640px){
  .top{padding:10px 16px;flex-wrap:wrap} main{padding:16px 16px 40px} h1{font-size:24px}
  .g2{grid-template-columns:1fr} .brand .tag{display:none}
}
.g3.gauges{grid-template-columns:repeat(3,minmax(0,1fr))}
@media (max-width:480px){.gauge .val{font-size:17px;margin-top:-24px}.gauge .lbl{font-size:11px;text-align:center}}
</style>
</head>
<body>

<header class="top">
  <div class="brand">
    <svg width="30" height="30" viewBox="0 0 64 64" aria-hidden="true">
      <defs><linearGradient id="eixo-lg" x1="0" y1="0" x2="1" y2="1"><stop offset="0" stop-color="#22D3EE"/><stop offset="1" stop-color="#2563EB"/></linearGradient></defs>
      <circle cx="32" cy="32" r="24" fill="none" stroke="currentColor" stroke-width="6" stroke-linecap="round" stroke-dasharray="118 40" transform="rotate(-210 32 32)"/>
      <path d="M12 34h10l4-10 6 20 5-14 3 4h12" fill="none" stroke="url(#eixo-lg)" stroke-width="4.5" stroke-linecap="round" stroke-linejoin="round"/>
      <circle cx="54" cy="12" r="5" fill="url(#eixo-lg)"/>
    </svg>
    <span class="word">eixo</span>
    <span class="tag">Coletor</span>
  </div>
  <div class="grow"></div>
  <span class="pill" id="post-pill"><span class="dot"></span><span id="post-lbl">Envio</span></span>
  <div class="theme-toggle" role="group" aria-label="Tema">
    <button type="button" data-t="light">Claro</button><button type="button" data-t="sys">Auto</button><button type="button" data-t="dark">Escuro</button>
  </div>
</header>

<main>
  <div class="topbar">
    <div class="grow">
      <p class="eyebrow">Coletor · <span id="ap-eb">&hellip;</span></p>
      <h1 id="title">Coletor</h1>
      <p class="muted" id="ip-lbl">Carregando&hellip;</p>
    </div>
    <button class="btn btn-secondary btn-sm" onclick="loadReadings();loadStatus()">&#8635; Atualizar</button>
  </div>

  <!-- Tempo real: o que vai para o Eixo -->
  <section class="card">
    <div class="card-h">
      <h3>Tempo real</h3>
      <span class="muted small" id="ts">Aguardando primeira leitura&hellip;</span>
    </div>
    <div class="grid g3 gauges">
      <div class="gauge" id="gauge-temp"></div>
      <div class="gauge" id="gauge-corr"></div>
      <div class="gauge" id="gauge-vib"></div>
    </div>
    <p class="hint" style="margin-top:12px">Valores como chegam ao Eixo: maior temperatura, m&eacute;dia das fases com leitura e velocidade RMS do eixo mais alto.
      Zona de vibra&ccedil;&atilde;o de refer&ecirc;ncia: ISO 10816-3, grupo 2, base r&iacute;gida.</p>
  </section>

  <div class="grid g2">
    <section class="card">
      <div class="card-h"><h3>Temperatura e corrente</h3></div>
      <p class="eyebrow">Temperatura &middot; DS18B20</p>
      <div class="grid g3 sub" id="temps"></div>
      <p class="eyebrow">Corrente &middot; SCT-013-005</p>
      <div class="grid g3 sub" id="amps"></div>
    </section>

    <section class="card">
      <div class="card-h">
        <h3>Vibra&ccedil;&atilde;o &middot; ADXL345</h3>
        <button class="btn btn-secondary btn-sm" id="btn-cal" onclick="calibrate()">Calibrar</button>
      </div>
      <p class="eyebrow">Velocidade RMS por eixo</p>
      <div class="grid g3 sub">
        <div class="tile"><div class="l">X</div><div><span class="v num" id="v-x">&ndash;</span><span class="u">mm/s</span></div></div>
        <div class="tile"><div class="l">Y</div><div><span class="v num" id="v-y">&ndash;</span><span class="u">mm/s</span></div></div>
        <div class="tile"><div class="l">Z</div><div><span class="v num" id="v-z">&ndash;</span><span class="u">mm/s</span></div></div>
      </div>
      <p class="eyebrow">Normalizado pela gravidade (repouso &asymp; 1,0)</p>
      <div class="grid g4 sub">
        <div class="tile sm"><div class="l">EMA</div><div class="v num" id="v-ema">&ndash;</div></div>
        <div class="tile sm"><div class="l">RMS</div><div class="v num" id="v-rms">&ndash;</div></div>
        <div class="tile sm"><div class="l">Pico</div><div class="v num" id="v-peak">&ndash;</div></div>
        <div class="tile sm"><div class="l">Desvio</div><div class="v num" id="v-std">&ndash;</div></div>
      </div>
      <p class="hint" id="vib-status">&nbsp;</p>
    </section>
  </div>

  <div class="grid g2">
    <section class="card">
      <div class="card-h"><h3>Destino &middot; Eixo</h3></div>
      <div class="field">
        <label for="post_url">Endpoint (POST)</label>
        <input type="url" id="post_url" class="mono" placeholder="https://PROJETO.supabase.co/rest/v1/rpc/ingest">
        <span class="hint">Copie da tela do motor, no Eixo. Vazio desliga o envio.</span>
      </div>
      <div class="field">
        <label for="device_token">Token do dispositivo</label>
        <input type="text" id="device_token" class="mono" autocomplete="off" placeholder="eixo_dev_...">
        <span class="hint">Gerado na tela do motor. Vai como <code>p_token</code>. Por seguran&ccedil;a, o coletor n&atilde;o mostra o token gravado.</span>
      </div>
      <div class="field">
        <label for="api_key">Chave public&aacute;vel do Supabase</label>
        <input type="text" id="api_key" class="mono" autocomplete="off" placeholder="sb_publishable_...">
        <span class="hint">Enviada no header <code>apikey</code>. &Eacute; a mesma <code>VITE_SUPABASE_KEY</code> do app.</span>
      </div>
      <div class="field">
        <label for="post_interval">Intervalo de envio (s)</label>
        <input type="number" id="post_interval" min="2" max="3600">
      </div>
      <div class="actions">
        <button class="btn btn-primary" onclick="saveDest()">Salvar destino</button>
        <button class="btn btn-secondary" onclick="postNow()">Enviar agora</button>
      </div>
      <hr>
      <div class="info"><span class="muted">&Uacute;ltimo envio</span><span class="v" id="post-info">&mdash;</span></div>
      <div class="info"><span class="muted">Fila de envio</span><span class="v" id="queue-info">&mdash;</span></div>
    </section>

    <section class="card">
      <div class="card-h"><h3>Rede e sistema</h3></div>
      <div class="field">
        <label for="name">Nome na rede</label>
        <div class="row">
          <div class="grow"><input type="text" id="name" maxlength="15" autocomplete="off"></div>
          <button class="btn btn-secondary" onclick="saveName()">Salvar</button>
        </div>
        <span class="hint">Letras, n&uacute;meros e h&iacute;fen. Com mais de um coletor na rede, use nomes diferentes. Reinicia ao salvar.</span>
      </div>
      <div class="info"><span class="muted">Rede conectada</span><span class="v" id="ssid-lbl">&mdash;</span></div>
      <div class="info"><span class="muted">Endere&ccedil;o IP (DHCP)</span><span class="v mono" id="ip-lbl2">&mdash;</span></div>
      <div class="info"><span class="muted">Acesso por nome</span><a class="v mono" id="mdns-lbl" href="#" target="_blank">&mdash;</a></div>
      <div class="info"><span class="muted">Rede de configura&ccedil;&atilde;o (AP)</span><span class="v mono" id="ap-lbl">&mdash;</span></div>
      <div class="actions" style="margin-top:14px">
        <button class="btn btn-secondary" onclick="resetWifi()">Redefinir Wi-Fi</button>
        <button class="btn btn-danger" onclick="restartDevice()">Reiniciar</button>
      </div>
    </section>
  </div>

  <section class="card">
    <div class="card-h"><h3>Coleta de vibra&ccedil;&atilde;o</h3></div>
    <div class="grid g3">
      <div class="field"><label for="batch_size">Amostras por janela</label><input type="number" id="batch_size" min="10" max="2000"></div>
      <div class="field"><label for="sample_delay">Intervalo entre amostras (ms)</label><input type="number" id="sample_delay" min="1" max="1000"></div>
      <div class="field">
        <label for="alpha">K do EMA (&alpha;)</label>
        <div class="row">
          <div class="grow"><input type="range" id="alpha_r" min="0.01" max="1" step="0.01" oninput="syncAlpha(this.value)"></div>
          <input type="number" id="alpha" class="alpha-num" min="0.01" max="1" step="0.01" oninput="syncAlpha(this.value)">
        </div>
        <span class="hint">Menor = mais suave &middot; maior = mais reativo</span>
      </div>
    </div>
    <div class="actions"><button class="btn btn-primary" onclick="saveSensor()">Salvar coleta</button></div>
  </section>
</main>

<div class="toast" id="toast" role="status"></div>

<script>
const $ = id => document.getElementById(id);

// ── Tema (mesmo seletor do Eixo) ──
function setTheme(t) {
  if (t === 'sys') document.documentElement.removeAttribute('data-theme');
  else document.documentElement.setAttribute('data-theme', t);
  try { localStorage.setItem('eixo-theme', t); } catch(e) {}
  document.querySelectorAll('.theme-toggle button').forEach(b => b.classList.toggle('on', b.dataset.t === t));
}
document.querySelectorAll('.theme-toggle button').forEach(b => b.onclick = () => setTheme(b.dataset.t));
setTheme(document.documentElement.getAttribute('data-theme') || 'sys');

// ── Gauges: faixa do sensor, cores das grandezas do Eixo ──
const G = {
  temp: {nome: 'Temperatura', un: '°C',   cor: 'var(--s-temp)', min: 20, max: 100, casas: 1},
  corr: {nome: 'Corrente',    un: 'A',    cor: 'var(--s-corr)', min: 0,  max: 5,   casas: 2},
  vib:  {nome: 'Vibração',    un: 'mm/s', cor: 'var(--s-vib)',  min: 0,  max: 8,   casas: 2},
};
const ARC = 'M12 68 A48 48 0 1 1 108 68';
Object.entries(G).forEach(([k, m]) => {
  $('gauge-' + k).innerHTML =
    '<svg viewBox="0 0 120 80"><path d="' + ARC + '" fill="none" stroke="var(--grid)" stroke-width="10" stroke-linecap="round"/>' +
    '<path class="arc" id="arc-' + k + '" d="' + ARC + '" fill="none" stroke="' + m.cor + '" stroke-width="10" stroke-linecap="round" stroke-dasharray="226" stroke-dashoffset="226"/></svg>' +
    '<span class="val num" id="g-' + k + '">–</span><span class="lbl">' + m.nome + ' (' + m.un + ')</span>' +
    (k === 'vib' ? '<span class="pill" id="zona"><span class="dot"></span><span id="zona-lbl">sem leitura</span></span>' : '');
});

const fmt = (v, casas) => v == null ? '–' : v.toLocaleString('pt-BR', {minimumFractionDigits: casas, maximumFractionDigits: casas});

function setGauge(k, v) {
  const m = G[k];
  const p = v == null ? 0 : Math.min(1, Math.max(0, (v - m.min) / (m.max - m.min)));
  $('arc-' + k).style.strokeDashoffset = 226 - 226 * p;
  $('g-' + k).textContent = fmt(v, m.casas);
}

// ISO 10816-3, grupo 2 (15 a 300 kW), base rigida: A/B 1,4 · B/C 2,8 · C/D 4,5 mm/s
function setZona(v) {
  let cls = '', txt = 'sem leitura';
  if (v != null) {
    if (v < 1.4)      { cls = 'ok';   txt = 'Zona A'; }
    else if (v < 2.8) { cls = 'ok';   txt = 'Zona B'; }
    else if (v < 4.5) { cls = 'warn'; txt = 'Zona C · atenção'; }
    else              { cls = 'crit'; txt = 'Zona D · crítico'; }
  }
  $('zona').className = 'pill ' + cls;
  $('zona-lbl').textContent = txt;
  $('g-vib').style.color = cls === 'crit' ? 'var(--crit)' : cls === 'warn' ? 'var(--warn)' : '';
}

function tile(label, val, unit, err) {
  return '<div class="tile' + (err ? ' err' : '') + '"><div class="l">' + label + '</div><div><span class="v num">' + val +
         '</span>' + (unit ? '<span class="u">' + unit + '</span>' : '') + '</div></div>';
}

// ── API ──
async function api(path, opts) {
  const r = await fetch(path, opts);
  if (!r.ok) {
    const t = await r.text();
    let msg = t || r.status;
    try { msg = JSON.parse(t).error || msg; } catch(e) {}
    throw new Error(msg);
  }
  return r.json();
}

function post(path, body) {
  return api(path, {method: 'POST', headers: {'Content-Type': 'application/json'},
    body: body ? JSON.stringify(body) : undefined});
}

let toastTimer;
function toast(msg, type = 'ok') {
  const el = $('toast');
  el.textContent = msg;
  el.className = 'toast show ' + type;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.className = 'toast', 3200);
}

function syncAlpha(v) {
  v = Math.min(1, Math.max(0.01, parseFloat(v) || 0.01));
  $('alpha').value = v.toFixed(2);
  $('alpha_r').value = v;
}

// ── Leituras ──
async function loadReadings() {
  try {
    const d = await api('/api/readings');
    const e = d.eixo || {};
    setGauge('temp', e.temp);
    setGauge('corr', e.corr);
    setGauge('vib', e.vib);
    setZona(e.vib);

    $('temps').innerHTML = d.temp_c.length
      ? d.temp_c.map((t, i) => t === null ? tile('Sensor ' + i, 'erro', '', true) : tile('Sensor ' + i, fmt(t, 1), '°C')).join('')
      : '<div class="empty" style="grid-column:1/-1">Nenhum sensor encontrado</div>';
    $('amps').innerHTML = Object.entries(d.current_a).map(([k, a]) => tile(k, fmt(a, 2), 'A')).join('');

    if (d.vib) {
      ['x', 'y', 'z'].forEach((k, i) => $('v-' + k).textContent = fmt(d.vib.vel_axes[i], 2));
      ['ema', 'rms', 'peak', 'std'].forEach(k => $('v-' + k).textContent = fmt(d.vib[k], k === 'std' ? 4 : 3));
    }
    $('ts').textContent = d.ts ? 'Atualizado às ' + new Date(d.ts).toLocaleTimeString('pt-BR')
                               : 'Atualizado (relógio ainda não sincronizado)';
  } catch(e) {
    $('ts').textContent = 'Sem dados ainda';
  }
}

async function loadStatus() {
  try {
    const s = await api('/api/status');
    $('title').textContent = s.hostname;
    $('ap-eb').textContent = s.ap;
    $('ip-lbl').textContent = s.ip + ' · ' + s.ssid + ' · ' + s.rssi + ' dBm';
    $('ip-lbl2').textContent = s.ip;
    $('ssid-lbl').textContent = s.ssid + ' (' + s.rssi + ' dBm)';
    $('mdns-lbl').textContent = s.hostname + '.local';
    $('mdns-lbl').href = 'http://' + s.hostname + '.local';
    $('ap-lbl').textContent = s.ap;

    $('vib-status').textContent = !s.accel ? 'ADXL345 não encontrado: confira SDA/SCL e CS em 3V3.'
      : !s.cal ? 'Sensor não calibrado.' : 'Calibração de repouso: ' + s.grav.toFixed(0) + ' LSB.';

    const pill = $('post-pill'), lbl = $('post-lbl');
    if (!s.post_enabled) {
      pill.className = 'pill';
      lbl.textContent = 'Sem destino';
      $('post-info').textContent = 'Destino não configurado';
    } else if (s.post_age_s < 0) {
      pill.className = 'pill';
      lbl.textContent = 'Aguardando';
      $('post-info').textContent = 'Nenhum envio ainda';
    } else {
      pill.className = 'pill ' + (s.post_ok ? 'ok' : 'crit');
      lbl.textContent = s.post_ok ? 'Enviando ao Eixo' : 'Erro no envio';
      const code = s.post_code > 0 ? 'HTTP ' + s.post_code : 'falha de conexão';
      $('post-info').textContent = code + ' · há ' + s.post_age_s + ' s';
    }
    $('queue-info').textContent = s.queue + ' de ' + s.queue_max + ' leituras · ' + s.lost + ' perdidas · boot ' + s.boot +
      ' · seq ' + s.seq + ' · fw ' + s.fw;
  } catch(e) {}
}

async function loadConfig() {
  try {
    const c = await api('/api/config');
    ['name', 'post_url', 'api_key', 'post_interval', 'batch_size', 'sample_delay'].forEach(k => {
      if (c[k] !== undefined) $(k).value = c[k];
    });
    $('device_token').value = '';
    $('device_token').placeholder = c.device_token_set
      ? 'gravado (…' + c.device_token_hint + '); deixe vazio para manter'
      : 'eixo_dev_...';
    syncAlpha(c.alpha ?? 0.2);
  } catch(e) {}
}

// ── Acoes ──
async function saveDest() {
  try {
    await post('/api/config', {
      post_url:      $('post_url').value.trim(),
      device_token:  $('device_token').value.trim(),
      api_key:       $('api_key').value.trim(),
      post_interval: Number($('post_interval').value),
    });
    toast('Destino salvo.');
    loadConfig();
    loadStatus();
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function postNow() {
  try {
    await post('/api/post-now');
    toast('Envio agendado para o próximo ciclo.');
    setTimeout(loadStatus, 4000);
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function saveName() {
  try {
    const r = await post('/api/config', {name: $('name').value.trim()});
    if (r.restart) {
      toast('Reiniciando… acesse http://' + r.name + '.local');
      setTimeout(() => location.href = 'http://' + r.name + '.local', 8000);
    } else {
      toast('Nome mantido.');
    }
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function saveSensor() {
  try {
    await post('/api/config', {
      batch_size:   Number($('batch_size').value),
      sample_delay: Number($('sample_delay').value),
      alpha:        parseFloat($('alpha').value),
    });
    toast('Configuração de coleta salva.');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function calibrate() {
  if (!confirm('O sensor precisa estar parado (máquina desligada).\nCalibrar agora?')) return;
  const btn = $('btn-cal');
  btn.disabled = true;
  btn.innerHTML = '<span class="spin"></span> Calibrando…';
  try {
    await post('/api/calibrate');
    toast('Calibração iniciada. Mantenha o sensor parado.');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
  setTimeout(() => { btn.disabled = false; btn.textContent = 'Calibrar'; loadStatus(); }, 6000);
}

async function resetWifi() {
  if (!confirm('Apaga o Wi-Fi salvo e reinicia em modo AP.\nContinuar?')) return;
  try {
    await post('/api/wifi/reset');
    toast('Wi-Fi redefinido. Conecte ao AP do coletor.');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function restartDevice() {
  if (!confirm('Reiniciar o coletor?')) return;
  try {
    await post('/api/restart');
    toast('Reiniciando…');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

loadConfig();
loadStatus();
loadReadings();
setInterval(loadReadings, 3000);
setInterval(loadStatus, 5000);
</script>

</body>
</html>
)rawliteral";
