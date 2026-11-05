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

## Hardware

Placa **ESP32-WROOM-32 DevKit (30 pinos)** + shield de expansão.

| Sensor | Ligação |
|---|---|
| DS18B20 (temperatura, 1-Wire) | GPIO4 (`D4`), resistor de 4,7 kΩ entre DATA e 3V3 |
| 3 × SCT-013-005 (corrente, 5 A → 1 V RMS) | GPIO34, 35 e 32 (ADC1), polarização de 1,65 V (divisor 10k/10k + 10 µF) |
| ADXL345 / GY-291 (vibração, I2C) | SDA `D21`, SCL `D22`, endereço 0x53 ou 0x1D |
| LED de status | GPIO2 (LED azul onboard) |
| Botão BOOT | toque curto abre o portal de configuração; segurar 5 s apaga o Wi-Fi |

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

Cada envio chama a função `ingest(p_token, p)` do eixo-tcc
(`prototipos/supabase/schema.sql`):

```
POST https://<projeto>.supabase.co/rest/v1/rpc/ingest
apikey: <chave publicável>
Content-Type: application/json

{
  "p_token": "eixo_dev_…",
  "p": {
    "ts": "2026-11-05T12:00:00.000Z",
    "temp": 42.5,
    "corr": 3.2,
    "vib": 2.71,
    "device_id": "coletor-a1b2c3",
    "ip": "192.168.0.50",
    "corr_fases": { "CT1": 3.2, "CT2": 3.1, "CT3": 3.3 },
    "vib_eixos": [2.71, 1.05, 0.88]
  }
}
```

| Campo | Origem |
|---|---|
| `temp` | maior leitura válida dos DS18B20, °C |
| `corr` | média das fases com leitura (acima de 0,05 A), A; `0` com o motor parado |
| `vib` | velocidade de vibração RMS do eixo mais alto, mm/s (critério da ISO 10816-3, a escala que o Eixo usa) |
| `ts` | hora da leitura; só vai depois que o NTP sincroniza (sem ele o banco usa a hora de chegada) |

Grandeza sem sensor fica fora do JSON. O `ingest` lê só `ts`, `temp`, `corr`, `vib`, `tens` e `rot`;
os outros campos de `p` (`device_id`, `ip`, `corr_fases`, `vib_eixos`) são ignorados por ele e servem para depuração.

### Velocidade de vibração

O ADXL345 mede aceleração. Para cada eixo, o firmware tira a média (a gravidade), integra a janela
de amostras (500 amostras a 2 ms = 1 s) e aplica um passa-altas de 1ª ordem em 5 Hz para tirar a deriva.
Conferido com senoides sintéticas: −3 % a 29,5 Hz, −1 % a 59 Hz, +4 % a 100 Hz, ruído de ~0,2 mm/s com o sensor parado.
A banda vai até a metade da taxa de amostragem (250 Hz), abaixo dos 1000 Hz da norma, então é um valor
indicativo para tendência e alerta, não uma medição certificada.

Os valores normalizados pela gravidade (EMA, RMS, pico, desvio padrão, repouso ≈ 1,0) continuam na
interface local e no serial.

## Equipe

Mateus Balbino Alves · Cauan Rodrigues de Oliveira Leite · Lucas Rodrigues Zajac · Gabriel Lima Miranda · Victor Brito Pinheiro · Pedro Henrique Gomes Pereira

Orientador: Prof. Nelson Aguiar
