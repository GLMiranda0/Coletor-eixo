// Esquemático do Coletor Eixo (placa principal, ESP32-WROOM-32E soldado).
// Cada peça: [designador, LCSC, x, y, rotação, { pino: rede }]. Pino sem rede fica sem ligação.
// Mesmos GPIOs do protótipo (firmware 1.1.0 roda sem alteração).
const P = [
  // ── Entrada USB-C, proteção e 3,3 V ──
  ['J1',  'C2765186', 150, 950, 0, { A1B12:'GND', B1A12:'GND', A4B9:'VBUS', B4A9:'VBUS', A5:'CC1', B5:'CC2', A6:'USB_DP', B6:'USB_DP', A7:'USB_DN', B7:'USB_DN', 13:'GND', 14:'GND' }],
  ['R1',  'C23186',   330, 1100, 0, { 1:'CC1', 2:'GND' }],            // 5,1k: o carregador entrega 5 V
  ['R2',  'C23186',   330, 1050, 0, { 1:'CC2', 2:'GND' }],
  ['F1',  'C883124',  330, 1000, 0, { 1:'VBUS', 2:'+5V' }],           // PTC 500 mA
  ['U4',  'C7519',    330, 860, 0,  { 1:'USB_DP', 6:'USB_DP', 3:'USB_DN', 4:'USB_DN', 2:'GND', 5:'+5V' }], // ESD USB
  ['C1',  'C45783',   500, 1100, 0, { 1:'+5V', 2:'GND' }],            // 22 uF
  ['U3',  'C6186',    640, 1050, 0, { 3:'+5V', 1:'GND', 2:'3V3', 4:'3V3' }], // AMS1117-3.3
  ['C2',  'C45783',   780, 1100, 0, { 1:'3V3', 2:'GND' }],            // 22 uF
  ['C3',  'C14663',   780, 1050, 0, { 1:'3V3', 2:'GND' }],            // 100 nF
  // ── ESP32 ──
  ['U1',  'C701341',  800, 650, 0, { 1:'GND', 15:'GND', 38:'GND', 39:'GND', 2:'3V3', 3:'EN',
        6:'CT1_ADC', 7:'CT2_ADC', 8:'CT3_ADC', 24:'IO2_LED', 25:'IO0', 26:'ONEWIRE', 33:'SDA', 36:'SCL', 34:'U0RXD', 35:'U0TXD' }],
  ['C4',  'C19702',   1000, 1100, 0, { 1:'3V3', 2:'GND' }],           // 10 uF junto ao modulo
  ['C5',  'C14663',   1000, 1050, 0, { 1:'3V3', 2:'GND' }],           // 100 nF
  ['R3',  'C25804',   450, 640, 0, { 1:'3V3', 2:'EN' }],              // EN: 10k + 1 uF
  ['C6',  'C15849',   450, 590, 0, { 1:'EN', 2:'GND' }],
  ['SW1', 'C318884',  450, 510, 0, { 1:'EN', 2:'EN', 3:'GND', 4:'GND' }],   // RESET
  ['R4',  'C25804',   250, 640, 0, { 1:'3V3', 2:'IO0' }],
  ['SW2', 'C318884',  250, 510, 0, { 1:'IO0', 2:'IO0', 3:'GND', 4:'GND' }], // BOOT (portal / gravacao)
  ['R7',  'C21190',   250, 420, 0, { 1:'IO2_LED', 2:'LED_A' }],       // 1k
  ['LED1','C72041',   420, 420, 0, { 2:'LED_A', 1:'GND' }],           // LED azul de status (GPIO2)
  // ── USB-serial CH340C + auto-reset ──
  ['U2',  'C84681',   1250, 980, 0, { 1:'GND', 2:'U0RXD', 3:'U0TXD', 4:'3V3', 5:'USB_DP', 6:'USB_DN', 16:'3V3', 13:'DTR', 14:'RTS' }],
  ['C7',  'C14663',   1450, 1080, 0, { 1:'3V3', 2:'GND' }],
  ['R5',  'C25804',   1150, 760, 0, { 1:'DTR', 2:'Q1_B' }],
  ['Q1',  'C20526',   1250, 760, 0, { B:'Q1_B', E:'RTS', C:'EN' }],
  ['R6',  'C25804',   1350, 760, 0, { 1:'RTS', 2:'Q2_B' }],
  ['Q2',  'C20526',   1450, 760, 0, { B:'Q2_B', E:'DTR', C:'IO0' }],
  // ── Sensores de corrente SCT-013-005 (P2) ──
  ['R8',  'C25804',   150, 300, 0, { 1:'3V3', 2:'VBIAS' }],           // polarizacao 1,65 V
  ['R9',  'C25804',   150, 250, 0, { 1:'VBIAS', 2:'GND' }],
  ['C8',  'C19702',   150, 200, 0, { 1:'VBIAS', 2:'GND' }],
  ['J2',  'C145814',  350, 300, 0, { 1:'VBIAS', 4:'CT1_IN' }],        // pino 1 = manga, 4 = ponta
  ['R10', 'C21190',   500, 320, 0, { 1:'CT1_IN', 2:'CT1_ADC' }],
  ['C9',  'C57112',   500, 260, 0, { 1:'CT1_ADC', 2:'GND' }],
  ['J3',  'C145814',  620, 300, 0, { 1:'VBIAS', 4:'CT2_IN' }],
  ['R11', 'C21190',   770, 320, 0, { 1:'CT2_IN', 2:'CT2_ADC' }],
  ['C10', 'C57112',   770, 260, 0, { 1:'CT2_ADC', 2:'GND' }],
  ['J4',  'C145814',  890, 300, 0, { 1:'VBIAS', 4:'CT3_IN' }],
  ['R12', 'C21190',   1040, 320, 0, { 1:'CT3_IN', 2:'CT3_ADC' }],
  ['C11', 'C57112',   1040, 260, 0, { 1:'CT3_ADC', 2:'GND' }],
  // ── Unidade de sensores (DS18B20 + ADXL345) ──
  ['R13', 'C23162',   1250, 600, 0, { 1:'3V3', 2:'ONEWIRE' }],        // 4,7k 1-Wire
  ['R14', 'C23162',   1250, 550, 0, { 1:'3V3', 2:'SDA' }],
  ['R15', 'C23162',   1250, 500, 0, { 1:'3V3', 2:'SCL' }],
  ['C12', 'C14663',   1250, 450, 0, { 1:'3V3', 2:'GND' }],
  ['J5',  'C441185',  1500, 550, 0, { 1:'3V3', 2:'GND', 3:'SDA', 4:'SCL', 5:'ONEWIRE' }],
];

const L = 20; // comprimento do fio de cada pino
await eda.dmt_EditorControl.openDocument("75106721db06226d");
await eda.sch_PrimitiveWire.delete(await eda.sch_PrimitiveWire.getAllPrimitiveId());
await eda.sch_PrimitiveComponent.delete(await eda.sch_PrimitiveComponent.getAllPrimitiveId("part"));
const devCache = {};
const report = [];
for (const [ref, lcsc, x, y, rot, nets] of P) {
  devCache[lcsc] ??= await eda.lib_Device.getByLcscIds(lcsc);
  const c = await eda.sch_PrimitiveComponent.create(devCache[lcsc], x, y, undefined, rot);
  if (!c) throw new Error('falha ao criar ' + ref);
  await eda.sch_PrimitiveComponent.modify(c.primitiveId, { designator: ref });
  const pins = await eda.sch_PrimitiveComponent.getAllPinsByPrimitiveId(c.primitiveId);
  const used = new Set();
  for (const p of pins) {
    const net = nets[p.pinNumber] ?? nets[p.pinName];
    if (!net) continue;
    used.add(String(p.pinNumber in nets ? p.pinNumber : p.pinName));
    const r = ((p.rotation % 360) + 360) % 360;
    const [dx, dy] = r === 0 ? [L, 0] : r === 180 ? [-L, 0] : r === 90 ? [0, L] : [0, -L];
    await eda.sch_PrimitiveWire.create([p.x, p.y, p.x + dx, p.y + dy], net);
  }
  const missing = Object.keys(nets).filter(k => !used.has(k));
  report.push(`${ref} ${lcsc} pinos=${pins.length} ligados=${used.size}` + (missing.length ? ` FALTANDO=${missing}` : ''));
  log(ref);
}
await eda.sch_Document.save();
return report;
