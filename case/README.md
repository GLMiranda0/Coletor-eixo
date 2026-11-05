# Cases do coletor — protótipo em 2 unidades

Modelo paramétrico em CadQuery. As medidas ficam no topo de cada arquivo.

| Arquivo | Conteúdo |
|---|---|
| [unidade_sensor.py](unidade_sensor.py) | unidade que vai no motor: base de alumínio, ímãs, sonda DS18B20, perfurada com o GY-291 |
| [unidade_principal.py](unidade_principal.py) | unidade principal: protoboard com ESP32, P2 dos SCT-013, USB-C, borneira |
| [common.py](common.py) | adaptador P2, utilitários, cores, checagem de interferência e exportação |
| [render.py](render.py) | imagens PNG das duas unidades |

```
.venv\Scripts\python.exe unidade_sensor.py      # STEP/STL em out/sensor + checa interferências
.venv\Scripts\python.exe unidade_principal.py   # STEP/STL em out/principal
.venv\Scripts\python.exe render.py              # imagens em out/imagens
```

Impressão: **ASA** (ou no mínimo PETG; o PLA amolece perto de 55 °C), 0,2 mm, 4 perímetros, 30 % de preenchimento.

---

## Unidade de sensores — 84 × 66 × 37 mm (a borneira sai ~9 mm pela lateral)

- **Base de alumínio 6061, 78 × 60 × 10 mm**, com 4 ímãs Ø20 × 5 embutidos rente ao fundo. O caminho motor → alumínio → espaçadores de latão → perfurada é todo de metal.
- **Sonda DS18B20** num poço térmico de Ø6,3 × 52 mm dentro da base, com pasta térmica. Um parafuso sem cabeça M3, apertado pela face de trás (há um furo de acesso na capa), trava a sonda.
- **Placa perfurada 7 × 5 cm**, em 3 espaçadores M2 × 8. O furo que fica embaixo da borneira não é usado. A placa está girada 180° em relação à foto.
- **Borneira de 12 vias e saída da sonda na lateral direita.** A abertura é aberta por baixo, para a capa descer por cima delas.
- **Capa impressa (1 peça):** abraça a base pelos lados, se apoia no topo da base por abas internas e é presa por 4 × M3 laterais roscados no alumínio. A capa fica 0,5 mm acima do fundo, então só a base encosta no motor.

### Usinagem da base
Origem no canto frontal esquerdo, embaixo. X aponta para a borneira.

| Item | Posição | Detalhe |
|---|---|---|
| Bolsos dos ímãs (por baixo) | (22,16) (58,16) (22,38) (58,38) | Ø20,4 × 5 mm |
| Rosca dos ímãs | mesmas posições | M4, 4 mm de profundidade a partir do fundo do bolso |
| Poço térmico | Y = 54; centro a 4,5 mm do fundo | Ø6,3 × 52 mm, entrando pela face direita |
| Trava da sonda | X = 70; centro a 4,5 mm do fundo | M3 roscado pela face de trás até o poço |
| Parafusos da capa | frente: X = 20 e 58; esquerda: Y = 15 e 45 | M3 × 6 mm, horizontal, a 7 mm do fundo |
| Espaçadores (topo) | (10.5,7.5) (10.5,52.5) (75.5,7.5) | M2 × 5 mm |

---

## Unidade principal — 92 × 86 × 44,5 mm, mais 8 mm dos pés

- **Caixa impressa:** fundo e paredes numa peça só. Tem 4 pés com ímã (o mesmo Ø20 × 5, preso com M4 × 10 autoatarraxante) e 2 torres para as presilhas da protoboard.
- **Frente:** 3 adaptadores P2 → borne 4 vias. Uma aleta de 0,9 mm no meio da parede entra na 1ª ranhura do adaptador, e a lingueta da tampa trava cada um por cima. O bico do P2 fica 10 mm para fora.
- **Lateral esquerda:** USB-C do ESP32.
- **Traseira:** borneira 2,54 mm da protoboard, saindo 6 mm para fora. É nela que entra o cabo da unidade de sensores.
- **Tampa:** ventilação sobre o ESP32, gravações CT1/CT2/CT3 e SENSOR, e 4 × M3 em insertos.

### Adaptador P2 → borne 4 vias
A ficha técnica do Goobay 59200, equivalente ao da foto, confirma **5,0 mm entre os bornes**, ou seja, um bloco de cerca de 20 mm. Anúncios do mesmo tipo dão **35,6 a 37,4 mm de comprimento**. O resto foi estimado pela foto e deve ser medido (`P2_*` em `common.py`):

| Parâmetro | Valor usado |
|---|---|
| Comprimento total | 36 mm (bico 10 + corpo 16 + borne 10) |
| Bico | Ø9 mm |
| Corpo sobre as cristas | 12 × 11 mm junto ao bico, 20 × 12,5 mm junto ao borne |
| Ranhuras | 1 mm de profundidade, passo de 2 mm |
| Bloco do borne | 20 × 12,5 mm |

---

## Ferragens

| Qtd | Item | Unidade |
|---|---|---|
| 8 | Ímã de neodímio Ø20 × 5 com furo escareado M4 | 4 em cada |
| 4 | Parafuso M4 × 8 cabeça chata | sensor (ímãs no alumínio) |
| 4 | Parafuso M4 × 10 autoatarraxante cabeça chata | principal (ímãs nos pés) |
| 4 | Parafuso M3 × 8 cabeça cilíndrica | sensor (capa) |
| 1 | Parafuso sem cabeça M3 × 6 + pasta térmica | sensor (sonda) |
| 3 | Espaçador de latão M2 × 8 macho-fêmea + parafuso M2 × 4 | sensor (perfurada) |
| 4 | Inserto M3 × 5,7 a quente + parafuso M3 × 8 cabeça chata | principal (tampa) |
| 2 | Parafuso M3 × 10 autoatarraxante | principal (presilhas) |

## Medidas a confirmar
- **Adaptador P2:** principalmente a medida no fundo da 1ª ranhura e o passo das ranhuras.
- **Protoboard e ESP32:** 83 × 55 × 8,5 mm, altura do USB-C e quanto ele fica recuado da ponta.
- **Borneira da perfurada:** passo (usei 3,5 mm) e quanto o plugue passa da borda da placa (usei 9 mm).
- **Sonda:** Ø6 × 50 mm.

## Fontes
- [Goobay/Wentronic 59200 — ficha técnica](https://electrokit.com/upload/quick/09/6e/988f_41022943_TDS.pdf)
- [BC Robotics — adaptador P2 fêmea → borne, 37,4 × 18,1 × 12,2 mm](https://bc-robotics.com/shop/3-5mm-2-channel-audio-to-terminal-block-adapter-female/)
- [Sharvi Electronics — P2 TRRS fêmea → borne, 35,6 mm](https://sharvielectronics.com/?p=82858)
