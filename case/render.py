"""Gera imagens PNG das duas unidades com VTK, sem abrir janela."""

from pathlib import Path

import vtk

import unidade_principal as principal
import unidade_sensor as sensor
from common import box

EXPLODE_SENSOR = {"base": 0.0, "capa": 45.0}
EXPLODE_PRINCIPAL = {"caixa": 0.0, "protoboard": 45.0, "tampa": 95.0}


def actor_for(shape, color, offset=(0.0, 0.0, 0.0)):
    poly = shape.toVtkPolyData(0.05, 0.2, normals=True)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputData(poly)
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    prop = actor.GetProperty()
    prop.SetColor(*color)
    prop.SetSpecular(0.25)
    prop.SetSpecularPower(30)
    prop.SetDiffuse(0.85)
    prop.SetAmbient(0.18)
    actor.SetPosition(*offset)
    return actor


def render(actors, path, azimuth, elevation, size=(1600, 1200), zoom=1.15):
    ren = vtk.vtkRenderer()
    ren.SetBackground(1, 1, 1)
    for a in actors:
        ren.AddActor(a)
    win = vtk.vtkRenderWindow()   # FXAA deixa a imagem preta no modo offscreen do Windows
    win.SetOffScreenRendering(1)
    win.SetMultiSamples(8)
    win.SetSize(*size)
    win.AddRenderer(ren)

    cam = ren.GetActiveCamera()
    cam.SetFocalPoint(0, 0, 0)
    cam.SetPosition(0, -1, 0)
    cam.SetViewUp(0, 0, 1)
    cam.Azimuth(azimuth)
    cam.Elevation(elevation)
    cam.OrthogonalizeViewUp()
    ren.ResetCamera()
    cam.Zoom(zoom)

    kit = vtk.vtkLightKit()
    kit.SetKeyLightIntensity(0.95)
    kit.AddLightsToRenderer(ren)

    win.Render()
    grab = vtk.vtkWindowToImageFilter()
    grab.SetInput(win)
    grab.SetInputBufferTypeToRGB()
    grab.ReadFrontBufferOff()
    grab.Update()
    writer = vtk.vtkPNGWriter()
    writer.SetFileName(str(path))
    writer.SetInputConnection(grab.GetOutputPort())
    writer.Write()
    print("Imagem:", path)


if __name__ == "__main__":
    out = Path(__file__).parent / "out" / "imagens"
    out.mkdir(parents=True, exist_ok=True)
    ps = sensor.build()
    pp = principal.build()

    # as duas unidades lado a lado
    shift = (-principal.W - 40, 0, sensor.BASE_T - (-principal.FOOT_H))
    both = [actor_for(s, c) for _, s, c, _ in ps] + [actor_for(s, c, shift) for _, s, c, _ in pp]
    render(both, out / "conjunto.png", azimuth=30, elevation=25, size=(1800, 1000), zoom=1.2)

    render([actor_for(s, c) for _, s, c, _ in ps], out / "sensor_montado.png", azimuth=35, elevation=25)
    render([actor_for(s, c, (0, 0, EXPLODE_SENSOR[g])) for _, s, c, g in ps],
           out / "sensor_explodido.png", azimuth=35, elevation=22, size=(1300, 1500), zoom=1.1)
    render([actor_for(s, c) for _, s, c, g in ps if g == "base"], out / "sensor_inferior.png",
           azimuth=30, elevation=-50, zoom=1.3)
    keep = box(-50, sensor.PROBE_Y, -50, 200, 200, 200)   # corte no eixo do poco termico
    section = []
    for n, s, c, _ in ps:
        if n in ("base_metalica", "ima", "sonda_inox", "capa"):
            cut = s.intersect(keep)
            if cut.Volume() > 0.01:
                section.append(actor_for(cut, (0.85, 0.25, 0.18) if n == "sonda_inox" else c))
    render(section, out / "sensor_corte_sonda.png", azimuth=-20, elevation=20, zoom=1.3)

    render([actor_for(s, c) for _, s, c, _ in pp], out / "principal_montado.png", azimuth=35, elevation=25)
    render([actor_for(s, c) for _, s, c, _ in pp], out / "principal_traseira.png", azimuth=215, elevation=25)
    render([actor_for(s, c) for _, s, c, g in pp if g != "tampa"], out / "principal_sem_tampa.png",
           azimuth=20, elevation=55)
    render([actor_for(s, c, (0, 0, EXPLODE_PRINCIPAL[g])) for _, s, c, g in pp],
           out / "principal_explodido.png", azimuth=35, elevation=22, size=(1300, 1500), zoom=1.1)
