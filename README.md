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
| `src/main.cpp` | Firmware: leitura dos sensores, Wi-Fi, servidor web local, envio HTTP(S) |
| `src/webui.h` | Interface web servida pelo próprio coletor (`http://coletor.local`) |
| `platformio.ini` | Projeto PlatformIO (placa, partições, bibliotecas) |
| `case/` | Cases paramétricos em CadQuery (unidade de sensores e unidade principal), STEP/STL e imagens. Veja [`case/README.md`](case/README.md) |

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

1. No primeiro boot o coletor abre o AP **`coletor-XXXXXX`** (portal do WiFiManager).
2. No portal, escolha a rede Wi-Fi e preencha:
   - **Nome na rede** — acesso por `http://NOME.local` (ou `http://NOME` no Windows);
   - **URL de destino (POST)** — endpoint que recebe as leituras;
   - **Token do dispositivo** — opcional, enviado como `device_token`.
3. Depois de conectado, a interface local (`http://coletor.local`) mostra as leituras, permite calibrar
   o acelerômetro, ajustar a coleta de vibração e o intervalo de envio, e forçar um envio.

## Payload enviado

```json
{
  "device_id": "coletor-XXXXXX",
  "name": "coletor",
  "ip": "192.168.0.50",
  "device_token": "…",
  "recorded_at": "2026-11-05T12:00:00.000Z",
  "payload": {
    "x": 0, "y": 0, "z": 256,
    "rms": 1.0, "ema": 1.0, "peak": 1.02, "std": 0.01,
    "temp_c": [42.5],
    "current_a": { "CT1": 3.2, "CT2": 3.1, "CT3": 3.3 }
  }
}
```

Os campos de vibração são normalizados pela gravidade (em repouso ≈ 1,0). `recorded_at` só é enviado
depois que o NTP sincroniza.

## Integração com o eixo-tcc (pendente)

A função de ingestão do [eixo-tcc](https://github.com/GLMiranda0/eixo-tcc)
(`prototipos/supabase/schema.sql`, `ingest(p_token text, p jsonb)`) espera outro formato:

```
POST https://<projeto>.supabase.co/rest/v1/rpc/ingest
apikey: <chave publicável>
Content-Type: application/json

{"p_token": "<token gerado na tela do motor>", "p": {"temp": 42.5, "corr": 3.2, "vib": 1.1}}
```

O firmware atual ainda não envia o header `apikey` nem monta `p_token` / `p` com as grandezas
`temp`, `corr`, `vib`. Até isso ser adaptado, o envio direto para o Supabase do Eixo não é aceito.

## Equipe

Mateus Balbino Alves · Cauan Rodrigues de Oliveira Leite · Lucas Rodrigues Zajac · Gabriel Lima Miranda · Victor Brito Pinheiro · Pedro Henrique Gomes Pereira

Orientador: Prof. Nelson Aguiar
