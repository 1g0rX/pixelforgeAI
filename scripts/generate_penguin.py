#!/usr/bin/env python3
"""
Generates a high-detail pixel art Penguin with a Cooking Pot Hat (32x64 px).
"""
import os
import shutil
from PIL import Image, ImageDraw

ASSETS_DIR = "/home/igor/Documentos/Projetos/pixelforgeAI/assets"
TECH_QUEST_DIR = "/home/igor/Documentos/Projetos/flutter-jail/home/dev/tech-quest/assets/images"

os.makedirs(ASSETS_DIR, exist_ok=True)
os.makedirs(TECH_QUEST_DIR, exist_ok=True)

def hex_to_rgba(h, a=255):
    h = h.lstrip('#')
    return tuple(int(h[i:i+2], 16) for i in (0, 2, 4)) + (a,)

def generate_penguin():
    w, h = 32, 64
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)

    # Color Palette
    # Outline / Darkest
    C_OUTLINE = hex_to_rgba("#12141C")
    
    # Penguin Feathers (Navy Black)
    C_FEATHER_DARK = hex_to_rgba("#1C202E")
    C_FEATHER_MID  = hex_to_rgba("#293045")
    C_FEATHER_HI   = hex_to_rgba("#3F4966")
    C_FEATHER_LGT  = hex_to_rgba("#59658A")

    # Penguin White Belly / Face
    C_WHITE_HI     = hex_to_rgba("#FFFFFF")
    C_WHITE_MID    = hex_to_rgba("#E8EDF5")
    C_WHITE_SHADOW = hex_to_rgba("#B4C0D4")
    C_WHITE_DARK   = hex_to_rgba("#8897B0")

    # Beak & Feet (Orange/Gold)
    C_ORANGE_DARK  = hex_to_rgba("#B84400")
    C_ORANGE_MID   = hex_to_rgba("#FF7700")
    C_ORANGE_LGT   = hex_to_rgba("#FFAA00")
    C_ORANGE_HI    = hex_to_rgba("#FFD452")

    # Pot / Metal (Stainless Steel / Cast Iron with metallic highlights)
    C_POT_DARK     = hex_to_rgba("#1F242B")
    C_POT_SHADOW   = hex_to_rgba("#3B4350")
    C_POT_MID      = hex_to_rgba("#5F6D82")
    C_POT_LGT      = hex_to_rgba("#8D9FB8")
    C_POT_HI       = hex_to_rgba("#BDCEE3")
    C_POT_SPEC     = hex_to_rgba("#EEF5FF")

    # Scarf / Bandana (Cozy adventuring scarf around neck)
    C_SCARF_DARK   = hex_to_rgba("#8A1828")
    C_SCARF_MID    = hex_to_rgba("#D1283E")
    C_SCARF_HI     = hex_to_rgba("#FF576C")
    C_SCARF_GOLD   = hex_to_rgba("#FFC436")

    # Blush
    C_BLUSH        = hex_to_rgba("#FF8FA3", 180)

    # Eyes
    C_EYE_PUPIL    = hex_to_rgba("#111318")
    C_EYE_SHINE    = hex_to_rgba("#FFFFFF")

    # ----------------------------------------------------
    # 1. FEET & SHADOW (y: 57..62)
    # ----------------------------------------------------
    # Ground soft shadow
    for x in range(7, 25):
        d.point((x, 62), fill=hex_to_rgba("#0E1017", 90))
    for x in range(9, 23):
        d.point((x, 63), fill=hex_to_rgba("#0E1017", 50))

    # Left Foot (x: 8..14, y: 57..61)
    d.rectangle([8, 58, 13, 60], fill=C_ORANGE_MID, outline=C_OUTLINE)
    d.point([(8, 61), (10, 61), (13, 61)], fill=C_ORANGE_DARK)
    d.line([(9, 58), (12, 58)], fill=C_ORANGE_LGT)
    d.point([(10, 57), (11, 57)], fill=C_ORANGE_HI)

    # Right Foot (x: 18..24, y: 57..61)
    d.rectangle([18, 58, 23, 60], fill=C_ORANGE_MID, outline=C_OUTLINE)
    d.point([(18, 61), (21, 61), (23, 61)], fill=C_ORANGE_DARK)
    d.line([(19, 58), (22, 58)], fill=C_ORANGE_LGT)
    d.point([(19, 57), (20, 57)], fill=C_ORANGE_HI)

    # ----------------------------------------------------
    # 2. BODY & WINGS (y: 33..57)
    # ----------------------------------------------------
    # Outer dark penguin body silhouette
    for y in range(33, 58):
        # Body curve formula
        if y < 36:
            x1, x2 = 9, 22
        elif y < 42:
            x1, x2 = 7, 24
        elif y < 52:
            x1, x2 = 6, 25
        elif y < 56:
            x1, x2 = 7, 24
        else:
            x1, x2 = 9, 22

        d.line([(x1, y), (x2, y)], fill=C_FEATHER_MID)
        d.point([(x1, y), (x2, y)], fill=C_OUTLINE)

    # Feather volume shading (Darker on right side / bottom)
    for y in range(35, 57):
        d.point([(7, y), (8, y)], fill=C_FEATHER_HI)
        d.point([(23, y), (24, y)], fill=C_FEATHER_DARK)

    # White belly (y: 36..55, x: ~10..21)
    for y in range(36, 56):
        if y < 39:
            bx1, bx2 = 12, 19
        elif y < 45:
            bx1, bx2 = 10, 21
        elif y < 52:
            bx1, bx2 = 10, 22
        elif y < 55:
            bx1, bx2 = 11, 20
        else:
            bx1, bx2 = 12, 19

        d.line([(bx1, y), (bx2, y)], fill=C_WHITE_MID)
        # Belly shading
        d.point([(bx1, y)], fill=C_WHITE_SHADOW)
        d.point([(bx2, y)], fill=C_WHITE_SHADOW)
        # Belly highlight in center
        if bx1 + 2 <= bx2 - 2:
            d.line([(bx1 + 1, y), (bx2 - 2, y)], fill=C_WHITE_HI)
        # Bottom shadow of belly
        if y >= 53:
            d.line([(bx1, y), (bx2, y)], fill=C_WHITE_SHADOW)
        if y == 55:
            d.line([(bx1 + 1, y), (bx2 - 1, y)], fill=C_WHITE_DARK)

    # Wings (Flippers)
    # Left wing (x: 4..8, y: 36..49) - slight resting angle
    d.polygon([(8, 35), (4, 40), (4, 46), (7, 49), (9, 44)], fill=C_FEATHER_MID, outline=C_OUTLINE)
    d.line([(5, 40), (5, 45)], fill=C_FEATHER_HI)
    d.point([(6, 38), (6, 39)], fill=C_FEATHER_LGT)

    # Right wing (x: 23..27, y: 36..49)
    d.polygon([(23, 35), (27, 40), (27, 46), (24, 49), (22, 44)], fill=C_FEATHER_MID, outline=C_OUTLINE)
    d.line([(26, 40), (26, 45)], fill=C_FEATHER_DARK)
    d.line([(24, 38), (24, 44)], fill=C_FEATHER_MID)

    # ----------------------------------------------------
    # 3. ADVENTURE SCARF (y: 31..36)
    # ----------------------------------------------------
    d.rectangle([9, 31, 22, 34], fill=C_SCARF_MID, outline=C_OUTLINE)
    d.line([(10, 32), (21, 32)], fill=C_SCARF_HI)
    d.line([(9, 34), (22, 34)], fill=C_SCARF_DARK)
    # Scarf knot and tail hanging down on the left
    d.rectangle([10, 34, 13, 37], fill=C_SCARF_MID, outline=C_OUTLINE)
    d.rectangle([9, 36, 12, 42], fill=C_SCARF_MID, outline=C_OUTLINE)
    d.line([(10, 36), (10, 41)], fill=C_SCARF_HI)
    d.line([(9, 42), (12, 42)], fill=C_SCARF_GOLD)  # gold fringe

    # ----------------------------------------------------
    # 4. PENGUIN HEAD & FACE (y: 19..32)
    # ----------------------------------------------------
    # Head base silhouette
    d.rectangle([8, 20, 23, 31], fill=C_FEATHER_MID, outline=C_OUTLINE)
    # Round cheeks
    d.point([(7, 25), (7, 26), (7, 27)], fill=C_OUTLINE)
    d.point([(8, 25), (8, 26), (8, 27)], fill=C_FEATHER_HI)
    d.point([(24, 25), (24, 26), (24, 27)], fill=C_OUTLINE)
    d.point([(23, 25), (23, 26), (23, 27)], fill=C_FEATHER_DARK)

    # White face patch (heart / mask shape)
    # Left eye white
    d.rectangle([10, 22, 14, 29], fill=C_WHITE_HI)
    d.point([(10, 22), (14, 22)], fill=C_WHITE_SHADOW)
    # Right eye white
    d.rectangle([17, 22, 21, 29], fill=C_WHITE_HI)
    d.point([(17, 22), (21, 22)], fill=C_WHITE_SHADOW)
    # Center forehead dark peak between eyes
    d.line([(15, 20), (16, 20)], fill=C_FEATHER_MID)
    d.line([(15, 21), (16, 21)], fill=C_FEATHER_MID)
    d.line([(15, 22), (16, 23)], fill=C_FEATHER_MID)

    # Cute big eyes
    # Left Eye (x: 11..13, y: 23..26)
    d.rectangle([11, 23, 13, 26], fill=C_EYE_PUPIL)
    d.point([(11, 23)], fill=C_EYE_SHINE) # Sparkle highlight
    d.point([(13, 26)], fill=hex_to_rgba("#384860")) # iris rim

    # Right Eye (x: 18..20, y: 23..26)
    d.rectangle([18, 23, 20, 26], fill=C_EYE_PUPIL)
    d.point([(18, 23)], fill=C_EYE_SHINE) # Sparkle highlight
    d.point([(20, 26)], fill=hex_to_rgba("#384860"))

    # Cheerful Blushing Cheeks
    d.point([(9, 27), (10, 27)], fill=C_BLUSH)
    d.point([(21, 27), (22, 27)], fill=C_BLUSH)

    # Penguin Beak (x: 13..18, y: 26..30)
    d.polygon([(13, 26), (18, 26), (17, 29), (14, 29)], fill=C_ORANGE_MID, outline=C_OUTLINE)
    d.line([(14, 26), (17, 26)], fill=C_ORANGE_HI)
    d.line([(14, 27), (17, 27)], fill=C_ORANGE_LGT)
    d.point([(15, 28), (16, 28)], fill=C_ORANGE_DARK)
    d.point([(15, 29), (16, 29)], fill=C_OUTLINE)

    # ----------------------------------------------------
    # 5. COOKING POT HAT (y: 6..21)
    # ----------------------------------------------------
    # Inverted soup pot placed on penguin's head
    # Rim / Brim of pot (resting right above the eyes / over forehead)
    # Brim is wide (x: 6..25, y: 18..20)
    d.rectangle([6, 18, 25, 20], fill=C_POT_MID, outline=C_OUTLINE)
    d.line([(7, 18), (24, 18)], fill=C_POT_LGT)
    d.line([(10, 18), (15, 18)], fill=C_POT_HI)
    d.line([(11, 19), (14, 19)], fill=C_POT_SPEC) # Specular sheen
    d.line([(7, 20), (24, 20)], fill=C_POT_DARK) # Under-rim shadow

    # Pot Main Cylindrical Body (y: 9..17, x: 8..23)
    d.rectangle([8, 9, 23, 17], fill=C_POT_MID, outline=C_OUTLINE)
    
    # Pot Bottom (which is the top surface of the hat, y: 7..9, x: 9..22)
    d.rectangle([9, 7, 22, 9], fill=C_POT_MID, outline=C_OUTLINE)
    d.line([(10, 7), (21, 7)], fill=C_POT_HI)
    d.line([(12, 7), (16, 7)], fill=C_POT_SPEC)

    # Metallic cylinder curved shading
    # Left specular gleam
    for py in range(9, 18):
        d.point([(9, py)], fill=C_POT_LGT)
        d.point([(10, py)], fill=C_POT_HI)
        d.point([(11, py)], fill=C_POT_SPEC)
        d.point([(12, py)], fill=C_POT_HI)
        d.point([(13, py)], fill=C_POT_LGT)
        # Right shadow gradient
        d.point([(20, py)], fill=C_POT_SHADOW)
        d.point([(21, py)], fill=C_POT_DARK)
        d.point([(22, py)], fill=C_POT_DARK)

    # Rivet / screw on the pot side
    d.point([(10, 14)], fill=C_POT_SPEC)
    d.point([(21, 14)], fill=C_POT_DARK)

    # Pot Handles (Asas da panela)
    # Left Handle (loop handle sticking out at x: 3..7, y: 12..16)
    d.line([(7, 12), (4, 12), (3, 13), (3, 15), (4, 16), (7, 16)], fill=C_OUTLINE, width=1)
    d.point([(4, 13), (4, 14), (4, 15)], fill=C_POT_HI)
    d.point([(5, 13), (5, 15)], fill=C_POT_LGT)
    d.point([(5, 14)], fill=hex_to_rgba("#000000", 0)) # hollow handle hole!

    # Right Handle (loop handle sticking out at x: 24..28, y: 12..16)
    d.line([(24, 12), (27, 12), (28, 13), (28, 15), (27, 16), (24, 16)], fill=C_OUTLINE, width=1)
    d.point([(27, 13), (27, 14), (27, 15)], fill=C_POT_SHADOW)
    d.point([(26, 13), (26, 15)], fill=C_POT_DARK)
    d.point([(26, 14)], fill=hex_to_rgba("#000000", 0)) # hollow handle hole!

    # Little metal knob or dent on top of the pot (bottom of pot)
    d.rectangle([14, 5, 17, 6], fill=C_POT_LGT, outline=C_OUTLINE)
    d.point([(15, 5)], fill=C_POT_SPEC)

    # Save outputs
    out_assets = os.path.join(ASSETS_DIR, "NPC_Pinguim.png")
    img.save(out_assets)
    print(f"Saved: {out_assets} ({w}x{h})")

    out_techquest = os.path.join(TECH_QUEST_DIR, "NPC_Pinguim.png")
    shutil.copyfile(out_assets, out_techquest)
    print(f"Mirrored to: {out_techquest}")

    # Also save as pinguim_panela.png for convenience
    alt_assets = os.path.join(ASSETS_DIR, "pinguim_panela.png")
    img.save(alt_assets)
    alt_techquest = os.path.join(TECH_QUEST_DIR, "pinguim_panela.png")
    shutil.copyfile(alt_assets, alt_techquest)

if __name__ == "__main__":
    generate_penguin()
