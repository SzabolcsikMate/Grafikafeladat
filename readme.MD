# Dark Museum

## Leírás

A Dark Museum egy first person nézetű horror hangulatú 3D játék OpenGL és SDL2 használatával.

A játékosnak egy sötét, procedurálisan generált múzeumszerű labirintuson kell végighaladnia, miközben fényforrásokat gyűjt össze és parkour akadályokat teljesít.

A cél az összes lámpa elérése és a kijárat megtalálása, mielőtt lejár az idő.

A játék atmoszférája a sötétségre, a dinamikus fényekre és a szűk folyosókra épül.

---

## Főbb funkciók

- First person kamera
- Egér és billentyűzet alapú mozgás
- Procedurálisan generált pálya
- Dinamikus fényforrások
- 3D modellek betöltése GLB fájlokból
- Textúrázott falak, padló és plafon
- Parkour elemek és ugrás
- Ütközésvizsgálat
- Köd és sötétedési effektek
- Death screen és escape screen
- Menü rendszer
- Fullscreen mód
- Inverz egér opció
- Interaktív UI

---

## Irányítás

| Billentyű | Funkció |
|----------|----------|
| W A S D | Mozgás |
| SPACE | Ugrás |
| Egér | Kamera mozgatás |
| ESC | Menü |
| F1 | Segítség megjelenítése |

---

## Technológiák

- C nyelv
- SDL2
- OpenGL
- stb_image
- cgltf

---

## Könyvtárszerkezet

```txt
assets/
    models/
    textures/

src/
    app.c
    game.c
    render.c
    collision.c
    texture.c
    model.c