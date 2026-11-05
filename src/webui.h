#pragma once

// Interface web servida em http://coletor-XXXXXX.local (mesmo visual do prototipo de vibracao)
const char WEBUI_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Coletor</title>
<style>
:root{
  --navy:#1C2E54;--orange:#E97420;--orange-d:#C55F0F;--bg:#EEF0F5;--surface:#FFFFFF;
  --surf2:#F6F7FA;--text:#1C2E54;--muted:#64748B;--border:#DDE3EE;--green:#0A9B6B;--red:#D63B3B;
  --r:10px;--sh:0 1px 4px rgba(28,46,84,.09),0 4px 16px rgba(28,46,84,.06);
}
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:var(--bg);color:var(--text);min-height:100vh}
header{background:var(--navy);color:#fff;height:60px;padding:0 24px;display:flex;align-items:center;
  justify-content:space-between;box-shadow:0 2px 10px rgba(28,46,84,.35);position:sticky;top:0;z-index:100}
.brand{display:flex;align-items:center;gap:12px}
.brand-mark{width:34px;height:34px;border-radius:8px;background:var(--orange);display:flex;align-items:center;justify-content:center}
.brand-mark svg{width:20px;height:20px;fill:none;stroke:#fff;stroke-width:2.2;stroke-linecap:round}
.brand-text h1{font-size:16px;font-weight:700;letter-spacing:.02em}
.brand-text p{font-size:11px;opacity:.6;margin-top:1px}
.badge{display:flex;align-items:center;gap:7px;background:rgba(255,255,255,.1);border-radius:20px;padding:5px 13px;font-size:12px;font-weight:500}
.dot{width:7px;height:7px;border-radius:50%;background:#6B7280;flex-shrink:0}
.dot.on{background:#34D399;box-shadow:0 0 0 3px rgba(52,211,153,.25)}
.dot.off{background:#F87171}
main{max-width:860px;margin:0 auto;padding:24px 16px;display:grid;grid-template-columns:1fr 1fr;gap:18px}
@media(max-width:620px){main{grid-template-columns:1fr}}
.full{grid-column:1/-1}
.card{background:var(--surface);border-radius:var(--r);box-shadow:var(--sh);padding:22px}
.card-head{display:flex;align-items:center;gap:10px;padding-bottom:14px;margin-bottom:16px;border-bottom:1px solid var(--border)}
.card-icon{width:30px;height:30px;border-radius:7px;background:rgba(233,116,32,.1);display:flex;align-items:center;justify-content:center;flex-shrink:0}
.card-icon svg{width:16px;height:16px;fill:none;stroke:var(--orange);stroke-width:2.2;stroke-linecap:round;stroke-linejoin:round}
.card-head h2{font-size:14px;font-weight:700;letter-spacing:.01em}
.metrics{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;margin-bottom:14px}
.metrics.m3{grid-template-columns:repeat(3,1fr)}
@media(max-width:480px){.metrics,.metrics.m3{grid-template-columns:repeat(2,1fr)}}
.metric{background:var(--surf2);border:1px solid var(--border);border-radius:8px;padding:14px 10px;text-align:center}
.metric .lbl{font-size:10px;font-weight:700;text-transform:uppercase;letter-spacing:.07em;color:var(--muted);margin-bottom:7px}
.metric .val{font-size:22px;font-weight:800;color:var(--navy);font-variant-numeric:tabular-nums;line-height:1}
.metric .unit{font-size:10px;color:var(--muted);margin-top:4px}
.metric.hi .val{color:var(--red)}
.metric.ok .val{color:var(--green)}
.sec{font-size:10px;font-weight:800;text-transform:uppercase;letter-spacing:.08em;color:var(--muted);margin:4px 0 10px}
.readings-footer{display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:10px}
.ts{font-size:11px;color:var(--muted)}
.fg{margin-bottom:12px}
label{display:block;font-size:12px;font-weight:600;color:var(--muted);margin-bottom:5px;letter-spacing:.02em}
input[type=text],input[type=number],input[type=url]{width:100%;padding:8px 11px;border:1.5px solid var(--border);border-radius:7px;
  font-size:13px;color:var(--text);background:var(--surf2);outline:none;transition:border-color .15s,background .15s}
input:focus{border-color:var(--orange);background:#fff}
.row2{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.slider-wrap{display:flex;align-items:center;gap:10px}
input[type=range]{flex:1;accent-color:var(--orange);height:4px}
.alpha-num{width:64px;text-align:center}
.btn{display:inline-flex;align-items:center;justify-content:center;gap:6px;padding:8px 16px;border-radius:7px;font-size:13px;
  font-weight:700;border:none;cursor:pointer;transition:all .15s}
.btn-primary{background:var(--orange);color:#fff}
.btn-primary:hover{background:var(--orange-d)}
.btn-ghost{background:var(--surf2);color:var(--text);border:1.5px solid var(--border)}
.btn-ghost:hover{border-color:var(--orange);color:var(--orange)}
.btn-danger{background:transparent;color:var(--red);border:1.5px solid var(--red)}
.btn-danger:hover{background:var(--red);color:#fff}
.btn:disabled{opacity:.5;cursor:not-allowed}
.actions{display:flex;gap:8px;margin-top:16px;flex-wrap:wrap}
hr.div{border:none;border-top:1px solid var(--border);margin:16px 0}
.spin{display:inline-block;width:13px;height:13px;border:2px solid rgba(255,255,255,.3);border-top-color:#fff;border-radius:50%;animation:spin .6s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
#toast{position:fixed;bottom:24px;right:24px;background:var(--navy);color:#fff;padding:11px 18px;border-radius:8px;font-size:13px;
  font-weight:600;box-shadow:0 4px 20px rgba(0,0,0,.2);transform:translateY(70px);opacity:0;
  transition:all .28s cubic-bezier(.34,1.56,.64,1);z-index:999;pointer-events:none}
#toast.show{transform:translateY(0);opacity:1}
#toast.ok{border-left:4px solid var(--green)}
#toast.err{border-left:4px solid var(--red)}
.info{display:flex;justify-content:space-between;align-items:center;gap:12px;padding:9px 0;border-bottom:1px solid var(--border);font-size:13px}
.info:last-child{border:none}
.info .k{color:var(--muted)}
.info .v{font-weight:600;text-align:right;word-break:break-all}
.hint{font-size:11px;color:var(--muted);margin-top:4px;line-height:1.4}
</style>
</head>
<body>

<header>
  <div class="brand">
    <div class="brand-mark">
      <svg viewBox="0 0 24 24"><polyline points="2,12 5,6 8,18 11,9 14,15 17,3 20,12 22,12"/></svg>
    </div>
    <div class="brand-text">
      <h1>Coletor</h1>
      <p id="ip-lbl">Carregando&hellip;</p>
    </div>
  </div>
  <div class="badge">
    <div class="dot" id="dot"></div>
    <span id="post-lbl">Envio</span>
  </div>
</header>

<main>

  <!-- ── Vibracao ── -->
  <div class="card full">
    <div class="card-head">
      <div class="card-icon"><svg viewBox="0 0 24 24"><polyline points="22,12 18,12 15,21 9,3 6,12 2,12"/></svg></div>
      <h2>Vibra&ccedil;&atilde;o (ADXL345)</h2>
    </div>
    <div class="metrics">
      <div class="metric" id="box-ema"><div class="lbl">EMA</div><div class="val" id="v-ema">&mdash;</div><div class="unit">adimensional</div></div>
      <div class="metric" id="box-rms"><div class="lbl">RMS</div><div class="val" id="v-rms">&mdash;</div><div class="unit">adimensional</div></div>
      <div class="metric" id="box-peak"><div class="lbl">Pico</div><div class="val" id="v-peak">&mdash;</div><div class="unit">adimensional</div></div>
      <div class="metric" id="box-std"><div class="lbl">Desvio Padr&atilde;o</div><div class="val" id="v-std">&mdash;</div><div class="unit">adimensional</div></div>
    </div>
    <div class="readings-footer">
      <span class="ts" id="vib-status">Repouso &asymp; 1.0</span>
      <button class="btn btn-primary" id="btn-cal" onclick="calibrate()">&#9878; Calibrar</button>
    </div>
  </div>

  <!-- ── Temperatura e corrente ── -->
  <div class="card full">
    <div class="card-head">
      <div class="card-icon"><svg viewBox="0 0 24 24"><path d="M14 14.76V3.5a2.5 2.5 0 00-5 0v11.26a4.5 4.5 0 105 0z"/></svg></div>
      <h2>Temperatura e Corrente</h2>
    </div>
    <div class="sec">Temperatura (DS18B20)</div>
    <div class="metrics" id="temps"></div>
    <div class="sec">Corrente (SCT-013)</div>
    <div class="metrics m3" id="amps"></div>
    <div class="readings-footer">
      <span class="ts" id="ts">Aguardando primeira leitura&hellip;</span>
      <button class="btn btn-ghost" onclick="loadReadings()">&#8635; Atualizar</button>
    </div>
  </div>

  <!-- ── Destino ── -->
  <div class="card">
    <div class="card-head">
      <div class="card-icon"><svg viewBox="0 0 24 24"><line x1="22" y1="2" x2="11" y2="13"/><polygon points="22,2 15,22 11,13 2,9"/></svg></div>
      <h2>Destino dos Dados</h2>
    </div>
    <div class="fg">
      <label>URL de destino (POST)</label>
      <input type="url" id="post_url" placeholder="https://servidor/api/ingest">
      <div class="hint">Recebe um JSON por envio. Vazio desliga o envio.</div>
    </div>
    <div class="fg">
      <label>Token do dispositivo (opcional)</label>
      <input type="text" id="device_token" autocomplete="off">
    </div>
    <div class="fg">
      <label>Intervalo de envio (s)</label>
      <input type="number" id="post_interval" min="2" max="3600">
    </div>
    <div class="actions">
      <button class="btn btn-primary" onclick="saveDest()">Salvar Destino</button>
      <button class="btn btn-ghost" onclick="postNow()">Enviar agora</button>
    </div>
    <hr class="div">
    <div class="info"><span class="k">&Uacute;ltimo envio</span><span class="v" id="post-info">&mdash;</span></div>
  </div>

  <!-- ── Sensor + Sistema ── -->
  <div class="card">
    <div class="card-head">
      <div class="card-icon"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="3"/><path d="M19.07 4.93a10 10 0 010 14.14M4.93 4.93a10 10 0 000 14.14"/></svg></div>
      <h2>Coleta de Vibra&ccedil;&atilde;o</h2>
    </div>
    <div class="row2">
      <div class="fg"><label>Amostras por janela</label><input type="number" id="batch_size" min="10" max="2000"></div>
      <div class="fg"><label>Intervalo (ms)</label><input type="number" id="sample_delay" min="1" max="1000"></div>
    </div>
    <div class="fg">
      <label>K do EMA (&alpha;)</label>
      <div class="slider-wrap">
        <input type="range" id="alpha_r" min="0.01" max="1" step="0.01" oninput="syncAlpha(this.value)">
        <input type="number" id="alpha" class="alpha-num" min="0.01" max="1" step="0.01" oninput="syncAlpha(this.value)">
      </div>
      <div class="hint">Menor = mais suave &bull; Maior = mais reativo</div>
    </div>
    <div class="actions"><button class="btn btn-primary" onclick="saveSensor()">Salvar Coleta</button></div>

    <hr class="div">
    <div class="sec">Wi-Fi &amp; Sistema</div>
    <div class="fg">
      <label>Nome na rede</label>
      <div class="slider-wrap">
        <input type="text" id="name" maxlength="15" autocomplete="off">
        <button class="btn btn-ghost" onclick="saveName()">Salvar</button>
      </div>
      <div class="hint">Letras, n&uacute;meros e h&iacute;fen. Com mais de um coletor na rede, use nomes diferentes. Reinicia ao salvar.</div>
    </div>
    <div class="info"><span class="k">Rede conectada</span><span class="v" id="ssid-lbl">&mdash;</span></div>
    <div class="info"><span class="k">Endere&ccedil;o IP (DHCP)</span><span class="v" id="ip-lbl2">&mdash;</span></div>
    <div class="info"><span class="k">Acesso por nome</span>
      <a class="v" id="mdns-lbl" href="#" target="_blank" style="color:var(--orange);text-decoration:none">&mdash;</a></div>
    <div class="info"><span class="k">Rede de configura&ccedil;&atilde;o (AP)</span><span class="v" id="ap-lbl">&mdash;</span></div>
    <div class="actions" style="margin-top:14px">
      <button class="btn btn-ghost" onclick="resetWifi()">Redefinir Wi-Fi</button>
      <button class="btn btn-danger" onclick="restartDevice()">Reiniciar</button>
    </div>
  </div>

</main>

<div id="toast"></div>

<script>
const $ = id => document.getElementById(id);

function syncAlpha(v) {
  v = Math.min(1, Math.max(0.01, parseFloat(v) || 0.01));
  $('alpha').value = v.toFixed(2);
  $('alpha_r').value = v;
}

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
  return api(path, {method:'POST', headers:{'Content-Type':'application/json'},
    body: body ? JSON.stringify(body) : undefined});
}

let toastTimer;
function toast(msg, type='ok') {
  const el = $('toast');
  el.textContent = msg;
  el.className = 'show ' + type;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => el.className = '', 3000);
}

function setVib(id, boxId, val) {
  $(id).textContent = val.toFixed(4);
  $(boxId).className = 'metric' + (val > 1.5 ? ' hi' : val < 1.1 ? ' ok' : '');
}

function metric(label, val, unit) {
  return '<div class="metric"><div class="lbl">' + label + '</div><div class="val">' + val +
         '</div><div class="unit">' + unit + '</div></div>';
}

async function loadReadings() {
  try {
    const d = await api('/api/readings');
    if (d.vib) {
      setVib('v-ema', 'box-ema', d.vib.ema);
      setVib('v-rms', 'box-rms', d.vib.rms);
      setVib('v-peak', 'box-peak', d.vib.peak);
      $('v-std').textContent = d.vib.std.toFixed(4);
    }
    $('temps').innerHTML = d.temp_c.length
      ? d.temp_c.map((t, i) => metric('Sensor ' + i, t === null ? 'erro' : t.toFixed(1), '&deg;C')).join('')
      : '<span class="ts">Nenhum sensor encontrado</span>';
    $('amps').innerHTML = Object.entries(d.current_a)
      .map(([k, a]) => metric(k, a.toFixed(2), 'A')).join('');
    $('ts').textContent = d.ts ? 'Atualizado: ' + new Date(d.ts).toLocaleTimeString('pt-BR')
                               : 'Atualizado (relógio ainda não sincronizado)';
  } catch(e) {
    $('ts').textContent = 'Sem dados ainda';
  }
}

async function loadStatus() {
  try {
    const s = await api('/api/status');
    $('ip-lbl').textContent = s.hostname + ' · ' + s.ip;
    $('ip-lbl2').textContent = s.ip;
    $('ssid-lbl').textContent = s.ssid + ' (' + s.rssi + ' dBm)';
    const url = 'http://' + s.hostname + '.local';
    $('mdns-lbl').textContent = s.hostname + '.local';
    $('mdns-lbl').href = url;
    $('ap-lbl').textContent = s.ap;

    $('vib-status').textContent = !s.accel ? 'ADXL345 não encontrado — confira SDA/SCL e CS em 3V3'
      : !s.cal ? 'Sensor não calibrado' : 'Repouso ≈ 1.0 · calibração ' + s.grav.toFixed(0) + ' LSB';

    const dot = $('dot'), lbl = $('post-lbl');
    if (!s.post_enabled) {
      dot.className = 'dot';
      lbl.textContent = 'Sem destino';
      $('post-info').textContent = 'Destino não configurado';
    } else if (s.post_age_s < 0) {
      dot.className = 'dot';
      lbl.textContent = 'Aguardando';
      $('post-info').textContent = 'Nenhum envio ainda';
    } else {
      dot.className = 'dot ' + (s.post_ok ? 'on' : 'off');
      lbl.textContent = s.post_ok ? 'Envio ok' : 'Envio erro';
      const code = s.post_code > 0 ? 'HTTP ' + s.post_code : 'falha de conexão';
      $('post-info').textContent = code + ' · há ' + s.post_age_s + ' s';
    }
  } catch(e) {}
}

async function loadConfig() {
  try {
    const c = await api('/api/config');
    ['name','post_url','device_token','post_interval','batch_size','sample_delay'].forEach(k => {
      if (c[k] !== undefined) $(k).value = c[k];
    });
    syncAlpha(c.alpha ?? 0.2);
  } catch(e) {}
}

async function saveDest() {
  try {
    await post('/api/config', {
      post_url:      $('post_url').value.trim(),
      device_token:  $('device_token').value.trim(),
      post_interval: Number($('post_interval').value),
    });
    toast('Destino salvo!');
    loadConfig();
    loadStatus();
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function postNow() {
  try {
    await post('/api/post-now');
    toast('Envio agendado para o próximo ciclo');
    setTimeout(loadStatus, 4000);
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function saveName() {
  try {
    const r = await post('/api/config', {name: $('name').value.trim()});
    if (r.restart) {
      toast('Reiniciando... acesse http://' + r.name + '.local');
      setTimeout(() => location.href = 'http://' + r.name + '.local', 8000);
    } else {
      toast('Nome mantido');
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
    toast('Configurações de coleta salvas!');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function calibrate() {
  if (!confirm('O sensor precisa estar parado (máquina desligada).\nCalibrar agora?')) return;
  const btn = $('btn-cal');
  btn.disabled = true;
  btn.innerHTML = '<span class="spin"></span> Calibrando...';
  try {
    await post('/api/calibrate');
    toast('Calibração iniciada! Mantenha o sensor parado.');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
  setTimeout(() => { btn.disabled = false; btn.innerHTML = '&#9878; Calibrar'; loadStatus(); }, 6000);
}

async function resetWifi() {
  if (!confirm('Apagará o Wi-Fi salvo e reiniciará em modo AP.\nContinuar?')) return;
  try {
    await post('/api/wifi/reset');
    toast('Wi-Fi redefinido. Conecte ao AP do coletor.');
  } catch(e) { toast('Erro: ' + e.message, 'err'); }
}

async function restartDevice() {
  if (!confirm('Reiniciar o coletor?')) return;
  try {
    await post('/api/restart');
    toast('Reiniciando...');
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
