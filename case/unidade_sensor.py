"""Unidade de sensores: vai presa no motor pelos imas.

Base de aluminio com 4 imas embutidos e poco termico para a sonda DS18B20; placa perfurada 7 x 5 cm
com o GY-291 em 3 espacadores de latao; capa impressa que abraca a base e e presa por 4 parafusos
laterais. A borneira de 12 vias da perfurada sai pela lateral direita, junto com a sonda.

Eixos: X esquerda -> direita (borneira em X+), Y frente -> tras, Z para cima. Medidas em mm.
"""

from pathlib import Path

import cadquery as cq

from common import (
    ALUMINIO, IMA, INOX, LATAO, M2_TAP, M3_CLEAR, M3_TAP, M4_TAP, MAG_D, MAG_T, NAVY, PRETO, VERDE, WALL,
    box, check_interferences, cyl, engrave, export_unit, fillet_z, fuse, magnet,
)

# ---------------------------------------------------------------- base de aluminio
BASE_L, BASE_W, BASE_T = 78.0, 60.0, 10.0
BASE_R = 3.0
MAGNETS = [(22, 16), (58, 16), (22, 38), (58, 38)]
M4_TAP_DEPTH = 4.0
PROBE_D, PROBE_LEN = 6.0, 50.0       # sonda inox 6 x 50
PROBE_Y, PROBE_Z = 54.0, 4.5         # poco termico ao longo de X, entrando pela face direita
PROBE_BORE_D, PROBE_BORE_DEPTH = 6.3, 52.0
PROBE_SETSCREW_X = 70.0              # parafuso sem cabeca M3 pela face de tras
COVER_SCREWS = [("frente", 20.0), ("frente", 58.0), ("esquerda", 15.0), ("esquerda", 45.0)]
COVER_SCREW_Z = 7.0

# ---------------------------------------------------------------- placa perfurada (girada 180 graus em relacao a foto)
PERF_L, PERF_W, PERF_T = 70.0, 50.0, 1.6
PERF_X0 = BASE_L - PERF_L            # borda da borneira alinhada com a face direita da base
PERF_Y0 = (BASE_W - PERF_W) / 2
STANDOFF_H = 8.0
PERF_Z = BASE_T + STANDOFF_H
HOLE_INSET = 2.5
PERF_TB_PITCH, PERF_TB_N = 3.5, 12
PERF_TB_LEN = PERF_TB_PITCH * PERF_TB_N
PERF_TB_Y0 = PERF_Y0 + 7.0
PERF_TB_OVERHANG, PERF_TB_DEPTH, PERF_TB_H = 9.0, 14.0, 14.0
GY291 = (PERF_X0 + 32.0, PERF_Y0 + 8.7, 20.3, 15.5)   # x0, y0, comprimento, largura


def perf_holes():
    """3 pontos: o furo de tras-direita fica embaixo da borneira."""
    x0, x1 = PERF_X0 + HOLE_INSET, PERF_X0 + PERF_L - HOLE_INSET
    y0, y1 = PERF_Y0 + HOLE_INSET, PERF_Y0 + PERF_W - HOLE_INSET
    return [(x0, y0), (x0, y1), (x1, y0)]


# ---------------------------------------------------------------- capa impressa
COVER_CLEAR = 0.2
COVER_Z0 = 0.5                       # a capa nao encosta no motor, so a base
COVER_TOP_T = 2.5
COVER_TOP = PERF_Z + PERF_T + PERF_TB_H + 1.4 + COVER_TOP_T
LIP_W, LIP_T = 2.0, 2.0              # aba interna que apoia a capa no topo da base
TB_NOTCH = (PERF_TB_Y0 - 1.0, PROBE_Y + PROBE_BORE_D / 2 + 0.8)

CI, CO = -COVER_CLEAR, -COVER_CLEAR - WALL   # faces interna/externa da capa (lado X0/Y0)
CXI, CYI = BASE_L + COVER_CLEAR, BASE_W + COVER_CLEAR
CXO, CYO = CXI + WALL, CYI + WALL


def make_base():
    b = fillet_z(box(0, 0, 0, BASE_L, BASE_W, BASE_T), BASE_R)
    b = cq.Workplane().add(b).faces("<Z").edges().chamfer(0.5).val()
    for x, y in MAGNETS:
        b = b.cut(cyl(MAG_D / 2 + 0.2, MAG_T + 0.01, (x, y, -0.01)))
        b = b.cut(cyl(M4_TAP / 2, M4_TAP_DEPTH, (x, y, MAG_T)))
    b = b.cut(cyl(PROBE_BORE_D / 2, PROBE_BORE_DEPTH, (BASE_L + 0.01, PROBE_Y, PROBE_Z), (-1, 0, 0)))
    b = b.cut(cyl(M3_TAP / 2, BASE_W - PROBE_Y + 0.5, (PROBE_SETSCREW_X, BASE_W + 0.01, PROBE_Z), (0, -1, 0)))
    for side, pos in COVER_SCREWS:
        if side == "frente":
            b = b.cut(cyl(M3_TAP / 2, 6.0, (pos, -0.01, COVER_SCREW_Z), (0, 1, 0)))
        else:
            b = b.cut(cyl(M3_TAP / 2, 6.0, (-0.01, pos, COVER_SCREW_Z), (1, 0, 0)))
    for x, y in perf_holes():
        b = b.cut(cyl(M2_TAP / 2, 5.0, (x, y, BASE_T - 5.0)))
    return b


def make_cover():
    outer = fillet_z(box(CO, CO, COVER_Z0, CXO, CYO, COVER_TOP), BASE_R + WALL)
    inner = fillet_z(box(CI, CI, COVER_Z0 - 1, CXI, CYI, COVER_TOP - COVER_TOP_T), BASE_R + COVER_CLEAR)
    cover = outer.cut(inner)
    cover = cq.Workplane().add(cover).faces(">Z").edges().chamfer(0.8).val()

    # abas internas (frente, tras, esquerda) que apoiam a capa no topo da base; chanfro p/ imprimir sem suporte
    z0 = BASE_T
    for x0, y0, x1, y1, ax in ((CI, CI, CXI, CI + LIP_W, "y"), (CI, CYI - LIP_W, CXI, CYI, "y"),
                               (CI, CI, CI + LIP_W, CYI, "x")):
        lip = box(x0, y0, z0, x1, y1, z0 + LIP_T)
        if ax == "y":
            yw = y0 if y0 == CI else y1
            yi = y1 if y0 == CI else y0
            pts = [(yw, z0 + LIP_T), (yi, z0 + LIP_T), (yw, z0 + LIP_T + LIP_W)]
            wedge = cq.Workplane("YZ", origin=(x0, 0, 0)).polyline(pts).close().extrude(x1 - x0).val()
        else:
            pts = [(CI, z0 + LIP_T), (CI + LIP_W, z0 + LIP_T), (CI, z0 + LIP_T + LIP_W)]
            wedge = (cq.Workplane("XZ", origin=(0, y1, 0)).polyline(pts).close()
                     .extrude(y1 - y0).val())
        cover = cover.fuse(lip.fuse(wedge))

    # abertura da borneira e da sonda na lateral direita (aberta por baixo, a capa desce por cima delas)
    cover = cover.cut(box(CXI - 1, TB_NOTCH[0], COVER_Z0 - 1, CXO + 1, TB_NOTCH[1], PERF_Z + PERF_T + PERF_TB_H + 0.5))
    # parafusos da capa e acesso a trava da sonda
    for side, pos in COVER_SCREWS:
        if side == "frente":
            cover = cover.cut(cyl(M3_CLEAR / 2, WALL + 1, (pos, CO - 0.5, COVER_SCREW_Z), (0, 1, 0)))
        else:
            cover = cover.cut(cyl(M3_CLEAR / 2, WALL + 1, (CO - 0.5, pos, COVER_SCREW_Z), (1, 0, 0)))
    cover = cover.cut(cyl(1.9, WALL + 1, (PROBE_SETSCREW_X, CYO + 0.5, PROBE_Z), (0, -1, 0)))
    cover = engrave(cover, "SENSOR", (BASE_L / 2, BASE_W / 2, COVER_TOP), (1, 0, 0), (0, 0, 1), size=8)
    return cover.clean()


def components():
    parts = []
    for x, y in MAGNETS:
        parts.append(("ima", magnet(x, y, 0), IMA))
    parts.append(("sonda_inox", cyl(PROBE_D / 2, PROBE_LEN, (BASE_L, PROBE_Y, PROBE_Z), (-1, 0, 0)), INOX))
    tb_face_x = PERF_X0 + PERF_L + PERF_TB_OVERHANG
    cz = PERF_Z + PERF_T + 5
    cable = fuse(
        cyl(2.2, 16, (BASE_L, PROBE_Y, PROBE_Z), (1, 0, 0)),
        cyl(2.2, cz - PROBE_Z, (BASE_L + 16, PROBE_Y, PROBE_Z)),
        cyl(2.2, 6, (BASE_L + 16, PROBE_Y, cz), (0, -1, 0)),
        cyl(2.2, BASE_L + 16 - tb_face_x, (BASE_L + 16, PROBE_Y - 6, cz), (-1, 0, 0)),
    )
    parts.append(("cabo_sonda", cable, PRETO))

    for x, y in perf_holes():
        parts.append(("espacador", cyl(2.0, STANDOFF_H, (x, y, BASE_T)), LATAO))
    parts.append(("perfurada", box(PERF_X0, PERF_Y0, PERF_Z, PERF_X0 + PERF_L, PERF_Y0 + PERF_W, PERF_Z + PERF_T),
                  (0.78, 0.52, 0.22)))
    gx, gy, gl, gw = GY291
    gz = PERF_Z + PERF_T + 2.5
    parts.append(("gy291", box(gx, gy, gz, gx + gl, gy + gw, gz + 1.6), (0.16, 0.30, 0.66)))
    parts.append(("adxl345", box(gx + 8, gy + 6, gz + 1.6, gx + 11, gy + 9, gz + 2.6), PRETO))
    tz = PERF_Z + PERF_T
    tb = box(tb_face_x - PERF_TB_DEPTH, PERF_TB_Y0, tz, tb_face_x, PERF_TB_Y0 + PERF_TB_LEN, tz + PERF_TB_H)
    for i in range(PERF_TB_N):
        sy = PERF_TB_Y0 + PERF_TB_PITCH * (i + 0.5)
        tb = tb.cut(cyl(1.0, 1.0, (tb_face_x - 4.0, sy, tz + PERF_TB_H - 0.99)))
    parts.append(("borneira_perfurada", tb, VERDE))
    return parts


def build():
    """Lista de (nome, solido, cor, grupo); o grupo define o deslocamento na vista explodida."""
    parts = [("base_metalica", make_base(), ALUMINIO, "base"), ("capa", make_cover(), NAVY, "capa")]
    parts += [(n, s, c, "base") for n, s, c in components()]
    return parts


if __name__ == "__main__":
    parts = build()
    print("Unidade de sensores:")
    check_interferences(parts)
    export_unit(parts, Path(__file__).parent / "out" / "sensor", "unidade_sensor",
                printed={"capa": True}, machined=("base_metalica",))
