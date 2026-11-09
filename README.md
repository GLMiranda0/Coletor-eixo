# Coletor Eixo · ESP32

Coletor de campo do **Eixo — Sistema Inteligente de Supervisão e Diagnóstico de Motores Elétricos**
(TCC · Engenharia da Computação · Universidade São Judas Tadeu · 2026).

Este repositório é o **hardware + firmware** do coletor. A plataforma web (front-end, banco Supabase,
função de ingestão, diagnóstico e alertas) fica no repositório complementar:

➡️ **[GLMiranda0/eixo-tcc](https://github.com/GLMiranda0/eixo-tcc)**

```
 motor ──► Coletor-eixo (ESP32) ──POST HTTPS──► Supabase /rest/v1/rpc/ingest ──► eixo-tcc (web)
          corrente · temperatura · vibração        token do dispositivo           painel, histórico, alertas
```

## Conteúdo

| Pasta | Conteúdo |
|---|---|
| `src/main.cpp` | Firmware: leitura dos sensores, Wi-Fi, servidor web local, envio para o `ingest` do Eixo |
| `src/webui.h` | Interface web servida pelo próprio coletor (`http://coletor.local`) |
| `platformio.ini` | Projeto PlatformIO (placa, partições, bibliotecas) |
| `pcb/` | **Proposta** de placa própria (esquemático, PCB, desenho técnico); não será fabricada |

## Hardware

Protótipo em uso: placa **ESP32-WROOM-32 DevKit (30 pinos)** + shield de expansão.

| Sensor | Ligação |
|---|---|
| DS18B20 (temperatura, 1-Wire) | GPIO4 (`D4`), resistor de 4,7 kΩ entre DATA e 3V3 |
| 3 × SCT-013-005 (corrente, 5 A → 1 V RMS) | GPIO34, 35 e 32 (ADC1), polarização de 1,65 V (divisor 10k/10k + 10 µF) |
| ADXL345 / GY-291 (vibração, I2C) | SDA `D21`, SCL `D22`, endereço 0x53 ou 0x1D |
| LED de status | GPIO2 (LED azul onboard) |
| Botão BOOT | toque curto abre o portal de configuração; segurar 5 s apaga o Wi-Fi |

**Próximo passo proposto (não será fabricado):** placa própria com ESP32-WROOM-32E soldado, os mesmos GPIOs
dos sensores e alimentação por USB-C ou bateria Li-ion com carga pela USB, com desenho técnico para a caixa —
ver [`pcb/`](pcb/README.md). O projeto continua com o protótipo acima.

## Compilar e gravar

Requer [PlatformIO](https://platformio.org/) (extensão do VS Code ou CLI).

```bash
pio run -t upload
pio device monitor
```

## Configuração

1. No **eixo-tcc**, cadastre o motor e abra a tela dele: lá ficam o **endpoint** e o **token do dispositivo**.
2. No primeiro boot o coletor abre o AP **`coletor-XXXXXX`** (portal do WiFiManager, `http://192.168.4.1`).
3. No portal, escolha a rede Wi-Fi e preencha:
   - **Nome na rede** — acesso por `http://NOME.local` (ou `http://NOME` no Windows);
   - **URL de destino (POST)** — `https://<projeto>.supabase.co/rest/v1/rpc/ingest`;
   - **Token do dispositivo** — o `eixo_dev_…` gerado na tela do motor;
   - **Chave publicável do Supabase** — a mesma `VITE_SUPABASE_KEY` do app (vai no header `apikey`).
4. Depois de conectado, a interface local (`http://coletor.local`) mostra as leituras, permite calibrar
   o acelerômetro, ajustar a coleta de vibração e o intervalo de envio, mudar o destino e forçar um envio.

Toque curto no botão BOOT reabre o portal sem apagar o Wi-Fi.

## Envio para o Eixo

A partir do firmware **1.1.0** o envio é em lote e exige a migração
`prototipos/supabase/003_ingestao_lotes_persistencia.sql` do eixo-tcc aplicada no banco.

Cada ciclo de leitura (2 s) vira um item numa **fila em RAM de 90 leituras** (3 min). No intervalo de
envio (padrão 10 s, ajustável de 2 a 3600 s), o coletor manda as até 30 leituras mais antigas num único
POST para a função `ingest(p_token, p)` e só as tira da fila quando o Eixo responde 2xx. Com a rede fora do
ar a fila segura as leituras; cheia, descarta a mais antiga e soma em `perdidas`. Depois da queda, os lotes
saem em sequência até a fila esvaziar. A conexão TLS fica aberta entre envios (keep-alive), então o
handshake (~1 s) só acontece na primeira vez ou depois de uma queda.

```
POST https://<projeto>.supabase.co/rest/v1/rpc/ingest
apikey: <chave publicável>
Content-Type: application/json

{
  "p_token": "eixo_dev_…",
  "p": [
    {
      "ts": "2026-11-05T12:00:00.000Z",
      "temp": 42.5,
      "corr": 3.2,
      "vib": 2.71,
      "device_id": "coletor-a1b2c3",
      "boot": 7,
      "seq": 120,
      "fw": "1.1.0",
      "perdidas": 0,
      "corr_fases": { "CT1": 3.2, "CT2": 3.1, "CT3": 3.3 },
      "vib_eixos": [2.71, 1.05, 0.88]
    },
    { "ts": "2026-11-05T12:00:02.000Z", "...": "..." }
  ]
}
```

| Campo | Origem |
|---|---|
| `temp` | maior leitura válida dos DS18B20, °C |
| `corr` | média das fases com leitura (acima de 0,05 A), A; `0` com o motor parado |
| `vib` | velocidade de vibração RMS do eixo mais alto, mm/s (critério da ISO 10816-3, a escala que o Eixo usa) |
| `ts` | hora da aquisição (NTP). Sem NTP vai `idade_ms` (ms desde a aquisição) e o banco calcula a hora |
| `device_id`, `boot`, `seq` | identificam a leitura: `boot` sobe a cada reinício (gravado em flash), `seq` conta dentro do boot. O banco ignora reenvio da mesma leitura |
| `perdidas` | leituras descartadas com a fila cheia desde o boot |

Grandeza sem sensor fica fora do JSON. `fw`, `perdidas`, `corr_fases` e `vib_eixos` ficam em
`leituras.extras` no Eixo; a disponibilidade (recebidas × esperadas pela `seq`) e o atraso aquisição → banco
saem da view `qualidade_telemetria`.

### Segurança

- O token do dispositivo não aparece na interface local nem no portal (só os 4 últimos caracteres);
  deixar o campo vazio ao salvar mantém o token gravado.
- O HTTPS **não verifica o certificado do servidor** (`setInsecure()`): o tráfego vai cifrado, mas um
  intermediário na rede poderia se passar pelo Supabase. Fixar a CA raiz do domínio é o próximo passo.
- A interface local (`http://coletor.local`) não tem login: deixe o coletor numa rede de equipamentos.

### Velocidade de vibração

O ADXL345 mede aceleração. Para cada eixo, o firmware tira a média (a gravidade), integra a janela
de amostras (500 amostras a 2 ms = 1 s) com o integrador de Al-Alaoui e aplica um passa-altas
**Butterworth de 2ª ordem em 10 Hz**, o limite de baixo da banda da ISO 10816. Os primeiros 0,1 s
ficam fora da conta (transiente do filtro).

Conferido com senoides sintéticas (ganho medido ÷ esperado):

| 10 Hz | 15 Hz | 29,5 Hz | 59 Hz | 100 Hz | 200 Hz | ruído, sensor parado |
|---|---|---|---|---|---|---|
| 0,71 (−3 dB, o corte) | 0,91 | 0,99 | 0,99 | 0,99 | 1,03 | ~0,13 mm/s |

O limite de cima é a metade da taxa de amostragem (250 Hz), abaixo dos 1000 Hz da norma: chegar lá
exige ligar o ADXL345 por SPI. O sensor trabalha a 800 Hz (banda interna de 400 Hz) e o firmware lê a
amostra mais recente a cada 2 ms, então vibração entre 250 e 400 Hz pode aparecer dobrada abaixo de
250 Hz; ler a FIFO do sensor a 800 Hz elimina esse dobramento (ainda não feito, precisa de teste na placa). Somado à resolução do sensor (~4 mg), é um valor indicativo para
tendência e alerta, não uma medição certificada.

Os valores normalizados pela gravidade (EMA, RMS, pico, desvio padrão, repouso ≈ 1,0) continuam na
interface local e no serial.

## Equipe

Mateus Balbino Alves · Cauan Rodrigues de Oliveira Leite · Lucas Rodrigues Zajac · Gabriel Lima Miranda · Victor Brito Pinheiro · Pedro Henrique Gomes Pereira

Orientador: Prof. Nelson Aguiar
