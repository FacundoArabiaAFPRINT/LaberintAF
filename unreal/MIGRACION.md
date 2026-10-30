# Laberint AF en Unreal Engine 5

Guía para pasar el juego de la versión web (`index.html`) a **Unreal Engine 5.5**, con gráficos realistas. La versión web sigue siendo la referencia: todo lo que se juega ahí está descrito en `CLAUDE.md` y sus números están en las tablas de `LaberintAF/Content/Data/`.

## Qué hay en esta carpeta

```
unreal/
├── MIGRACION.md                ← esta guía
└── LaberintAF/                 ← el proyecto de Unreal (se abre con LaberintAF.uproject)
    ├── LaberintAF.uproject
    ├── Config/                 ← gráficos realistas (Lumen, Nanite, niebla volumétrica), teclas, mapa inicial
    ├── Content/Data/           ← los datos del juego exportados de la versión web (CSV y JSON)
    └── Source/LaberintAF/      ← el código C++ del núcleo del juego
```

**El proyecto no tiene ni un archivo binario** (`.uasset`/`.umap`): el modo de juego arma todo con código al darle Play (laberinto, luz de luna, niebla, jugador, bestias, jefes, cofres y cazador). Así se puede guardar en Git sin problemas y se ve qué cambió en cada versión. Los modelos, materiales, animaciones y sonidos realistas se suman después desde el editor (fase 3).

> **Importante:** el código se escribió sin poder compilarlo (en el entorno de Claude no hay Unreal Engine ni placa de video). Lo revisaron dos lectores por separado y se corrigió lo que encontraron, pero es probable que la primera compilación tire algún error chico. Si pasa, copiá el error y pasámelo: se arregla rápido.

## Qué necesitás en tu compu

| | Mínimo | Recomendado |
|---|---|---|
| Sistema | Windows 10/11 de 64 bits | Windows 11 |
| Procesador | 4 núcleos | 8 núcleos o más |
| Memoria | 16 GB | 32 GB o más |
| Placa de video | con DirectX 12 y 6 GB (GTX 1660 / RX 5600) | 8 a 12 GB (RTX 3060 o mejor) para Lumen y Nanite cómodos |
| Disco | 150 GB libres en SSD | 250 GB en SSD (el motor ocupa ~60 GB, Megascans suma mucho) |

Programas (todos gratis):

1. **Epic Games Launcher** → pestaña Unreal Engine → Biblioteca → instalar **Unreal Engine 5.5**.
2. **Visual Studio 2022 Community** con las cargas de trabajo «Desarrollo de juegos con C++» (incluye el componente de Unreal Engine) y «Desarrollo para el escritorio con C++». Unreal la usa para compilar el código.
3. **Git** (ya lo tenés si clonás el repositorio).

Unreal es gratis hasta que el juego facture USD 1.000.000; después cobra el 5 %.

## Cómo abrirlo por primera vez

1. Cloná el repositorio `LaberintAF` y entrá a `unreal/LaberintAF`.
2. Clic derecho en `LaberintAF.uproject` → **Generate Visual Studio project files**.
3. Doble clic en `LaberintAF.uproject`. Va a decir que faltan módulos y preguntar si querés compilarlos: **Sí**. La primera vez tarda (5 a 20 minutos).
4. Cuando abre el editor, apretá **Play** (o Alt+P). Arranca la partida: estás en el laberinto, de noche, con tu farol.
5. Si querés jugar en pantalla completa: menú de Play → *Standalone Game*.

Teclas: WASD moverse · Shift correr · Espacio rodar · C agacharse · clic izquierdo pegar (mantené para el golpe cargado) · E usar (cofres, escotilla) · Q poción · F farol. Con mando: palanca izquierda, A rueda, X pega, B usa, gatillo izquierdo corre.

## Qué ya está hecho (fases 0 y 1)

| Versión web | En Unreal | Archivo |
|---|---|---|
| Tablas de armas, botín, rarezas, cazadores, bestias, tienda, contratos, logros, habilidades y personalización | Exportadas tal cual a CSV/JSON y leídas al arrancar | `Content/Data/*`, `LabDatos.*`, `tools/exportar-datos.mjs` |
| Habilidades de cazadores y jefes (hunterSkills, bossTick) | Tabla editable con forma, alcance, aviso, daño, efectos y enfriamiento; el código las ejecuta todas | `HabilidadesCazadores.csv`, `LabEnemigo.cpp` |
| Generación del laberinto (genMaze): pasillos, atajos, sala de la salida, 2 salas de jefe, 2 bóvedas, mercaderes, altares, tesoros, salones con columnas, cuevas | Mismo algoritmo en C++, muros como instancias de una malla | `LabLaberinto.*` |
| Camino de los enemigos (bfs) | Búsqueda en la grilla, sin NavMesh | `LabLaberinto::Camino` |
| Jugador: caminar 58, correr 88, aire, voltereta invulnerable, agacharse, combo de 3, golpe cargado, farol, poción | Igual, con las velocidades llevadas a centímetros | `LabPersonaje.*` |
| Vida, veneno, fuego, sangrado, aturdido, enredado, lento, ceguera, miembros cortados (pierna ×0,6, brazo ×0,62) | Componente de salud para todos | `LabSalud.*` |
| Avisos en el piso que se llenan y se esquivan rodando | Actor con malla procedural (círculo, línea, cono, mira láser) | `LabAviso.*` |
| Proyectiles, gancho del Carnicero, charcos de veneno y fuego, telarañas, cepo de Roberto | Actores propios | `LabProyectil.*`, `LabCharco.*` |
| Bestias con patrulla, sigilo («?» que se llena), búsqueda del cazador por la zona aproximada | Igual | `LabEnemigo.*` |
| Jefes: presentación, barra arriba, furia a la mitad de la vida | Igual (Guardián, Carnicero y Bruja) | `LabEnemigo.*`, `LabHUD.*` |
| Cofres común, rico, de oro y legendario sellado | Igual | `LabCofre.*` |
| Farmeo de 2 minutos, salida de los cazadores, 15 minutos, escotilla de 10 s, final | Igual | `LabModoJuego.*` |
| Interfaz | Provisoria, dibujada con el HUD (reloj, vida, aire, oro, avisos, barra del jefe, final) | `LabHUD.*` |

## Qué falta, por fases

### Fase 2 · Los diez jugadores
- Los 9 bots con su lógica (`botThink`/`botAct`: lootear, pelear o huir, ir a la salida, alianzas y traiciones, devolver llaves) y un cazador para cada uno, como en la web. Hoy solo está tu cazador.
- Llaves que se caen y se levantan, saqueo de cuerpos, escotilla compartida.
- La niebla que achica el laberinto (`FOG_ST`), los muros que se mueven (zarzas, `shiftForest`) y los eventos (lluvia, niebla espesa, gallinas, modo cabezón, noche de sangre).

### Fase 3 · Gráficos realistas
Es la parte que más trabajo lleva, y casi todo se hace en el editor, no en código.

- **Escenario:** muros de piedra, ladrillo con musgo, raíces y tierra de **Megascans** (gratis con Unreal desde **Fab**, la tienda de Epic, dentro del editor). En `LabLaberinto` se cambian `MallaMuro`, `MallaPiso` y sus materiales. Conviene armar 3 o 4 variantes de muro por bioma (bosque, cripta, pantano, nieve) y que el laberinto elija al azar para que no se repita. Nanite está prendido, así que las mallas de alta definición no bajan el rendimiento.
- **Vegetación:** arbustos para esconderse y follaje con las plantas de Megascans o el *Procedural Foliage* de Unreal.
- **Luz:** ya está armada con Lumen (la luz rebota), sombras virtuales, niebla volumétrica (la luz del farol se ve en el aire) y exposición fija para que la noche sea oscura de verdad. Falta sumar antorchas con parpadeo, partículas de polvo y luciérnagas con **Niagara**.
- **Personajes:**
  - Jugadores: **MetaHuman** (gratis, personas realistas con ropa y peinados editables). La personalización de la web (abrigo, piel, pelo, peinado, barba, accesorio, capa) pasa a las opciones del MetaHuman.
  - Cazadores, jefes y bestias: comprar o conseguir criaturas en Fab (perros, zombis, arañas, demonios, golems) o encargarlas a un artista 3D. Roberto, el ninja, Bob, Verase y la Bruja pueden salir de MetaHuman con vestuario.
- **Animaciones:**
  - Caminar, correr, rodar, agacharse y pegar: el *Game Animation Sample* de Epic (gratis, con *Motion Matching*) da movimiento muy natural.
  - Ataques y habilidades específicas: paquetes de Fab o captura de movimiento.
  - Hoy el «golpe» es provisorio: el cuerpo se adelanta y el arma gira.
- **Gore:** cortar miembros ocultando el hueso en el esqueleto y soltando una malla del miembro con física (Chaos), sangre con Niagara y calcomanías en el piso que van quedando como reguero.
- **Sonido:** MetaSounds con sonido 3D (pasos, respiración, gruñidos que se acercan), como en la web.
- **Interfaz:** con UMG, en el estilo de historieta de la web (bordes negros gruesos, sombras duras), con inventario, barra de armas, cinturón, mapa y menú de personaje.

### Fase 4 · Todo lo demás de la versión web
- Inventario completo: barra de 4 armas, mochila de 12, cinturón, armas que se gastan, revólver con recarga, rarezas y afijos.
- Mercader y altares.
- Sigilo completo: arbustos, ruido que atrae (`noise`), «!» al descubrirte.
- Contratos, logros, guía de la primera partida, momentos de la partida, «otra partida».
- Biomas y clima.
- Menú, pausa, opciones y guardado: personaje, habilidad, duración y logros, con `SaveGame`.

### Fase 5 · Opcional: multijugador en línea de verdad
Unreal trae red incorporada (replicación). El código se puede preparar para que los otros nueve jugadores sean personas. Hace falta un servidor dedicado en la nube (AWS GameLift, Edgegap o similar), que cuesta por hora de uso, y probar mucho. Es lo único de la lista de «qué hace bueno a un juego» que no se pudo hacer en la web.

## Cosas para decidir antes de la fase 3

1. **Cámara.** Quedó como en la web: desde arriba e inclinada (58°), estilo Diablo. Con gráficos realistas también funciona en tercera persona por detrás del hombro, que da más miedo pero cambia mucho el sigilo y el combate. Se cambia en `ALabPersonaje` (el brazo de la cámara).
2. **Plataforma.** Con gráficos realistas el destino natural es **PC** (Steam) y consolas. En celular habría que bajar mucho el detalle.
3. **Arte.** Lo gratis (Megascans, MetaHuman, Game Animation Sample) alcanza para el escenario, las personas y el movimiento. Las criaturas y los jefes casi seguro hay que comprarlos o encargarlos.

## Cómo seguir trabajando

- **Rebalancear:** se cambia en la versión web, se corre `node tools/exportar-datos.mjs` y se copian las tablas nuevas. También se pueden editar los CSV a mano: las distancias están en unidades de la web (1 = 12,5 cm) y los tiempos en segundos.
- **Habilidades:** cada fila de `HabilidadesCazadores.csv` es una habilidad. Para sumar una nueva a un cazador alcanza con agregar una fila con su `Duenio`.
- **Escala:** un casillero de la web (16 unidades) mide 2 m. Las velocidades se achican un 45 % (`Lab::Vel`) para que una persona real no parezca un auto.
- **Pruebas:** la versión web conserva sus pruebas (`npm test`). En Unreal, la prueba es jugar y mirar el registro (*Output Log*, categoría `LogLab`: dice cuántos datos cargó y cómo armó la partida).
