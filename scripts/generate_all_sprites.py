#!/usr/bin/env python3
"""
PixelForge AI - Game Asset Generator
Generates all 22 requested game sprites in authentic retro pixel art style.
"""

import os
import math
import shutil
from PIL import Image, ImageDraw

ASSETS_DIR = "/home/igor/Documentos/Projetos/pixelforgeAI/assets"
TECH_QUEST_DIR = "/home/igor/Documentos/Projetos/flutter-jail/home/dev/tech-quest/assets/images"

os.makedirs(ASSETS_DIR, exist_ok=True)
os.makedirs(TECH_QUEST_DIR, exist_ok=True)

def hex_to_rgba(h, a=255):
    h = h.lstrip('#')
    if len(h) == 6:
        r, g, b = tuple(int(h[i:i+2], 16) for i in (0, 2, 4))
        return (r, g, b, a)
    elif len(h) == 8:
        return tuple(int(h[i:i+2], 16) for i in (0, 2, 4, 6))
    return (0, 0, 0, a)

# =============================================================================
# 1. Building_Guilda (320x256)
# =============================================================================
def generate_building_guilda():
    w, h = 320, 256
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    stone_dark = hex_to_rgba("#222329")
    stone_shadow = hex_to_rgba("#3B3D4A")
    stone_mid = hex_to_rgba("#686B7D")
    stone_light = hex_to_rgba("#9FA2B4")
    stone_hi = hex_to_rgba("#D5D7E3")
    
    bronze_shadow = hex_to_rgba("#593616")
    bronze_mid = hex_to_rgba("#8C5824")
    bronze_light = hex_to_rgba("#C2823C")
    bronze_hi = hex_to_rgba("#E8B668")
    
    gold_circuit = hex_to_rgba("#FFD700")
    gold_glow = hex_to_rgba("#FFE875")
    
    wood_door = hex_to_rgba("#2E1B0E")
    wood_door_light = hex_to_rgba("#54341E")
    parchment = hex_to_rgba("#F2E4C4")

    # 1. Base / Escadaria monumental (y: 220..255)
    for step, y_pos in enumerate(range(240, 256, 4)):
        pad = step * 4
        d.rectangle([10 - pad, y_pos, 310 + pad, y_pos + 3], fill=stone_shadow, outline=stone_dark)
        d.line([10 - pad, y_pos, 310 + pad, y_pos], fill=stone_light)

    # 2. Corpo do Templo (Paredes de pedra lavrada: x: 30..290, y: 100..240)
    d.rectangle([30, 100, 290, 240], fill=stone_mid, outline=stone_dark)
    # Textura de blocos de pedra
    for by in range(104, 240, 12):
        d.line([30, by, 290, by], fill=stone_shadow)
        offset = 12 if (by // 12) % 2 == 0 else 0
        for bx in range(30 + offset, 290, 24):
            d.line([bx, by, bx, by + 12], fill=stone_shadow)

    # 3. Grande Cúpula Superior com Trilhas de Circuito (x: 90..230, y: 15..95)
    d.pieslice([90, 15, 230, 155], 180, 360, fill=stone_shadow, outline=stone_dark)
    d.pieslice([94, 19, 226, 151], 180, 360, fill=stone_mid)
    d.arc([98, 23, 222, 147], 180, 360, fill=bronze_mid, width=2)
    
    circuit_pts = [
        (160, 20), (160, 50),
        (140, 26), (140, 48), (120, 70),
        (180, 26), (180, 48), (200, 70),
        (160, 50), (135, 65), (135, 85),
        (160, 50), (185, 65), (185, 85),
    ]
    for i in range(0, len(circuit_pts)-1, 2):
        d.line([circuit_pts[i], circuit_pts[i+1]], fill=gold_circuit, width=2)
        d.rectangle([circuit_pts[i+1][0]-2, circuit_pts[i+1][1]-2, circuit_pts[i+1][0]+2, circuit_pts[i+1][1]+2], fill=gold_glow)

    d.rectangle([157, 5, 163, 20], fill=bronze_hi, outline=stone_dark)
    d.ellipse([156, 2, 164, 10], fill=gold_glow)

    # 4. Entablamento e Friso com Engrenagens de Bronze (y: 85..110)
    d.rectangle([20, 85, 300, 105], fill=bronze_mid, outline=stone_dark)
    d.line([20, 85, 300, 85], fill=bronze_hi)
    d.line([20, 105, 300, 105], fill=bronze_shadow)

    for gx in range(35, 290, 25):
        d.ellipse([gx - 6, 90, gx + 6, 102], fill=bronze_light, outline=bronze_shadow)
        d.ellipse([gx - 2, 94, gx + 2, 98], fill=stone_dark)
        d.point([(gx, 88), (gx, 104), (gx-8, 96), (gx+8, 96)], fill=bronze_hi)

    # Frontão triangular neoclássico
    d.polygon([(20, 85), (160, 45), (300, 85)], fill=stone_mid, outline=stone_dark)
    d.line([(20, 85), (160, 45), (300, 85)], fill=stone_hi, width=2)
    d.ellipse([150, 60, 170, 80], fill=bronze_hi, outline=stone_dark)
    d.rectangle([155, 66, 165, 74], fill=parchment, outline=stone_dark)

    # 5. Colunata Jônica (6 colunas)
    col_x = [40, 85, 130, 190, 235, 280]
    for cx in col_x:
        d.rectangle([cx - 9, 105, cx + 9, 113], fill=stone_hi, outline=stone_dark)
        d.ellipse([cx - 11, 106, cx - 6, 112], fill=bronze_light, outline=stone_dark)
        d.ellipse([cx + 6, 106, cx + 11, 112], fill=bronze_light, outline=stone_dark)
        d.rectangle([cx - 7, 113, cx + 7, 235], fill=stone_light, outline=stone_dark)
        d.line([cx - 4, 114, cx - 4, 234], fill=stone_hi)
        d.line([cx - 1, 114, cx - 1, 234], fill=stone_mid)
        d.line([cx + 2, 114, cx + 2, 234], fill=stone_shadow)
        d.line([cx + 5, 114, cx + 5, 234], fill=stone_dark)
        d.rectangle([cx - 9, 235, cx + 9, 242], fill=stone_hi, outline=stone_dark)

    # 6. Grande Portal em Arco Central
    d.arc([130, 130, 190, 190], 180, 360, fill=bronze_hi, width=4)
    d.rectangle([132, 160, 188, 240], fill=wood_door, outline=stone_dark)
    d.pieslice([132, 132, 188, 188], 180, 360, fill=wood_door)
    d.line([160, 145, 160, 240], fill=bronze_shadow, width=2)
    d.rectangle([138, 175, 155, 205], fill=wood_door_light, outline=stone_dark)
    d.rectangle([165, 175, 182, 205], fill=wood_door_light, outline=stone_dark)
    d.rectangle([138, 210, 155, 235], fill=wood_door_light, outline=stone_dark)
    d.rectangle([165, 210, 182, 235], fill=wood_door_light, outline=stone_dark)

    # Brasão da Pena e Pergaminho
    d.ellipse([148, 118, 172, 142], fill=bronze_hi, outline=stone_dark)
    d.rectangle([153, 126, 167, 136], fill=parchment, outline=stone_dark)
    d.line([151, 138, 169, 122], fill=gold_circuit, width=2)
    d.point([(170, 121)], fill=gold_glow)

    for wx in [60, 250]:
        d.arc([wx - 10, 145, wx + 10, 165], 180, 360, fill=bronze_shadow, width=2)
        d.rectangle([wx - 10, 155, wx + 10, 195], fill=hex_to_rgba("#1C2330"), outline=stone_dark)
        d.rectangle([wx - 8, 157, wx + 8, 193], fill=hex_to_rgba("#E8A838", 180))
        d.line([wx, 155, wx, 195], fill=stone_dark)
        d.line([wx - 10, 175, wx + 10, 175], fill=stone_dark)

    img.save(os.path.join(ASSETS_DIR, "Building_Guilda.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Building_Guilda.png"), os.path.join(TECH_QUEST_DIR, "Building_Guilda.png"))
    print("✔ Building_Guilda gerado.")

# =============================================================================
# 2. Building_Forja (288x224)
# =============================================================================
def generate_building_forja():
    w, h = 288, 224
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    iron_dark = hex_to_rgba("#17191C")
    iron_shadow = hex_to_rgba("#292D33")
    iron_mid = hex_to_rgba("#454B54")
    iron_light = hex_to_rgba("#707885")
    rivet = hex_to_rgba("#9EA7B5")
    
    brick_dark = hex_to_rgba("#2B1812")
    brick_mid = hex_to_rgba("#4A281E")
    
    copper_dark = hex_to_rgba("#612612")
    copper_mid = hex_to_rgba("#A34B24")
    copper_hot = hex_to_rgba("#FF6A1C")
    copper_incandescent = hex_to_rgba("#FFB443")
    
    steam_color = hex_to_rgba("#DDE8F0", 190)
    steam_core = hex_to_rgba("#FFFFFF", 230)

    # Chaminés Duplas
    for cx in [45, 215]:
        d.rectangle([cx, 30, cx + 28, 110], fill=brick_mid, outline=iron_dark)
        for cy in range(35, 110, 10):
            d.line([cx, cy, cx + 28, cy], fill=brick_dark)
        for ry in [40, 70, 100]:
            d.rectangle([cx - 2, ry, cx + 30, ry + 5], fill=iron_mid, outline=iron_dark)
            d.point([(cx + 4, ry + 2), (cx + 14, ry + 2), (cx + 24, ry + 2)], fill=rivet)
        d.rectangle([cx - 4, 25, cx + 32, 32], fill=iron_dark)
        d.rectangle([cx - 2, 26, cx + 30, 30], fill=iron_light)

        d.ellipse([cx - 8, 2, cx + 18, 24], fill=steam_color)
        d.ellipse([cx + 6, -2, cx + 36, 20], fill=steam_color)
        d.ellipse([cx + 2, 6, cx + 22, 22], fill=steam_core)

    # Galpão Principal
    d.rectangle([20, 80, 268, 215], fill=iron_shadow, outline=iron_dark)

    for px in range(25, 265, 48):
        for py in range(85, 210, 40):
            d.rectangle([px, py, px + 44, py + 36], fill=iron_mid, outline=iron_dark)
            d.line([px + 1, py + 1, px + 43, py + 1], fill=iron_light)
            d.point([(px + 3, py + 3), (px + 41, py + 3), (px + 3, py + 33), (px + 41, py + 33)], fill=rivet)

    # Telhado Dente de Serra
    for sx in range(20, 268, 62):
        d.polygon([(sx, 80), (sx + 40, 60), (sx + 62, 80)], fill=iron_dark)
        d.line([(sx, 80), (sx + 40, 60), (sx + 62, 80)], fill=iron_light, width=2)

    # Portal com forja e pistões
    d.rectangle([104, 120, 184, 215], fill=iron_dark, outline=copper_dark)
    d.rectangle([110, 130, 178, 215], fill=copper_dark)
    d.rectangle([118, 145, 170, 215], fill=copper_hot)
    d.rectangle([126, 165, 162, 215], fill=copper_incandescent)

    # Pistão Esquerdo
    d.rectangle([82, 110, 100, 215], fill=copper_mid, outline=iron_dark)
    d.rectangle([86, 125, 96, 195], fill=copper_hot)
    d.line([88, 125, 88, 195], fill=copper_incandescent, width=2)
    for py in [120, 150, 180, 210]:
        d.rectangle([80, py, 102, py + 4], fill=iron_light, outline=iron_dark)

    # Pistão Direito
    d.rectangle([188, 110, 206, 215], fill=copper_mid, outline=iron_dark)
    d.rectangle([192, 125, 202, 195], fill=copper_hot)
    d.line([194, 125, 194, 195], fill=copper_incandescent, width=2)
    for py in [120, 150, 180, 210]:
        d.rectangle([186, py, 208, py + 4], fill=iron_light, outline=iron_dark)

    # Esteira Mecânica
    d.rectangle([10, 195, 278, 220], fill=iron_dark)
    for rx in range(16, 275, 14):
        d.ellipse([rx, 200, rx + 10, 216], fill=iron_light, outline=iron_shadow)
        d.point([(rx + 5, 208)], fill=rivet)
    for ix in [35, 75, 220, 250]:
        d.rectangle([ix, 190, ix + 18, 199], fill=copper_hot, outline=iron_dark)
        d.line([ix + 2, 192, ix + 16, 192], fill=copper_incandescent)

    img.save(os.path.join(ASSETS_DIR, "Building_Forja.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Building_Forja.png"), os.path.join(TECH_QUEST_DIR, "Building_Forja.png"))
    print("✔ Building_Forja gerado.")

# =============================================================================
# 3. Building_Sentinela (256x224)
# =============================================================================
def generate_building_sentinela():
    w, h = 256, 224
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    navy_black = hex_to_rgba("#0C111C")
    navy_dark = hex_to_rgba("#131D30")
    navy_mid = hex_to_rgba("#1E2E4C")
    navy_light = hex_to_rgba("#314B7A")
    navy_hi = hex_to_rgba("#4B71B5")

    cyan_dark = hex_to_rgba("#006B75")
    cyan_mid = hex_to_rgba("#00B4C4")
    cyan_bright = hex_to_rgba("#00FFFF")
    cyan_glow = hex_to_rgba("#AAFFFF", 180)
    
    hazard_yellow = hex_to_rgba("#FFCC00")
    red_alarm = hex_to_rgba("#FF2233")

    # Torre Principal
    d.polygon([(40, 215), (70, 60), (186, 60), (216, 215)], fill=navy_mid, outline=navy_black)
    for ty in range(65, 210, 25):
        d.line([(45 + (210 - ty) * 0.18, ty), (211 - (210 - ty) * 0.18, ty)], fill=navy_dark, width=2)
        d.line([(45 + (210 - ty) * 0.18, ty + 1), (211 - (210 - ty) * 0.18, ty + 1)], fill=navy_light)

    # Plataforma do Topo
    d.rectangle([60, 48, 196, 60], fill=navy_light, outline=navy_black)
    d.rectangle([64, 50, 192, 54], fill=navy_hi)
    for rx in range(62, 196, 8):
        d.line([rx, 40, rx, 48], fill=navy_light)
    d.line([60, 40, 196, 40], fill=navy_hi, width=2)

    # Parabólica
    d.line([128, 40, 128, 25], fill=navy_light, width=4)
    d.arc([102, 6, 154, 40], 210, 360, fill=navy_light, width=5)
    d.arc([105, 9, 151, 37], 210, 360, fill=cyan_bright, width=2)
    d.line([128, 23, 142, 12], fill=navy_black, width=2)
    d.ellipse([140, 10, 146, 16], fill=red_alarm)
    d.arc([144, 2, 160, 20], 300, 40, fill=cyan_glow, width=2)

    # Refletores Ciano
    d.rectangle([62, 55, 74, 65], fill=navy_dark, outline=navy_black)
    d.ellipse([64, 57, 72, 63], fill=cyan_bright)
    d.polygon([(68, 63), (10, 215), (75, 215)], fill=hex_to_rgba("#00FFFF", 40))

    d.rectangle([182, 55, 194, 65], fill=navy_dark, outline=navy_black)
    d.ellipse([184, 57, 192, 63], fill=cyan_bright)
    d.polygon([(188, 63), (181, 215), (246, 215)], fill=hex_to_rgba("#00FFFF", 40))

    # Visor de Comando
    d.rectangle([95, 80, 161, 115], fill=navy_black, outline=navy_light)
    d.rectangle([99, 84, 157, 111], fill=cyan_dark)
    d.line([99, 97, 157, 97], fill=cyan_bright)
    d.line([128, 84, 128, 111], fill=cyan_bright)

    # Portal Blindado com Scanner Biométrico
    d.rectangle([106, 155, 150, 215], fill=navy_black, outline=navy_light)
    d.rectangle([110, 160, 146, 215], fill=navy_dark)
    d.line([128, 160, 128, 215], fill=navy_black, width=2)
    for hy in range(162, 172, 3):
        d.line([110, hy, 146, hy], fill=hazard_yellow)

    d.rectangle([153, 175, 163, 190], fill=navy_black, outline=navy_light)
    d.ellipse([155, 177, 161, 183], fill=cyan_bright)
    d.line([154, 186, 162, 186], fill=cyan_mid)

    img.save(os.path.join(ASSETS_DIR, "Building_Sentinela.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Building_Sentinela.png"), os.path.join(TECH_QUEST_DIR, "Building_Sentinela.png"))
    print("✔ Building_Sentinela gerado.")

# =============================================================================
# 4. Building_Taverna (288x224)
# =============================================================================
def generate_building_taverna():
    w, h = 288, 224
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    timber_dark = hex_to_rgba("#2B180E")
    timber_mid = hex_to_rgba("#52301A")
    timber_light = hex_to_rgba("#7A4927")
    
    brick_dark = hex_to_rgba("#471C15")
    brick_mid = hex_to_rgba("#7A3225")
    
    tile_dark = hex_to_rgba("#5C1E14")
    tile_mid = hex_to_rgba("#9C3524")
    tile_light = hex_to_rgba("#D44D35")
    
    neon_amber = hex_to_rgba("#FFAA00")
    neon_bright = hex_to_rgba("#FFF0AA")
    neon_glow = hex_to_rgba("#FF8800", 120)
    
    stained_glass_glow = hex_to_rgba("#FFE07A")
    stained_lead = hex_to_rgba("#1F1E24")
    
    barrel_wood = hex_to_rgba("#402818")
    coolant_green = hex_to_rgba("#00FFB0")

    # Paredes mistas
    d.rectangle([35, 135, 253, 215], fill=brick_mid, outline=timber_dark)
    for by in range(138, 215, 8):
        d.line([35, by, 253, by], fill=brick_dark)
        offset = 8 if (by // 8) % 2 == 0 else 0
        for bx in range(35 + offset, 253, 16):
            d.line([bx, by, bx, by + 8], fill=brick_dark)

    d.rectangle([35, 80, 253, 135], fill=hex_to_rgba("#D9CBAC"), outline=timber_dark)
    for vx in [35, 80, 125, 165, 210, 253]:
        d.rectangle([vx - 3, 80, vx + 3, 135], fill=timber_mid, outline=timber_dark)
        d.line([vx - 1, 80, vx - 1, 135], fill=timber_light)
    d.line([35, 80, 80, 135], fill=timber_mid, width=4)
    d.line([125, 80, 80, 135], fill=timber_mid, width=4)
    d.line([165, 80, 210, 135], fill=timber_mid, width=4)
    d.line([253, 80, 210, 135], fill=timber_mid, width=4)

    # Telhado
    d.polygon([(15, 85), (144, 20), (273, 85)], fill=tile_mid, outline=timber_dark)
    for ty in range(25, 85, 7):
        d.line([(15 + (85 - ty) * 2.1, ty), (273 - (85 - ty) * 2.1, ty)], fill=tile_dark, width=2)
        step_x = 10
        cur_left = int(15 + (85 - ty) * 2.1)
        cur_right = int(273 - (85 - ty) * 2.1)
        for tx in range(cur_left, cur_right, step_x):
            d.arc([tx, ty - 2, tx + step_x, ty + 5], 180, 360, fill=tile_light, width=2)

    d.rectangle([10, 82, 278, 88], fill=timber_mid, outline=timber_dark)

    # Vitrais
    for wx in [60, 225]:
        d.arc([wx - 14, 92, wx + 14, 120], 180, 360, fill=timber_dark, width=3)
        d.rectangle([wx - 14, 105, wx + 14, 130], fill=stained_glass_glow, outline=timber_dark)
        d.pieslice([wx - 13, 93, wx + 13, 119], 180, 360, fill=stained_glass_glow)
        d.line([wx, 95, wx, 130], fill=stained_lead)
        d.line([wx - 14, 115, wx + 14, 115], fill=stained_lead)

    # Portal
    d.rectangle([118, 145, 170, 215], fill=timber_dark)
    d.rectangle([122, 150, 166, 215], fill=timber_mid)
    d.pieslice([122, 135, 166, 175], 180, 360, fill=timber_mid, outline=timber_dark)
    d.line([144, 140, 144, 215], fill=timber_dark, width=2)
    d.ellipse([138, 185, 142, 192], fill=neon_amber)

    # Letreiro Neon (;:)
    d.rectangle([110, 65, 178, 95], fill=hex_to_rgba("#151118"), outline=timber_dark)
    d.rectangle([108, 63, 180, 97], outline=neon_glow, width=2)
    d.rectangle([120, 72, 134, 88], outline=neon_amber, width=2)
    d.arc([132, 75, 140, 85], 270, 90, fill=neon_amber, width=2)
    d.line([118, 72, 136, 72], fill=neon_bright, width=2)
    d.ellipse([148, 73, 152, 77], fill=neon_bright)
    d.line([150, 80, 148, 86], fill=neon_bright, width=2)
    d.ellipse([162, 73, 166, 77], fill=neon_bright)
    d.ellipse([162, 82, 166, 86], fill=neon_bright)

    # Barris de fluido
    d.rectangle([80, 185, 102, 215], fill=barrel_wood, outline=timber_dark)
    d.ellipse([80, 180, 102, 190], fill=barrel_wood, outline=timber_dark)
    d.line([80, 192, 102, 192], fill=timber_light, width=2)
    d.ellipse([86, 183, 96, 187], fill=coolant_green)
    d.line([91, 187, 91, 215], fill=coolant_green, width=2)

    d.rectangle([180, 190, 200, 215], fill=barrel_wood, outline=timber_dark)
    d.ellipse([180, 186, 200, 194], fill=barrel_wood, outline=timber_dark)

    img.save(os.path.join(ASSETS_DIR, "Building_Taverna.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Building_Taverna.png"), os.path.join(TECH_QUEST_DIR, "Building_Taverna.png"))
    print("✔ Building_Taverna gerado.")

# =============================================================================
# 5 & 6. Drone_Leste e Drone_Oeste (48x48)
# =============================================================================
def generate_drones():
    w, h = 48, 48
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    brass_dark = hex_to_rgba("#422A14")
    brass_shadow = hex_to_rgba("#704B24")
    brass_mid = hex_to_rgba("#A87539")
    brass_light = hex_to_rgba("#DDA254")
    brass_hi = hex_to_rgba("#FCE092")
    
    lens_red_dark = hex_to_rgba("#6E0D19")
    lens_red_bright = hex_to_rgba("#FF2E44")
    lens_red_glow = hex_to_rgba("#FFA3AD", 210)
    
    spark_blue = hex_to_rgba("#00E5FF")
    spark_white = hex_to_rgba("#FFFFFF")
    glitch_magenta = hex_to_rgba("#FF00D4")

    # Rotor inferior
    d.ellipse([12, 36, 36, 44], outline=brass_dark, width=2)
    d.ellipse([14, 38, 34, 42], fill=brass_shadow)
    d.point([(10, 42), (15, 45), (24, 46), (33, 45), (38, 41)], fill=spark_blue)
    d.point([(23, 44), (25, 45)], fill=spark_white)

    # Garras
    d.line([(14, 32), (10, 36), (12, 39)], fill=brass_dark, width=2)
    d.line([(34, 32), (38, 36), (36, 39)], fill=brass_dark, width=2)

    # Carcaça esférica
    d.ellipse([8, 6, 40, 38], fill=brass_mid, outline=brass_dark)
    d.arc([10, 8, 38, 36], 120, 260, fill=brass_hi, width=2)
    d.arc([12, 10, 36, 34], 140, 240, fill=brass_light, width=2)
    d.line([(24, 6), (22, 14), (26, 20), (20, 28)], fill=brass_dark, width=2)
    d.point([(23, 14), (25, 20)], fill=glitch_magenta)

    # Lente frontal vermelha rachada
    d.ellipse([22, 14, 36, 28], fill=brass_dark)
    d.ellipse([24, 16, 34, 26], fill=lens_red_dark)
    d.ellipse([26, 18, 32, 24], fill=lens_red_bright)
    d.point([(29, 21)], fill=lens_red_glow)
    d.line([(23, 23), (35, 19)], fill=hex_to_rgba("#000000"), width=1)
    d.point([(35, 17), (20, 25), (38, 23)], fill=glitch_magenta)

    img.save(os.path.join(ASSETS_DIR, "Drone_Leste.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Drone_Leste.png"), os.path.join(TECH_QUEST_DIR, "Drone_Leste.png"))
    
    img_oeste = img.transpose(Image.FLIP_LEFT_RIGHT)
    img_oeste.save(os.path.join(ASSETS_DIR, "Drone_Oeste.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Drone_Oeste.png"), os.path.join(TECH_QUEST_DIR, "Drone_Oeste.png"))
    print("✔ Drone_Leste e Drone_Oeste gerados.")

# =============================================================================
# 7. InimigoVoid (128x128)
# =============================================================================
def generate_inimigo_void():
    w, h = 128, 128
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    void_black = hex_to_rgba("#05020A")
    void_purple_dark = hex_to_rgba("#2B0A3D")
    void_magenta_mid = hex_to_rgba("#871694")
    void_magenta_bright = hex_to_rgba("#D923B3")
    void_pink_glow = hex_to_rgba("#FF70DF")
    void_core_white = hex_to_rgba("#FFFFFF")
    cyan_glitch = hex_to_rgba("#00FFFF")

    tentacle_paths = [
        [(64, 64), (40, 85), (20, 110), (12, 122)],
        [(64, 64), (88, 85), (108, 110), (116, 122)],
        [(64, 64), (30, 50), (15, 40), (6, 25)],
        [(64, 64), (98, 50), (113, 40), (122, 25)],
        [(64, 64), (55, 95), (45, 120)],
        [(64, 64), (73, 95), (83, 120)]
    ]
    for path in tentacle_paths:
        for i in range(len(path)-1):
            d.line([path[i], path[i+1]], fill=void_purple_dark, width=4)
            d.line([path[i], path[i+1]], fill=void_magenta_mid, width=2)
            mx = (path[i][0] + path[i+1][0]) // 2
            my = (path[i][1] + path[i+1][1]) // 2
            d.point([(mx + 2, my - 1), (mx - 2, my + 1)], fill=cyan_glitch)
            d.point([(mx + 1, my + 2), (mx - 1, my - 2)], fill=void_pink_glow)

    shards = [
        [(25, 20), (35, 15), (38, 30), (28, 35)],
        [(90, 18), (105, 22), (100, 38), (88, 30)],
        [(15, 70), (25, 65), (22, 80)],
        [(102, 68), (115, 75), (106, 85)],
        [(45, 105), (55, 100), (50, 115)],
        [(75, 102), (85, 108), (78, 118)]
    ]
    for s in shards:
        d.polygon(s, fill=void_black, outline=void_magenta_bright)
        d.line([s[0], s[1]], fill=void_pink_glow)

    d.arc([24, 24, 104, 104], 30, 210, fill=void_magenta_mid, width=2)
    d.arc([20, 20, 108, 108], 220, 340, fill=void_magenta_bright, width=2)
    d.arc([34, 34, 94, 94], 120, 300, fill=cyan_glitch, width=1)
    
    code_dots = [(30, 45), (42, 32), (86, 32), (98, 45), (98, 83), (86, 96), (42, 96), (30, 83)]
    for cx, cy in code_dots:
        d.rectangle([cx-1, cy-1, cx+1, cy+1], fill=cyan_glitch)

    d.ellipse([44, 44, 84, 84], fill=void_black, outline=void_magenta_bright)
    d.ellipse([48, 48, 80, 80], fill=void_purple_dark)
    d.ellipse([52, 52, 76, 76], fill=void_magenta_bright)
    d.ellipse([56, 56, 72, 72], fill=void_pink_glow)
    d.ellipse([60, 60, 68, 68], fill=void_core_white)
    d.line([(58, 64), (70, 64)], fill=void_core_white, width=2)
    d.line([(64, 58), (64, 70)], fill=void_core_white, width=2)

    img.save(os.path.join(ASSETS_DIR, "InimigoVoid.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "InimigoVoid.png"), os.path.join(TECH_QUEST_DIR, "InimigoVoid.png"))
    print("✔ InimigoVoid gerado.")

# =============================================================================
# 8. Barreira_Norte (480x48)
# =============================================================================
def generate_barreira_norte():
    w, h = 480, 48
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    metal_dark = hex_to_rgba("#151821")
    metal_mid = hex_to_rgba("#353B4D")
    metal_hi = hex_to_rgba("#9AA6C7")
    
    plasma_deep = hex_to_rgba("#3F0B66", 180)
    plasma_violet = hex_to_rgba("#8814D4", 220)
    plasma_bright = hex_to_rgba("#C847FF")
    plasma_core = hex_to_rgba("#F3B8FF")
    cyan_spark = hex_to_rgba("#00FFFF")

    for tx in [0, 452]:
        d.rectangle([tx, 8, tx + 27, 44], fill=metal_mid, outline=metal_dark)
        d.line([tx + 1, 9, tx + 26, 9], fill=metal_hi)
        for by in [14, 24, 34]:
            d.rectangle([tx + 3, by, tx + 24, by + 6], fill=metal_dark)
            d.rectangle([tx + 5, by + 1, tx + 22, by + 5], fill=plasma_bright)
            d.line([tx + 7, by + 3, tx + 20, by + 3], fill=plasma_core)
        emitter_x = tx + 25 if tx == 0 else tx
        d.ellipse([emitter_x - 3, 18, emitter_x + 3, 30], fill=cyan_spark)

    d.rectangle([28, 14, 452, 34], fill=plasma_deep)
    d.rectangle([28, 18, 452, 30], fill=plasma_violet)
    d.rectangle([28, 22, 452, 26], fill=plasma_bright)
    d.line([28, 24, 452, 24], fill=plasma_core, width=2)

    for ax in range(35, 445, 12):
        d.line([(ax, 16), (ax + 4, 24), (ax + 8, 32)], fill=cyan_spark)

    def draw_pixel_text(text, start_x, y):
        font_3x5 = {
            '4': [(0,0), (0,1), (0,2), (1,2), (2,0), (2,1), (2,2), (2,3), (2,4)],
            '0': [(0,0),(0,1),(0,2),(0,3),(0,4), (1,0),(1,4), (2,0),(2,1),(2,2),(2,3),(2,4)],
            '3': [(0,0),(1,0),(2,0),(2,1),(1,2),(2,3),(2,4),(1,4),(0,4)],
            'E': [(0,0),(0,1),(0,2),(0,3),(0,4),(1,0),(1,2),(1,4),(2,0),(2,4)],
            'R': [(0,0),(0,1),(0,2),(0,3),(0,4),(1,0),(1,2),(2,0),(2,1),(2,3),(2,4)],
            'H': [(0,0),(0,1),(0,2),(0,3),(0,4),(1,2),(2,0),(2,1),(2,2),(2,3),(2,4)],
            'A': [(0,1),(0,2),(0,3),(0,4),(1,0),(1,2),(2,1),(2,2),(2,3),(2,4)],
            'L': [(0,0),(0,1),(0,2),(0,3),(0,4),(1,4),(2,4)],
            'T': [(0,0),(1,0),(2,0),(1,1),(1,2),(1,3),(1,4)]
        }
        cx = start_x
        for ch in text:
            if ch in font_3x5:
                for px, py in font_3x5[ch]:
                    d.point([(cx + px, y + py)], fill=plasma_core)
                    d.point([(cx + px + 1, y + py)], fill=cyan_spark)
            cx += 5

    draw_pixel_text("403", 80, 21)
    draw_pixel_text("ERR", 170, 21)
    draw_pixel_text("HALT", 260, 21)
    draw_pixel_text("403", 350, 21)

    img.save(os.path.join(ASSETS_DIR, "Barreira_Norte.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Barreira_Norte.png"), os.path.join(TECH_QUEST_DIR, "Barreira_Norte.png"))
    print("✔ Barreira_Norte gerada.")

# =============================================================================
# 9. Barreira_Licenca (64x64) e Barreira_Feixe (32x32)
# =============================================================================
def generate_barreira_licenca():
    w, h = 64, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    steel_dark = hex_to_rgba("#1E222B")
    steel_mid = hex_to_rgba("#383F4F")
    steel_light = hex_to_rgba("#616C85")
    glass_tint = hex_to_rgba("#1C3B47", 220)
    
    yellow_hazard = hex_to_rgba("#FFD000")
    red_lamp = hex_to_rgba("#FF2233")
    cyan_holo = hex_to_rgba("#00FFFF")

    d.rectangle([4, 16, 40, 60], fill=steel_mid, outline=steel_dark)
    d.rectangle([2, 12, 42, 16], fill=steel_light, outline=steel_dark)
    d.rectangle([8, 22, 36, 36], fill=glass_tint, outline=steel_dark)
    d.line([22, 22, 22, 36], fill=steel_dark)
    
    d.rectangle([8, 42, 24, 52], fill=steel_dark)
    d.line([10, 44, 22, 44], fill=cyan_holo, width=2)
    d.polygon([(16, 42), (11, 38), (21, 38)], fill=hex_to_rgba("#00FFFF", 160))

    d.rectangle([46, 18, 54, 38], fill=steel_dark, outline=steel_light)
    d.ellipse([48, 20, 52, 26], fill=red_lamp)
    d.ellipse([48, 28, 52, 34], fill=hex_to_rgba("#005511"))

    d.rectangle([42, 46, 48, 60], fill=steel_dark)
    for bx in range(46, 64, 4):
        d.line([(bx, 48), (bx + 3, 51)], fill=yellow_hazard, width=2)
        d.line([(bx + 2, 48), (bx - 1, 51)], fill=steel_dark, width=2)

    img.save(os.path.join(ASSETS_DIR, "Barreira_Licenca.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Barreira_Licenca.png"), os.path.join(TECH_QUEST_DIR, "Barreira_Licenca.png"))

    img_feixe = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    df = ImageDraw.Draw(img_feixe)
    df.rectangle([0, 10, 32, 22], fill=hex_to_rgba("#6B14B0", 180))
    df.rectangle([0, 13, 32, 19], fill=hex_to_rgba("#B93BFF"))
    df.line([0, 16, 32, 16], fill=hex_to_rgba("#FFFFFF"), width=2)
    df.point([(8, 12), (16, 20), (24, 11)], fill=cyan_holo)
    
    img_feixe.save(os.path.join(ASSETS_DIR, "Barreira_Feixe.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Barreira_Feixe.png"), os.path.join(TECH_QUEST_DIR, "Barreira_Feixe.png"))
    print("✔ Barreira_Licenca e Barreira_Feixe gerados.")

# =============================================================================
# 10..16. NPCs (Todos 32x64)
# =============================================================================
def generate_npcs():
    skin_light = hex_to_rgba("#F5CDB4")
    skin_mid = hex_to_rgba("#D49B79")
    outline_dark = hex_to_rgba("#1C1518")

    # 1. NPC_Grace
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    robe_dark = hex_to_rgba("#877A64")
    robe_mid = hex_to_rgba("#C2B499")
    gold_line = hex_to_rgba("#FFCC00")
    
    d.rectangle([10, 8, 22, 24], fill=robe_mid, outline=outline_dark)
    d.line([10, 8, 22, 8], fill=gold_line, width=2)
    d.rectangle([12, 14, 20, 21], fill=skin_light)
    d.ellipse([12, 16, 15, 19], outline=gold_line)
    d.ellipse([17, 16, 20, 19], outline=gold_line)
    d.line([15, 17, 17, 17], fill=gold_line)
    d.ellipse([14, 23, 18, 26], fill=gold_line, outline=outline_dark)

    d.polygon([(10, 25), (22, 25), (26, 56), (6, 56)], fill=robe_mid, outline=outline_dark)
    d.line([16, 25, 16, 56], fill=robe_dark)
    d.line([6, 54, 26, 54], fill=gold_line)
    
    d.line([(10, 28), (6, 38)], fill=robe_mid, width=3)
    d.rectangle([4, 36, 10, 44], fill=hex_to_rgba("#FFF8E7"), outline=outline_dark)
    d.point([(6, 38), (8, 40), (6, 42)], fill=outline_dark)

    d.rectangle([11, 57, 14, 60], fill=outline_dark)
    d.rectangle([18, 57, 21, 60], fill=outline_dark)
    
    img.save(os.path.join(ASSETS_DIR, "NPC_Grace.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Grace.png"), os.path.join(TECH_QUEST_DIR, "NPC_Grace.png"))

    # 2. NPC_Ada
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    hair_brown = hex_to_rgba("#4A2810")
    apron_leather = hex_to_rgba("#6E3A1A")
    jumpsuit = hex_to_rgba("#223B59")
    bronze_arm = hex_to_rgba("#CD7F32")
    goggle_cyan = hex_to_rgba("#00E5FF")

    d.ellipse([12, 5, 20, 12], fill=hair_brown)
    d.rectangle([10, 10, 22, 20], fill=hair_brown, outline=outline_dark)
    d.rectangle([11, 11, 21, 14], fill=outline_dark)
    d.ellipse([12, 11, 15, 14], fill=goggle_cyan)
    d.ellipse([17, 11, 20, 14], fill=goggle_cyan)
    d.rectangle([12, 15, 20, 21], fill=skin_light)
    d.point([(14, 18), (18, 18)], fill=outline_dark)

    d.rectangle([9, 22, 23, 50], fill=jumpsuit, outline=outline_dark)
    d.rectangle([11, 25, 21, 48], fill=apron_leather, outline=outline_dark)

    d.line([(9, 24), (5, 34)], fill=bronze_arm, width=4)
    d.rectangle([3, 33, 7, 39], fill=hex_to_rgba("#8C5824"), outline=outline_dark)

    d.line([(23, 24), (27, 34)], fill=jumpsuit, width=3)
    d.rectangle([25, 34, 29, 44], fill=hex_to_rgba("#555555"), outline=outline_dark)
    d.line([27, 44, 27, 49], fill=goggle_cyan, width=2)

    d.rectangle([10, 51, 14, 60], fill=outline_dark)
    d.rectangle([18, 51, 22, 60], fill=outline_dark)

    img.save(os.path.join(ASSETS_DIR, "NPC_Ada.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Ada.png"), os.path.join(TECH_QUEST_DIR, "NPC_Ada.png"))

    # 3. NPC_Rust
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    jumpsuit_orange = hex_to_rgba("#E85D04")
    orange_shadow = hex_to_rgba("#9D3800")
    oil_stain = hex_to_rgba("#1C1A17")
    gloves_thick = hex_to_rgba("#4A463F")
    steel_toe = hex_to_rgba("#B0B8C4")

    d.rectangle([10, 8, 22, 16], fill=hex_to_rgba("#8A3311"), outline=outline_dark)
    d.rectangle([11, 14, 21, 22], fill=skin_light)
    d.point([(13, 17), (19, 17)], fill=outline_dark)
    d.point([(13, 20)], fill=oil_stain)

    d.rectangle([9, 23, 23, 50], fill=jumpsuit_orange, outline=outline_dark)
    d.line([16, 23, 16, 50], fill=orange_shadow)
    d.point([(12, 30), (13, 31), (18, 42), (19, 43), (12, 45)], fill=oil_stain)

    d.rectangle([5, 34, 9, 40], fill=gloves_thick, outline=outline_dark)
    d.rectangle([23, 34, 27, 40], fill=gloves_thick, outline=outline_dark)

    d.rectangle([21, 40, 31, 52], fill=hex_to_rgba("#8A1C14"), outline=outline_dark)
    d.line([23, 43, 29, 43], fill=steel_toe, width=2)

    d.rectangle([10, 51, 14, 57], fill=outline_dark)
    d.rectangle([10, 58, 14, 61], fill=steel_toe)
    d.rectangle([18, 51, 22, 57], fill=outline_dark)
    d.rectangle([18, 58, 22, 61], fill=steel_toe)

    img.save(os.path.join(ASSETS_DIR, "NPC_Rust.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Rust.png"), os.path.join(TECH_QUEST_DIR, "NPC_Rust.png"))

    # 4. NPC_Turing
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    military_navy = hex_to_rgba("#182538")
    military_grey = hex_to_rgba("#455263")
    gold_epaulette = hex_to_rgba("#E6B800")
    hud_green = hex_to_rgba("#00FF66")

    d.rectangle([10, 7, 22, 14], fill=military_navy, outline=outline_dark)
    d.line([8, 12, 24, 12], fill=outline_dark, width=2)
    d.rectangle([11, 14, 21, 22], fill=skin_mid)
    d.rectangle([16, 15, 21, 19], fill=hex_to_rgba("#003311"), outline=hud_green)
    d.point([(18, 17)], fill=hud_green)

    d.rectangle([8, 23, 24, 52], fill=military_navy, outline=outline_dark)
    d.line([16, 23, 16, 52], fill=military_grey, width=2)
    d.rectangle([6, 23, 10, 27], fill=gold_epaulette, outline=outline_dark)
    d.rectangle([22, 23, 26, 27], fill=gold_epaulette, outline=outline_dark)
    d.point([(8, 25), (24, 25)], fill=outline_dark)

    d.rectangle([8, 38, 24, 42], fill=outline_dark)
    d.rectangle([6, 36, 9, 44], fill=hex_to_rgba("#333333"), outline=outline_dark)
    d.line([7, 36, 7, 28], fill=hex_to_rgba("#888888"))

    d.rectangle([10, 52, 14, 61], fill=outline_dark)
    d.rectangle([18, 52, 22, 61], fill=outline_dark)

    img.save(os.path.join(ASSETS_DIR, "NPC_Turing.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Turing.png"), os.path.join(TECH_QUEST_DIR, "NPC_Turing.png"))

    # 5. NPC_Babbage
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    lab_white = hex_to_rgba("#F0F4F8")
    lab_shadow = hex_to_rgba("#BAC6D1")
    circuit_blue = hex_to_rgba("#0099FF")
    hair_grey = hex_to_rgba("#8C9199")

    d.ellipse([11, 7, 21, 15], fill=skin_light)
    d.rectangle([9, 14, 12, 21], fill=hair_grey)
    d.rectangle([20, 14, 23, 21], fill=hair_grey)
    d.rectangle([12, 14, 20, 21], fill=skin_light)
    
    d.line([10, 11, 22, 11], fill=outline_dark, width=2)
    d.ellipse([11, 9, 15, 13], outline=hex_to_rgba("#FFD700"))
    d.ellipse([17, 9, 21, 13], outline=hex_to_rgba("#FFD700"))

    d.rectangle([8, 22, 24, 52], fill=lab_white, outline=outline_dark)
    d.line([16, 22, 16, 52], fill=lab_shadow)
    d.line([10, 26, 14, 26], fill=circuit_blue)
    d.line([14, 26, 14, 34], fill=circuit_blue)
    d.line([18, 30, 22, 30], fill=circuit_blue)

    d.rectangle([4, 32, 13, 46], fill=hex_to_rgba("#8A5A2B"), outline=outline_dark)
    d.rectangle([6, 34, 11, 44], fill=hex_to_rgba("#FFFDE8"))
    d.line([7, 36, 10, 36], fill=outline_dark)
    d.line([7, 39, 10, 39], fill=outline_dark)

    d.rectangle([10, 52, 14, 60], fill=hex_to_rgba("#3B3D45"))
    d.rectangle([18, 52, 22, 60], fill=hex_to_rgba("#3B3D45"))

    img.save(os.path.join(ASSETS_DIR, "NPC_Babbage.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Babbage.png"), os.path.join(TECH_QUEST_DIR, "NPC_Babbage.png"))

    # 6. NPC_Pascal
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    apron_green = hex_to_rgba("#2E6E38")
    apron_brown = hex_to_rgba("#704621")
    beer_amber = hex_to_rgba("#FFB703")
    beer_foam = hex_to_rgba("#FFF6D6")

    d.ellipse([10, 7, 22, 19], fill=skin_light)
    d.point([(13, 14), (19, 14)], fill=outline_dark)
    d.polygon([(11, 18), (16, 17), (21, 18), (24, 15), (16, 19), (8, 15)], fill=hex_to_rgba("#3D220E"))

    d.rectangle([6, 21, 26, 52], fill=hex_to_rgba("#F5EADB"), outline=outline_dark)
    d.rectangle([8, 25, 24, 52], fill=apron_green, outline=outline_dark)
    for ly in range(27, 52, 6):
        d.line([8, ly, 24, ly], fill=apron_brown, width=2)

    d.polygon([(7, 22), (11, 22), (9, 36)], fill=hex_to_rgba("#FFFFFF"), outline=outline_dark)

    d.rectangle([21, 32, 29, 44], fill=beer_amber, outline=outline_dark)
    d.rectangle([20, 30, 30, 34], fill=beer_foam)
    d.arc([27, 34, 32, 42], 270, 90, fill=outline_dark, width=2)

    d.rectangle([9, 52, 14, 60], fill=hex_to_rgba("#4A3525"))
    d.rectangle([18, 52, 23, 60], fill=hex_to_rgba("#4A3525"))

    img.save(os.path.join(ASSETS_DIR, "NPC_Pascal.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Pascal.png"), os.path.join(TECH_QUEST_DIR, "NPC_Pascal.png"))

    # 7. NPC_Byron
    img = Image.new("RGBA", (32, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    velvet_bordeaux = hex_to_rgba("#800E26")
    velvet_bright = hex_to_rgba("#B81D3E")
    fiber_cyan = hex_to_rgba("#00FFFF")
    doublet_cream = hex_to_rgba("#FFF5E0")
    lute_wood = hex_to_rgba("#8C5523")

    d.ellipse([9, 6, 23, 14], fill=velvet_bordeaux, outline=outline_dark)
    d.line([(19, 7), (25, 2)], fill=fiber_cyan, width=2)
    d.point([(26, 1), (27, 2)], fill=hex_to_rgba("#FFFFFF"))

    d.rectangle([11, 13, 21, 20], fill=skin_light)
    d.point([(13, 16), (19, 16)], fill=outline_dark)

    d.rectangle([9, 21, 23, 44], fill=doublet_cream, outline=outline_dark)
    d.polygon([(7, 22), (10, 22), (6, 46), (3, 44)], fill=velvet_bordeaux, outline=outline_dark)
    d.polygon([(22, 22), (25, 22), (28, 44), (25, 46)], fill=velvet_bright, outline=outline_dark)

    d.ellipse([9, 42, 16, 50], fill=velvet_bordeaux)
    d.ellipse([16, 42, 23, 50], fill=velvet_bordeaux)
    d.rectangle([10, 48, 14, 60], fill=hex_to_rgba("#211913"))
    d.rectangle([18, 48, 22, 60], fill=hex_to_rgba("#211913"))

    d.ellipse([14, 28, 27, 44], fill=lute_wood, outline=outline_dark)
    d.ellipse([18, 33, 23, 38], fill=outline_dark)
    d.line([(16, 32), (8, 25)], fill=lute_wood, width=3)
    for kx, ky in [(16, 30), (20, 30), (24, 30), (16, 40), (20, 40), (24, 40)]:
        d.point([(kx, ky)], fill=hex_to_rgba("#E8E8E8"))

    img.save(os.path.join(ASSETS_DIR, "NPC_Byron.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "NPC_Byron.png"), os.path.join(TECH_QUEST_DIR, "NPC_Byron.png"))
    print("✔ Todos os 7 NPCs gerados.")

# =============================================================================
# 17. Quadro_Missoes (48x64)
# =============================================================================
def generate_quadro_missoes():
    w, h = 48, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    wood_frame = hex_to_rgba("#3B2213")
    copper_patina = hex_to_rgba("#35826D")
    cork_mid = hex_to_rgba("#B88552")
    cork_dark = hex_to_rgba("#8F5E32")
    parchment = hex_to_rgba("#F5E6C9")
    wax_red = hex_to_rgba("#B81424")
    pin_gold = hex_to_rgba("#FFD700")

    d.rectangle([2, 4, 45, 59], fill=cork_mid, outline=wood_frame, width=3)
    d.rectangle([1, 3, 46, 60], outline=copper_patina, width=1)
    
    for cx in range(6, 42, 4):
        for cy in range(8, 56, 4):
            if (cx + cy) % 3 == 0:
                d.point([(cx, cy)], fill=cork_dark)

    d.rectangle([8, 10, 24, 26], fill=parchment, outline=hex_to_rgba("#222222"))
    d.line([10, 14, 22, 14], fill=hex_to_rgba("#555555"))
    d.line([10, 18, 20, 18], fill=hex_to_rgba("#555555"))
    d.ellipse([15, 8, 17, 10], fill=pin_gold)
    d.ellipse([19, 22, 23, 26], fill=wax_red)

    d.rectangle([27, 12, 41, 32], fill=hex_to_rgba("#FFFDE8"), outline=hex_to_rgba("#222222"))
    for py in [16, 20, 24, 28]:
        d.point([(30, py), (34, py), (38, py)], fill=hex_to_rgba("#222222"))
    d.ellipse([33, 10, 35, 12], fill=pin_gold)

    d.rectangle([10, 32, 28, 52], fill=parchment, outline=hex_to_rgba("#222222"))
    d.line([12, 36, 26, 36], fill=hex_to_rgba("#555555"))
    d.line([12, 40, 22, 40], fill=hex_to_rgba("#555555"))
    d.line([12, 44, 24, 44], fill=hex_to_rgba("#555555"))
    d.ellipse([18, 30, 20, 32], fill=pin_gold)
    d.ellipse([12, 48, 16, 52], fill=wax_red)

    d.rectangle([30, 38, 42, 50], fill=hex_to_rgba("#FFE853"), outline=hex_to_rgba("#222222"))
    d.ellipse([35, 36, 37, 38], fill=pin_gold)

    d.rectangle([8, 59, 12, 63], fill=wood_frame)
    d.rectangle([35, 59, 39, 63], fill=wood_frame)

    img.save(os.path.join(ASSETS_DIR, "Quadro_Missoes.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Quadro_Missoes.png"), os.path.join(TECH_QUEST_DIR, "Quadro_Missoes.png"))
    print("✔ Quadro_Missoes gerado.")

# =============================================================================
# 18. Bigorna_Pneumatica (64x64)
# =============================================================================
def generate_bigorna_pneumatica():
    w, h = 64, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    cast_iron = hex_to_rgba("#1E2024")
    iron_mid = hex_to_rgba("#383C45")
    iron_light = hex_to_rgba("#697182")
    iron_hi = hex_to_rgba("#A3ADC2")
    
    copper_dark = hex_to_rgba("#5C2B14")
    copper_mid = hex_to_rgba("#9C4B24")
    copper_hot = hex_to_rgba("#FF621F")
    copper_bright = hex_to_rgba("#FFA733")
    
    steam_white = hex_to_rgba("#EDF6FF", 210)
    gauge_yellow = hex_to_rgba("#FFCC00")

    d.rectangle([14, 6, 50, 60], fill=cast_iron, outline=hex_to_rgba("#0F1114"))
    d.line([15, 7, 49, 7], fill=iron_light)

    d.rectangle([20, 10, 44, 28], fill=iron_mid, outline=cast_iron)
    d.line([22, 10, 22, 28], fill=iron_hi)
    d.line([19, 14, 45, 14], fill=copper_mid, width=2)
    d.line([19, 24, 45, 24], fill=copper_mid, width=2)

    d.ellipse([6, 14, 16, 24], fill=copper_dark, outline=cast_iron)
    d.ellipse([7, 15, 15, 23], fill=hex_to_rgba("#FFFDE8"))
    d.line([11, 19, 14, 16], fill=gauge_yellow, width=2)
    d.ellipse([10, 18, 12, 20], fill=cast_iron)

    d.ellipse([46, 12, 58, 22], fill=steam_white)

    d.rectangle([28, 28, 36, 42], fill=iron_hi, outline=cast_iron)
    d.line([30, 28, 30, 42], fill=hex_to_rgba("#FFFFFF"))

    d.rectangle([24, 40, 40, 46], fill=iron_mid, outline=cast_iron)
    d.rectangle([26, 46, 38, 50], fill=copper_hot)
    d.line([27, 48, 37, 48], fill=copper_bright)

    d.rectangle([16, 50, 48, 60], fill=iron_mid, outline=cast_iron)
    d.line([16, 50, 48, 50], fill=iron_hi, width=2)
    d.polygon([(16, 50), (10, 52), (16, 56)], fill=iron_mid, outline=cast_iron)

    img.save(os.path.join(ASSETS_DIR, "Bigorna_Pneumatica.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Bigorna_Pneumatica.png"), os.path.join(TECH_QUEST_DIR, "Bigorna_Pneumatica.png"))
    print("✔ Bigorna_Pneumatica gerada.")

# =============================================================================
# 19. Radar_Firewall (64x64)
# =============================================================================
def generate_radar_firewall():
    w, h = 64, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    console_dark = hex_to_rgba("#171A21")
    console_mid = hex_to_rgba("#2E3442")
    console_light = hex_to_rgba("#4D566E")
    
    crt_bg = hex_to_rgba("#041409")
    crt_grid = hex_to_rgba("#0E471E")
    radar_green = hex_to_rgba("#00FF48")
    radar_bright = hex_to_rgba("#AAFFBE")
    
    led_red = hex_to_rgba("#FF2233")
    led_amber = hex_to_rgba("#FF9900")
    led_green = hex_to_rgba("#00FF66")

    d.rectangle([4, 6, 60, 60], fill=console_mid, outline=console_dark)
    d.line([5, 7, 59, 7], fill=console_light)

    d.rectangle([10, 10, 42, 38], fill=console_dark, outline=console_light)
    d.rectangle([12, 12, 40, 36], fill=crt_bg)
    d.ellipse([14, 14, 38, 34], outline=crt_grid)
    d.ellipse([20, 18, 32, 30], outline=crt_grid)
    d.line([26, 12, 26, 36], fill=crt_grid)
    d.line([12, 24, 40, 24], fill=crt_grid)
    d.line([(26, 24), (36, 16)], fill=radar_green, width=2)
    d.point([(34, 18), (35, 17)], fill=radar_bright)
    d.point([(20, 28), (30, 31)], fill=radar_green)

    d.rectangle([46, 10, 56, 38], fill=console_dark)
    led_colors = [led_green, led_red, led_amber, led_green, led_red, led_amber]
    for idx, ly in enumerate(range(13, 36, 4)):
        d.ellipse([49, ly, 53, ly + 3], fill=led_colors[idx])

    d.polygon([(8, 58), (12, 42), (52, 42), (56, 58)], fill=console_dark, outline=console_light)
    for ty in [45, 49, 53]:
        for tx in range(14, 50, 4):
            d.rectangle([tx, ty, tx + 2, ty + 2], fill=console_light)

    img.save(os.path.join(ASSETS_DIR, "Radar_Firewall.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Radar_Firewall.png"), os.path.join(TECH_QUEST_DIR, "Radar_Firewall.png"))
    print("✔ Radar_Firewall gerado.")

# =============================================================================
# 20. Lareira_Vapor (64x64)
# =============================================================================
def generate_lareira_vapor():
    w, h = 64, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    stone_dark = hex_to_rgba("#222329")
    stone_mid = hex_to_rgba("#4B4E59")
    
    copper_dark = hex_to_rgba("#5C2B14")
    copper_mid = hex_to_rgba("#A34E24")
    copper_hi = hex_to_rgba("#E87F48")
    
    flame_red = hex_to_rgba("#E61D00")
    flame_orange = hex_to_rgba("#FF7300")
    flame_yellow = hex_to_rgba("#FFE135")
    
    steam_soft = hex_to_rgba("#E8F4FF", 160)

    d.rectangle([8, 16, 56, 60], fill=stone_mid, outline=stone_dark)
    for sy in range(20, 60, 8):
        d.line([8, sy, 56, sy], fill=stone_dark)
        for sx in range(8 + (8 if sy % 16 == 0 else 0), 56, 16):
            d.line([sx, sy, sx, sy + 8], fill=stone_dark)

    d.rectangle([18, 32, 46, 58], fill=stone_dark)
    d.ellipse([20, 48, 44, 57], fill=flame_red)
    d.ellipse([24, 46, 40, 56], fill=flame_orange)
    d.ellipse([28, 44, 36, 54], fill=flame_yellow)
    d.rectangle([19, 33, 45, 57], outline=hex_to_rgba("#88C8FF", 120), width=2)
    for gy in range(36, 56, 4):
        d.line([19, gy, 45, gy], fill=stone_dark)

    d.line([(12, 60), (12, 22), (16, 22)], fill=copper_mid, width=3)
    d.line([(52, 60), (52, 22), (48, 22)], fill=copper_mid, width=3)
    d.line([16, 22, 48, 22], fill=copper_hi, width=3)

    d.rectangle([22, 10, 42, 16], fill=stone_dark, outline=copper_dark)
    d.ellipse([20, 2, 34, 10], fill=steam_soft)
    d.ellipse([30, 0, 44, 8], fill=steam_soft)

    img.save(os.path.join(ASSETS_DIR, "Lareira_Vapor.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Lareira_Vapor.png"), os.path.join(TECH_QUEST_DIR, "Lareira_Vapor.png"))
    print("✔ Lareira_Vapor gerada.")

# =============================================================================
# 21. Clicavel (32x32)
# =============================================================================
def generate_clicavel():
    w, h = 32, 32
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    ti_dark = hex_to_rgba("#1C2129")
    ti_mid = hex_to_rgba("#5B6575")
    ti_light = hex_to_rgba("#8894A8")
    
    blue_core_dark = hex_to_rgba("#0052CC")
    blue_core_bright = hex_to_rgba("#0099FF")
    blue_core_glow = hex_to_rgba("#80D4FF")

    d.polygon([(7, 8), (25, 8), (28, 11), (28, 25), (25, 28), (7, 28), (4, 25), (4, 11)], fill=ti_mid, outline=ti_dark)
    d.line([(7, 9), (25, 9)], fill=ti_light)
    d.line([(5, 11), (5, 25)], fill=ti_light)

    d.rectangle([6, 17, 26, 20], fill=blue_core_dark)
    d.line([6, 18, 26, 18], fill=blue_core_bright, width=2)
    d.line([10, 18, 22, 18], fill=blue_core_glow)

    d.rectangle([3, 15, 7, 22], fill=ti_dark)
    d.rectangle([25, 15, 29, 22], fill=ti_dark)

    d.rectangle([12, 11, 20, 15], fill=ti_dark)
    d.rectangle([13, 12, 19, 14], fill=blue_core_bright)
    d.point([(16, 13)], fill=hex_to_rgba("#FFFFFF"))

    img.save(os.path.join(ASSETS_DIR, "Clicavel.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Clicavel.png"), os.path.join(TECH_QUEST_DIR, "Clicavel.png"))
    print("✔ Clicavel gerado.")

# =============================================================================
# 22. Holograma_Core (64x64)
# =============================================================================
def generate_holograma_core():
    w, h = 64, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    holo_dark = hex_to_rgba("#003847", 140)
    holo_mid = hex_to_rgba("#008FA8", 190)
    holo_cyan = hex_to_rgba("#00FFFF")
    holo_white = hex_to_rgba("#FFFFFF")

    d.ellipse([4, 4, 60, 60], outline=holo_mid, width=2)
    d.ellipse([8, 8, 56, 56], outline=holo_dark)
    for deg in range(0, 360, 20):
        rad = math.radians(deg)
        x1 = int(32 + 25 * math.cos(rad))
        y1 = int(32 + 25 * math.sin(rad))
        d.point([(x1, y1)], fill=holo_cyan if deg % 40 == 0 else holo_mid)

    d.ellipse([14, 14, 50, 50], outline=holo_cyan, width=2)
    d.ellipse([14, 22, 50, 42], outline=holo_mid)
    d.line([14, 32, 50, 32], fill=holo_cyan)
    d.ellipse([22, 14, 42, 50], outline=holo_mid)
    d.line([32, 14, 32, 50], fill=holo_cyan)

    d.ellipse([28, 28, 36, 36], fill=holo_cyan)
    d.ellipse([30, 30, 34, 34], fill=holo_white)
    d.arc([10, 26, 54, 38], 0, 360, fill=holo_cyan, width=1)

    img.save(os.path.join(ASSETS_DIR, "Holograma_Core.png"))
    shutil.copyfile(os.path.join(ASSETS_DIR, "Holograma_Core.png"), os.path.join(TECH_QUEST_DIR, "Holograma_Core.png"))
    print("✔ Holograma_Core gerado.")

if __name__ == "__main__":
    print("Iniciando geração de sprites PixelForge AI...")
    generate_building_guilda()
    generate_building_forja()
    generate_building_sentinela()
    generate_building_taverna()
    generate_drones()
    generate_inimigo_void()
    generate_barreira_norte()
    generate_barreira_licenca()
    generate_npcs()
    generate_quadro_missoes()
    generate_bigorna_pneumatica()
    generate_radar_firewall()
    generate_lareira_vapor()
    generate_clicavel()
    generate_holograma_core()
    print("--- Todos os sprites foram gerados e salvos em assets/ com sucesso! ---")
