"""Unidade principal: protoboard com o ESP32, entradas P2 dos SCT-013, USB-C e borneira para a unidade de sensores.

Caixa impressa (fundo + paredes) com 4 pes de ima, tampa com linguetas que travam os P2 e a borneira,
e 2 presilhas que prendem a protoboard.

Eixos: X esquerda -> direita, Y frente (P2) -> tras (borneira), Z para cima. Medidas em mm.
"""

from pathlib import Path

import cadquery as cq

from common import (
    IMA, LARANJA, M3_CLEAR, M3_TAP, MAG_D, MAG_T, NAVY, P2_BLOCK, PRETO, VERDE, WALL, box,
    check_interferences, corner_boss, cyl, engrave, export_unit, fillet_z, magnet, p2_adapter,
    p2_slot_cutters, p2_tongue,
)

W, D = 92.0, 86.0
FLOOR_T = 3.0
WALL_TOP = 42.0
LID_T = 2.5
CORNER_R = 4.0

# pes com ima (mesmo ima da unidade de sensores, preso com M4 x 10 autoatarraxante)
FEET = [(16, 16), (76, 16), (16, 70), (76, 70)]
FOOT_R, FOOT_H = 13.0, 8.0

LID_BOSSES = [(7, 7), (W - 7, 7), (7, D - 7), (W - 7, D - 7)]

# protoboard 400 pontos encostada na parede de tras
BB_L, BB_W, BB_H = 83.0, 55.0, 8.5
BB_X0 = (W - BB_L) / 2
BB_Y0 = D - WALL - BB_W
BB_TOP = FLOOR_T + BB_H
CLAMP_X = [33.0, 59.0]               # entre os blocos dos P2

# ESP32 DevKit sobre a protoboard, USB-C virado para a parede esquerda
ESP_L, ESP_W = 51.0, 28.0
ESP_Y_C = BB_Y0 + BB_W / 2
ESP_PCB_Z = BB_TOP + 2.5
USB_Z = ESP_PCB_Z + 1.6 + 1.6
USB_WIN = (15.0, 10.0)

# borneira 2,54 mm da protoboard: sai pela parede de tras e recebe o cabo da unidade de sensores
MAIN_TB_X0, MAIN_TB_N, MAIN_TB_PITCH = 14.0, 12, 2.54
MAIN_TB_LEN = MAIN_TB_N * MAIN_TB_PITCH + 1.5
MAIN_TB_H = 13.0
MAIN_TB_OUT = 6.0

# adaptadores P2 -> borne 4 vias na parede da frente
P2_X = [20.0, 46.0, 72.0]
P2_Z = FLOOR_T + P2_BLOCK[1] / 2


def main_tb_slot():
    return box(MAIN_TB_X0 - 2, D - WALL - 1, BB_TOP - 0.5, MAIN_TB_X0 + MAIN_TB_LEN + 2, D + 1, WALL_TOP + 1)


def make_box():
    b = fillet_z(box(0, 0, 0, W, D, WALL_TOP), CORNER_R)
    b = b.cut(fillet_z(box(WALL, WALL, FLOOR_T, W - WALL, D - WALL, WALL_TOP + 1), 1.0))
    for x, y in FEET:
        foot = cyl(FOOT_R, FOOT_H, (x, y, -FOOT_H))
        b = b.fuse(foot)
        b = b.cut(cyl(MAG_D / 2 + 0.15, MAG_T + 0.01, (x, y, -FOOT_H - 0.01)))
        b = b.cut(cyl(1.75, 5.5, (x, y, -FOOT_H + MAG_T)))     # guia p/ M4 autoatarraxante
    for x, y in LID_BOSSES:
        wx = WALL if x < W / 2 else W - WALL
        wy = WALL if y < D / 2 else D - WALL
        b = b.fuse(corner_boss(x, y, wx, wy, WALL_TOP))
    for cx in CLAMP_X:   # torres das presilhas da protoboard
        post = box(cx - 2.5, BB_Y0 - 6, FLOOR_T, cx + 2.5, BB_Y0, BB_TOP)
        b = b.fuse(post.cut(cyl(M3_TAP / 2, BB_H + 1, (cx, BB_Y0 - 3, FLOOR_T))))

    for x in P2_X:
        for c in p2_slot_cutters(x, P2_Z, WALL_TOP):
            b = b.cut(c)
    b = b.cut(main_tb_slot())
    usb = cq.Workplane().add(
        box(-1, ESP_Y_C - USB_WIN[0] / 2, USB_Z - USB_WIN[1] / 2, WALL + 1, ESP_Y_C + USB_WIN[0] / 2, USB_Z + USB_WIN[1] / 2)
    ).edges("|X").fillet(2.0).val()
    b = b.cut(usb)
    b = engrave(b, "USB-C", (0, ESP_Y_C, USB_Z - USB_WIN[1] / 2 - 4.5), (0, -1, 0), (-1, 0, 0), size=4.0)
    return b.clean()


def make_clamp(cx):
    c = box(cx - 2.5, BB_Y0 - 6, BB_TOP, cx + 2.5, BB_Y0 + 3, BB_TOP + 2)
    return c.cut(cyl(M3_CLEAR / 2, 3, (cx, BB_Y0 - 3, BB_TOP - 0.5)))


def make_lid():
    lid = fillet_z(box(0, 0, WALL_TOP, W, D, WALL_TOP + LID_T), CORNER_R)
    lid = cq.Workplane().add(lid).faces(">Z").edges().chamfer(0.8).val()
    lid = cq.Workplane().add(lid).faces(">Z").workplane(centerOption="CenterOfBoundBox") \
        .pushPoints([(x - W / 2, y - D / 2) for x, y in LID_BOSSES]).cskHole(M3_CLEAR, 6.5, 90).val()
    for i in range(6):   # ventilacao sobre o ESP32
        y = ESP_Y_C - 12 + i * 4.5
        lid = lid.cut(cq.Workplane().add(box(18, y, WALL_TOP - 1, 50, y + 2, WALL_TOP + LID_T + 1))
                      .edges("|Z").fillet(0.9).val())
    top = WALL_TOP + LID_T
    for i, x in enumerate(P2_X):
        lid = lid.fuse(p2_tongue(x, P2_Z, WALL_TOP))
        lid = engrave(lid, f"CT{i + 1}", (x, 10, top), (1, 0, 0), (0, 0, 1), size=5)
    slot = main_tb_slot().BoundingBox()
    lid = lid.fuse(box(slot.xmin + 0.15, D - WALL + 0.1, BB_TOP + MAIN_TB_H + 0.5, slot.xmax - 0.15, D - 0.1, WALL_TOP))
    lid = engrave(lid, "SENSOR", ((slot.xmin + slot.xmax) / 2, D - 10, top), (1, 0, 0), (0, 0, 1), size=5)
    return lid.clean()


def components():
    parts = []
    for x, y in FEET:
        parts.append(("ima", magnet(x, y, -FOOT_H), IMA))
    parts.append(("protoboard", box(BB_X0, BB_Y0, FLOOR_T, BB_X0 + BB_L, BB_Y0 + BB_W, BB_TOP)
                  .cut(box(BB_X0 + 3, ESP_Y_C - 1, BB_TOP - 1.5, BB_X0 + BB_L - 3, ESP_Y_C + 1, BB_TOP + 1)),
                  (0.95, 0.95, 0.93)))
    ex0 = BB_X0 - 0.5
    parts.append(("esp32_pcb", box(ex0, ESP_Y_C - ESP_W / 2, ESP_PCB_Z, ex0 + ESP_L, ESP_Y_C + ESP_W / 2,
                                   ESP_PCB_Z + 1.6), PRETO))
    parts.append(("esp32_modulo", box(ex0 + 30, ESP_Y_C - 9, ESP_PCB_Z + 1.6, ex0 + ESP_L - 2, ESP_Y_C + 9,
                                      ESP_PCB_Z + 4.7), (0.75, 0.76, 0.78)))
    parts.append(("usb_c", box(ex0 - 0.3, ESP_Y_C - 4.5, ESP_PCB_Z + 1.6, ex0 + 7.2, ESP_Y_C + 4.5,
                               ESP_PCB_Z + 4.8), (0.85, 0.85, 0.85)))
    tb = box(MAIN_TB_X0, D - WALL - 6, BB_TOP, MAIN_TB_X0 + MAIN_TB_LEN, D + MAIN_TB_OUT, BB_TOP + MAIN_TB_H)
    for i in range(MAIN_TB_N):
        sx = MAIN_TB_X0 + 1.5 + MAIN_TB_PITCH * (i + 0.5)
        tb = tb.cut(cyl(0.9, 1.0, (sx, D + MAIN_TB_OUT - 4, BB_TOP + MAIN_TB_H - 0.99)))
    parts.append(("borneira_principal", tb, VERDE))
    for x in P2_X:
        parts += p2_adapter(x, P2_Z)
    return parts


GROUPS = {"protoboard", "esp32_pcb", "esp32_modulo", "usb_c", "borneira_principal", "presilha"}


def build():
    parts = [("caixa", make_box(), NAVY, "caixa"), ("tampa", make_lid(), LARANJA, "tampa")]
    for i, cx in enumerate(CLAMP_X):
        parts.append((f"presilha_{i + 1}", make_clamp(cx), LARANJA, "protoboard"))
    for n, s, c in components():
        parts.append((n, s, c, "protoboard" if n in GROUPS else "caixa"))
    return parts


if __name__ == "__main__":
    parts = build()
    print("Unidade principal:")
    check_interferences(parts)
    export_unit(parts, Path(__file__).parent / "out" / "principal", "unidade_principal",
                printed={"caixa": False, "tampa": True, "presilha_1": False})
