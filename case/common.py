"""Pecas e utilitarios compartilhados pelas duas unidades (sensor e principal)."""

from pathlib import Path

import cadquery as cq

V = cq.Vector

WALL = 3.0
CORNER_R = 4.0
M3_CLEAR = 3.4
M3_TAP = 2.5
M2_TAP = 1.6
M4_TAP = 3.3
INSERT_D = 4.0      # furo p/ inserto M3 a quente (M3 x 5,7)
INSERT_DEPTH = 6.0
MAG_D, MAG_T = 20.0, 5.0   # ima neodimio com furo escareado M4

# adaptador P2 femea -> borne 4 vias (Goobay 59200 e similares: ~36 mm, bornes a cada 5,0 mm)
P2_NOSE_D, P2_NOSE_L = 9.0, 10.0
P2_BODY_L = 16.0
P2_BODY_NARROW = (12.0, 11.0)   # largura x altura sobre as cristas, junto ao bico
P2_BODY_WIDE = (20.0, 12.5)     # junto ao borne
P2_GROOVE = 1.0                 # profundidade da ranhura de cada lado
P2_PITCH = 2.0                  # crista 1 mm + ranhura 1 mm
P2_BLOCK_L = 10.0
P2_BLOCK = (20.0, 12.5)         # 4 bornes x 5,0 mm
P2_LEN = P2_NOSE_L + P2_BODY_L + P2_BLOCK_L
P2_FIN = 0.9                    # aleta da parede que entra na ranhura
P2_CLEAR = 0.15

ALUMINIO = (0.72, 0.74, 0.77)
NAVY = (0.11, 0.18, 0.33)
LARANJA = (0.91, 0.45, 0.13)
CINZA = (0.30, 0.33, 0.38)
VERDE = (0.24, 0.74, 0.35)
PRETO = (0.12, 0.12, 0.12)
IMA = (0.62, 0.64, 0.67)
INOX = (0.82, 0.84, 0.86)
LATAO = (0.80, 0.66, 0.30)


def box(x0, y0, z0, x1, y1, z1):
    return cq.Solid.makeBox(x1 - x0, y1 - y0, z1 - z0, V(x0, y0, z0))


def cyl(r, h, p, d=(0, 0, 1)):
    return cq.Solid.makeCylinder(r, h, V(*p), V(*d))


def fillet_z(solid, r):
    return cq.Workplane().add(solid).edges("|Z").fillet(r).val()


def fuse(*shapes):
    out = shapes[0]
    for s in shapes[1:]:
        out = out.fuse(s)
    return out.clean()


def corner_boss(cx, cy, wx, wy, z_top, h_flat=10.0, h_chamfer=8.0, size=8.0):
    """Ressalto de canto p/ inserto M3; embaixo vira piramide ate o canto (wx, wy) e imprime sem suporte."""
    h = size / 2
    x0, x1, y0, y1 = cx - h, cx + h, cy - h, cy + h
    zf = z_top - h_flat
    flat = box(x0, y0, zf, x1, y1, z_top)
    top = cq.Wire.makePolygon([V(x0, y0, zf), V(x1, y0, zf), V(x1, y1, zf), V(x0, y1, zf)], close=True)
    sx = 1.0 if wx < cx else -1.0
    sy = 1.0 if wy < cy else -1.0
    zb = zf - h_chamfer
    bot = cq.Wire.makePolygon(
        [V(wx, wy, zb), V(wx + sx, wy, zb), V(wx + sx, wy + sy, zb), V(wx, wy + sy, zb)], close=True)
    boss = flat.fuse(cq.Solid.makeLoft([bot, top], True))
    return boss.cut(cyl(INSERT_D / 2, INSERT_DEPTH, (cx, cy, z_top - INSERT_DEPTH)))


def engrave(solid, text, origin, x_dir, normal, size=4.5, depth=0.6):
    plane = cq.Plane(origin=origin, xDir=x_dir, normal=normal)
    txt = cq.Workplane(plane).text(text, size, -depth, combine=False, halign="center", valign="center").val()
    return solid.cut(txt)


def magnet(x, y, z0):
    """Ima com furo escareado, face de contato em z0 (para baixo)."""
    m = cyl(MAG_D / 2, MAG_T, (x, y, z0))
    return m.cut(cq.Solid.makeCone(4.4, 2.0, 2.4, V(x, y, z0 - 0.01)))


# ---------------------------------------------------------------- adaptador P2 preso na parede da frente (Y 0..WALL)
def _p2_crest(s):
    """Medida sobre as cristas a uma distancia s do inicio do corpo (s = 0 na face externa da parede)."""
    t = min(max(s / P2_BODY_L, 0.0), 1.0)
    (w0, h0), (w1, h1) = P2_BODY_NARROW, P2_BODY_WIDE
    return w0 + (w1 - w0) * t, h0 + (h1 - h0) * t


def p2_wall_dims():
    """Medidas usadas no recorte da parede: fundo da ranhura no plano da aleta e maior crista dentro da parede."""
    cw, ch = _p2_crest(WALL / 2)
    pocket_w, pocket_h = _p2_crest(WALL)
    return (cw - 2 * P2_GROOVE, ch - 2 * P2_GROOVE), (pocket_w, pocket_h)


def p2_adapter(x, z_axis):
    """Corpo do adaptador com o bico para fora (Y < 0); a 1a ranhura cai no meio da parede."""
    parts = []
    nose = cyl(P2_NOSE_D / 2, P2_NOSE_L, (x, 0, z_axis), (0, -1, 0))
    nose = nose.cut(cyl(1.8, P2_NOSE_L - 1, (x, -P2_NOSE_L - 0.01, z_axis), (0, 1, 0)))
    slices = []
    n = int(round(P2_BODY_L / (P2_PITCH / 2)))
    for i in range(n):
        s0 = i * P2_PITCH / 2
        cw, ch = _p2_crest(s0 + P2_PITCH / 4)
        if i % 2:   # ranhura
            cw, ch = cw - 2 * P2_GROOVE, ch - 2 * P2_GROOVE
        slices.append(box(x - cw / 2, s0, z_axis - ch / 2, x + cw / 2, s0 + P2_PITCH / 2, z_axis + ch / 2))
    parts.append(("p2_adaptador", fuse(nose, *slices), PRETO))
    bw, bh = P2_BLOCK
    y0 = P2_BODY_L
    block = box(x - bw / 2, y0, z_axis - bh / 2, x + bw / 2, y0 + P2_BLOCK_L, z_axis + bh / 2)
    for i in range(4):
        sx = x - bw / 2 + 2.5 + 5.0 * i
        block = block.cut(cyl(1.2, 1.0, (sx, y0 + P2_BLOCK_L / 2, z_axis + bh / 2 - 0.99)))
    parts.append(("p2_borne", block, VERDE))
    return parts


def p2_slot_cutters(x, z_axis, z_top):
    (rw, rh), (pw, ph) = p2_wall_dims()
    fin_y0 = (WALL - P2_FIN) / 2
    fin_y1 = fin_y0 + P2_FIN
    hw_root, hw_pocket = rw / 2 + P2_CLEAR, pw / 2 + 0.3
    z_pocket_top = z_axis + ph / 2 + 0.3
    return [
        box(x - hw_root, -1, z_axis - rh / 2 - P2_CLEAR, x + hw_root, WALL + 1, z_top + 1),
        box(x - hw_pocket, -1, z_axis - ph / 2 - 0.3, x + hw_pocket, fin_y0, z_top + 1),
        box(x - hw_pocket, fin_y1, z_axis - ph / 2 - 0.3, x + hw_pocket, WALL + 1, z_top + 1),
        box(x - hw_pocket, -1, z_pocket_top, x + hw_pocket, WALL + 1, z_top + 1),
    ]


def p2_tongue(x, z_axis, z_top):
    """Lingueta da tampa: fecha a abertura por cima e encosta no fundo da ranhura."""
    (rw, rh), (pw, ph) = p2_wall_dims()
    fin_y0 = (WALL - P2_FIN) / 2
    hw = pw / 2 + 0.3 - P2_CLEAR
    z_pocket_top = z_axis + ph / 2 + 0.3
    t = box(x - hw, 0.1, z_pocket_top + 0.1, x + hw, WALL - 0.1, z_top)
    return t.fuse(box(x - rw / 2 + 0.1, fin_y0 + 0.05, z_axis + rh / 2 + 0.1,
                      x + rw / 2 - 0.1, fin_y0 + P2_FIN - 0.05, z_pocket_top + 0.2))


# ---------------------------------------------------------------- verificacao e exportacao
def check_interferences(parts, min_volume=0.05):
    """Mostra pares de pecas que se sobrepoem (encostar nao conta)."""
    found = False
    for i in range(len(parts)):
        for j in range(i + 1, len(parts)):
            a, b = parts[i], parts[j]
            ba, bb = a[1].BoundingBox(), b[1].BoundingBox()
            if (ba.xmax < bb.xmin or bb.xmax < ba.xmin or ba.ymax < bb.ymin or bb.ymax < ba.ymin
                    or ba.zmax < bb.zmin or bb.zmax < ba.zmin):
                continue
            vol = a[1].intersect(b[1]).Volume()
            if vol > min_volume:
                found = True
                print(f"  INTERFERENCIA: {a[0]} x {b[0]} = {vol:.2f} mm3")
    if not found:
        print("  sem interferencias entre as pecas")


def export_unit(parts, out_dir: Path, assembly_name, printed, machined=()):
    """printed: {nome: virar_de_cabeca_para_baixo}. Gera STEP da montagem, STL das impressas e STEP das usinadas."""
    out_dir.mkdir(parents=True, exist_ok=True)
    assy = cq.Assembly(name=assembly_name)
    for i, (name, shape, color, _) in enumerate(parts):
        assy.add(shape, name=f"{name}_{i}", color=cq.Color(*color))
    assy.export(str(out_dir / f"{assembly_name}_montagem.step"))

    byname = {name: shape for name, shape, _, _ in parts}
    for name in machined:
        cq.exporters.export(byname[name], str(out_dir / f"{name}.step"))
    for name, flip in printed.items():
        shape = byname[name]
        if flip:
            c = shape.Center()
            shape = shape.rotate(c, c + V(1, 0, 0), 180)
        bb = shape.BoundingBox()
        shape = shape.translate(V(-bb.xmin, -bb.ymin, -bb.zmin))
        cq.exporters.export(shape, str(out_dir / f"{name.removesuffix('_1')}.stl"),
                            tolerance=0.02, angularTolerance=0.1)
