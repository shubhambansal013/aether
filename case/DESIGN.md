# Aether Air Monitor — Case Design Specification

> Single source of truth for case geometry, component layout, and design intent.  
> Any agent generating or modifying OpenSCAD must derive all dimensions from this document.  
> Do not hardcode values that appear here as variables.

---

## 1. Device Overview

A custom double-sided PCB air quality monitor with the following stack (back to front):

```
[BACK WALL] → [PMS7003] → [NodeMCU on headers] → [PCB] → [headers] → [OLED + WS2812] → [FRONT WALL]
```

The device sits on a desk in landscape orientation with:
- **Front face** — OLED display window + WS2812 dome
- **Back face** — PMS7003 sensor grille + DHT22 vent holes
- **Right face** — Micro USB cutout + tactile button hole
- **Left face** — plain (no cutouts)
- **Top face** — plain (case lid split line runs here or along the long axis)
- **Bottom face** — plain, with optional rubber feet clearance

---

## 2. PCB

| Parameter | Value |
|---|---|
| Length (X axis, long) | 70.04 mm |
| Width (Y axis, short) | 45.3 mm |
| Thickness | 1.6 mm |
| Mounting | No screw holes. PCB retained by 4 printed corner posts gripping PCB edges with a ledge/channel. Press-fit, no adhesive. |
| Orientation reference | Bottom-left corner when looking at front face = origin (0, 0) |

---

## 3. Component Inventory & Dimensions

### 3.1 NodeMCU (back side of PCB)

| Parameter | Value |
|---|---|
| Female header height (PCB surface → NodeMCU underside) | 11 mm |
| Total height above PCB surface (incl. headers + module) | 14 mm |
| Notes | ESP-12E antenna protrudes slightly at one end. USB port and button face the right PCB edge. |

### 3.2 OLED Display (front side of PCB)

| Parameter | Value |
|---|---|
| Module PCB size | 25.2 × 26.6 mm |
| Gap: left PCB edge → left module edge | 4.7 mm |
| Gap: top PCB edge → top module edge | 8.0 mm |
| Total height: main PCB surface → glass front | 14.6 mm |
| Active display glass size | 21.74 × 10.86 mm (standard for SSD1306/SSD1315 0.96") |
| Gap: left module edge → left glass edge | 1.73 mm (derived: (25.2 − 21.74) / 2) |
| Gap: top module edge → top glass edge | 4.25 mm (measured) |

**Derived — case window cutout position (from PCB origin):**
- Window left edge: 4.7 + 1.73 = **6.43 mm**
- Window top edge: 8.0 + 4.25 = **12.25 mm**
- Window size: 21.74 × 10.86 mm
- Case cutout size (with tolerance): **22.34 × 11.46 mm** (+ 0.3 mm each side)

### 3.3 WS2812 Breakout (front side of PCB, red board)

| Parameter | Value |
|---|---|
| Board size | 17.6 × 12.25 mm |
| Gap: left PCB edge → left board edge | 13.65 mm |
| Gap: top PCB edge → top board edge | 15.8 mm |
| LED dome center: from left board edge | 7.5 mm |
| LED dome center: from top board edge | 14.8 mm |
| Total height: main PCB surface → LED dome top | 14.8 mm |

**Derived — LED dome center position (from PCB origin):**
- X: 13.65 + 7.5 = **21.15 mm from left PCB edge**
- Y: 15.8 + 14.8 = **30.6 mm from top PCB edge**

> Note: `led_dome_from_top_board_edge_mm: 14.8` equals the board height — this likely means the LED is at the very bottom edge of the red board. Verify physically before generating cutout.

### 3.4 Tactile Button (top/back edge of PCB, 90° mounted)

| Parameter | Value |
|---|---|
| Button center: from left PCB edge | 20.75 mm |
| Protrusion beyond PCB top edge | 2.4 mm |
| Stem diameter | 3.5 mm |
| Case cutout | Round hole, diameter 4.5 mm (stem + 0.5 mm clearance), on right face |

> The button is 90°-mounted so the stem points outward through the right face of the case. The stem protrudes 2.4 mm beyond the PCB edge — the case wall must have a through-hole aligned to this.

### 3.5 Micro USB Port (right edge of PCB)

| Parameter | Value |
|---|---|
| USB center: from top PCB edge | 24.45 mm |
| Shell width | 7.75 mm |
| Shell height | 2.65 mm |
| Protrusion beyond PCB right edge | 1.3 mm |
| Case cutout size | 9.75 × 4.65 mm (shell + 1 mm clearance each axis) |
| Case cutout center | Aligned to USB center from top |

### 3.6 JST Connectors (top/back edge of PCB)

| Connector | Center from left PCB edge | Height above PCB |
|---|---|---|
| PMS7003 JST | 13.3 mm | 7.6 mm |
| DHT22 JST | 34.35 mm | 6.85 mm |

> Both JST sockets face the back/top edge. Cables route internally from PCB to sensors mounted on back wall. No cutouts needed on case for JST — cables are internal.

---

## 4. External Sensors

### 4.1 PMS7003 (particulate matter sensor)

| Parameter | Value |
|---|---|
| Body dimensions | 47 × 37 × 12.2 mm |
| Mounting | Flush against inside of back wall, held by printed lip/clips |
| Inlet (fan intake) diameter | 19.7 mm |
| Inlet center from sensor corner (X) | 12.2 mm |
| Inlet center from sensor corner (Y) | 12.2 mm |
| Exhaust slot size | 11.2 × 8.4 mm |
| Exhaust from sensor corner (X) | 11.6 mm |
| Exhaust from sensor corner (Y) | 10.8 mm |
| JST connector face | Right side of sensor |
| JST center from sensor corner | 11.65 mm |
| Back wall cutout — inlet | Honeycomb pattern inside a 22 mm diameter circle (inlet + 1.15 mm margin) |
| Back wall cutout — exhaust | Slot array matching exhaust face, with 1 mm margin |

**Airflow note:** Fan intake must align to honeycomb on back wall. Exhaust points inward/upward — no case cutout needed for exhaust, just internal clearance. DHT22 must not sit in the exhaust airstream.

### 4.2 DHT22 / AM2302 (temperature + humidity sensor)

| Parameter | Value |
|---|---|
| Body dimensions (plastic, excl. pins) | 15.2 × 20.2 × 6.25 mm |
| Full module dimensions (incl. pins) | ~28 × 20 × 10 mm |
| Vent face | Front face (the side with the DHT22 label / sensing element) |
| Mounting | Seated against inside of back wall, next to PMS7003, offset from PMS exhaust |
| Back wall cutout | Grid of 2 mm holes, 3 mm pitch, over a 10 × 15 mm area aligned to sensor vent face |

**Placement rule:** DHT22 must be placed away from PMS7003 exhaust to avoid false temperature readings. Place on the inlet side or toward one corner of the back face.

---

## 5. Case Structure

### 5.1 Split Line

The case splits into **two halves along the PCB mid-plane**:
- **Back half (NodeMCU half):** contains NodeMCU cavity, PMS7003 mount, DHT22 mount, back wall vents, button hole, USB cutout
- **Front half (OLED half):** contains OLED window, WS2812 dome recess + hole, front wall

The split line runs along the plane of the main PCB. The PCB sits in a channel/ledge formed by both halves meeting.

### 5.2 Dimensions

| Parameter | Value | Derivation |
|---|---|---|
| Wall thickness | 2.0 mm | Preference |
| Fit tolerance | 0.3 mm | Preference |
| Case width (X) | 70.04 + 2×2.0 + 2×0.3 = **74.64 mm** | PCB length + walls + tolerance |
| Case height (Y) | 45.3 + 2×2.0 + 2×0.3 = **49.9 mm** | PCB width + walls + tolerance |
| Back half depth | 2.0 (back wall) + 12.2 (PMS7003) + gap + 14.0 (NodeMCU) + 0.3 = **~30 mm** | Back wall → PCB back surface |
| Front half depth | 14.6 (OLED to glass) + 2.0 (front wall) = **~17 mm** | PCB front surface → front wall outer |
| Total depth | ~47 mm (back to front) | Sum of both halves |

> Back half depth has a gap between PMS7003 body and NodeMCU. PMS7003 is 12.2 mm deep, NodeMCU extends 14 mm from PCB. These overlap spatially only if PMS is not directly behind NodeMCU — the PMS (47×37 mm) and NodeMCU (49×26 mm) may need relative offset to avoid collision. Resolve in geometry: PMS sits offset to one side if needed, or back wall is deepened by max(12.2, 14) = 14 mm from PCB to back wall inner surface, making back half = 2 + 14 + 0.3 ≈ **16.3 mm**.

### 5.3 PCB Retention

Four corner posts inside the case, each with a 1.5 mm deep ledge that grips the PCB edge. Post height = full interior height. Posts are part of the case body, not separate inserts. No screws, no adhesive — friction fit with 0.1 mm clearance on each side.

Corner post positions: 2 mm inset from each corner of the inner case wall.

### 5.4 Lid / Join Mechanism

- **Style:** Snap-fit
- **Mechanism:** 4 cantilever clips (one per side, centered), clip length 8 mm, 0.5 mm lip, 1 mm ramp chamfer
- **Location:** On the split line rim, clips on back half, receivers on front half
- **Rim width:** 3 mm (forms the mating surface at the split line)

---

## 6. WS2812 Dome

| Parameter | Value |
|---|---|
| Dome outer diameter | 12 mm |
| Dome height (hemisphere) | 6 mm |
| Dome wall thickness | 1.0 mm |
| Collar outer diameter | 13 mm |
| Collar height | 2.5 mm |
| Snap lip thickness | 0.5 mm |
| Snap lip overhang | 0.4 mm |
| Recess in front wall | 13.6 mm diameter × 1.5 mm deep (hides collar base, covers tolerance gap) |
| Dome material / colour | White or natural PLA (semi-translucent when thin) |
| Body material / colour | Brown PLA (or user choice) |
| LED clearance inside | Min 3 mm air gap between LED dome top and case inner surface |

**Dome center position on front face (from PCB origin → translated to case outer face):**
- X: 21.15 mm from left PCB edge → 21.15 + 2.0 (wall) + 0.3 (tolerance) = **23.45 mm from left case edge**
- Y: 30.6 mm from top PCB edge → 30.6 + 2.0 + 0.3 = **32.9 mm from top case edge**

**Print orientation:** Dome printed dome-up on print bed. Thin walls (~1 mm) at 0.2 mm layer height, 2 perimeters, 0% infill in dome region = natural translucency.

---

## 7. Back Face Layout

Looking at the back face from outside:

```
┌─────────────────────────────────────────────┐
│                                             │
│   ┌─────────────────────────┐  ┌─────────┐  │
│   │                         │  │  DHT22  │  │
│   │       PMS7003           │  │  vent   │  │
│   │                         │  │  grid   │  │
│   │   ◯ (honeycomb inlet)   │  └─────────┘  │
│   │   ═══ (exhaust slots)   │               │
│   └─────────────────────────┘               │
│                                             │
└─────────────────────────────────────────────┘
```

- PMS7003 sits left/center, DHT22 sits right, separated by at least 5 mm
- DHT22 vent grid must NOT be in line with PMS7003 exhaust
- JST cables route internally along the top edge of the back half

---

## 8. Right Face Layout

Looking at the right face from outside:

```
┌─────────────────────────────┐
│                             │
│   ┌──────────┐              │
│   │  USB     │              │
│   │  cutout  │              │
│   └──────────┘              │
│                             │
│              ◯ (button)     │
│                             │
└─────────────────────────────┘
```

- USB cutout centered at 24.45 mm from top case edge (inner), rectangular 9.75 × 4.65 mm with 1 mm corner radius
- Button hole centered at 20.75 mm from left case edge (inner), circular 4.5 mm diameter
- Button hole Y position: derived from button's position on PCB top edge, translated to right face

---

## 9. OpenSCAD Implementation Notes

- All measurements must be declared as named variables at the top of the file, grouped by component
- A `debug_pcb = false` toggle renders a transparent green box representing the PCB + components inside the assembled case
- The dome is a **separate module/file** — it must be printable independently
- The back half and front half are **separate modules** rendered individually for printing
- Use `$fn = 64` for circles and cylinders
- Honeycomb pattern on back wall: hexagon diameter 4 mm, wall between hexagons 1 mm, clipped to sensor inlet circle
- DHT22 vent grid: 2 mm round holes on 3 mm pitch grid, clipped to 10 × 15 mm rectangle
- Corner post ledge: `difference()` of a tall post with a channel cut at PCB height
- Snap clips: cantilever beam with a triangular cross-section tip (ramp on insertion side, flat on lock side)
- External edges: 1 mm fillet radius on all outer edges for print quality and feel
- No supports should be required for either half when printed in the correct orientation (back half open-face-up, front half open-face-up)

---

## 10. Outstanding / To Verify Before Code Generation

| Item | Status | Note |
|---|---|---|
| WS2812 LED dome Y position | ⚠️ Verify | `led_dome_from_top_board_edge_mm: 14.8` equals the board height — LED may be at very bottom edge of red board. Confirm physically. |
| PMS7003 vs NodeMCU overlap | ⚠️ Resolve | Check if PMS (47 mm wide) and NodeMCU (49 mm wide) need lateral offset to avoid collision in the back half. |
| Button hole Y on right face | ⚠️ Derive | Button is on top/back edge of PCB — need to confirm which face this translates to when device is desk-standing. |
| DHT22 exact back-face position | 🔲 Not set | Decide exact X/Y placement on back face relative to PMS7003. |
| Snap-fit clip count and positions | 🔲 Not set | 4 clips assumed — confirm placement doesn't conflict with corner posts or cutouts. |
| Rubber feet | 🔲 Not decided | Add 4 × recessed rubber foot positions on bottom face? |
| Cable management | 🔲 Not decided | Internal routing guide for JST cables from PCB top edge to sensors on back wall. |
