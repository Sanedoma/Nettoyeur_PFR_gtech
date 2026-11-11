# Journal — editeur-contenu

> Entrées selon `doc/CONVENTION.md`, la plus récente en haut.

---

## T000 (partie A) — Vérifier la config éditeur et documenter l'existant — 2026-11-11

**Objectif :** savoir exactement comment le prototype est câblé côté éditeur (Blueprints, inputs, widget, niveaux, réglages) avant de toucher au code, pour que les tâches T001+ partent d'une base connue.
**Étape du GDD :** préalable (§10.1).
**Branche :** `tache/T000-verification-existant`

> Cette partie est en **lecture seule** : aucun asset n'a été créé, modifié, sauvegardé ni supprimé. Toutes les informations ci-dessous ont été relevées dans l'éditeur ouvert (Unreal 5.7.4), puis chaque relevé est retraduit en « comment le voir toi-même dans l'éditeur ».

### 1. Fichiers créés / modifiés
- `doc/agents/editeur-contenu.md` — modifié — cette entrée.
- `doc/notions.md` — modifié — ajout des notions éditeur rencontrées (sections Éditeur, Moteur, Physique, Entrées, Interface, Outils). Les notions C++ ne sont pas touchées (partie B).
- **Aucun asset** (`.uasset` / `.umap`) modifié.

### 2. Implémentation du code
Aucune (pas de C++ dans cette partie). Les notions éditeur sont expliquées au fil du §4, dans des blocs « Notion nouvelle ».

### 3. Configuration
Aucune modification. Relevé des réglages existants :

| Réglage | Où le voir | Valeur relevée |
|---|---|---|
| Carte par défaut du jeu (Game Default Map) | **Modifier (Edit) → Paramètres du projet (Project Settings) → Projet → Cartes et modes (Maps & Modes)** → *Default Maps* | `/Game/FirstPerson/Lvl_FirstPerson` |
| Carte de démarrage de l'éditeur (Editor Startup Map) | même endroit | `/Game/FirstPerson/Lvl_FirstPerson` |
| Mode de jeu global par défaut (Default GameMode) | même endroit → *Default Modes* | `BP_FirstPersonGameMode` |
| Fichier correspondant | `Config/DefaultEngine.ini`, section `[/Script/EngineSettings.GameMapsSettings]` | mêmes valeurs (`GameDefaultMap`, `EditorStartupMap`, `GlobalDefaultGameMode`) |
| Exécution Python à distance | **Project Settings → Plugins → Python → Enable Remote Execution** | activé (`bRemoteExecution=True` dans `DefaultEngine.ini`) — c'est ce qui permet aux outils d'inspecter l'éditeur |

### 4. Étapes dans l'éditeur (relevés des 8 points)

#### Comment ces relevés ont été faits (et comment les refaire)
Les outils MCP `mcp__unreal__*` ne trouvaient pas l'éditeur au démarrage (l'éditeur répond lentement à la découverte réseau quand il est en arrière-plan). L'inspection a donc été faite par l'exécution Python à distance de l'éditeur, **en lecture seule** : lecture des classes parentes, des composants, des Class Defaults, des Input Mapping Contexts, et **export texte des graphes Blueprint** (le même texte que celui que l'éditeur met dans le presse-papiers quand on copie des nœuds).

**Équivalent manuel** — tout ce qui suit se vérifie à la main :
- Classe parente : ouvrir le Blueprint, regarder en haut à droite **Classe parente (Parent class)**, ou bouton **Paramètres de classe (Class Settings)** → panneau **Détails** → *Class Options* → **Parent Class**.
- Composants : ouvrir le Blueprint → panneau **Composants (Components)** en haut à gauche.
- Graphe : ouvrir le Blueprint → onglet **Event Graph** (ou une fonction dans **Mon Blueprint (My Blueprint)** → *Fonctions*).
- Astuce pour relever un graphe nœud par nœud sans se tromper : voir la notion [Copier des nœuds en texte](#notion-copier-noeuds-texte).

<a id="notion-copier-noeuds-texte"></a>
> **Notion nouvelle — Copier des nœuds en texte (T3D)**
> Dans un graphe Blueprint, sélectionne des nœuds puis **Ctrl+C**, et colle (**Ctrl+V**) dans un éditeur de texte (Bloc-notes). Tu obtiens un texte qui commence par `Begin Object Class=/Script/BlueprintGraph.K2Node_...`. Chaque nœud y liste ses broches (`CustomProperties Pin (PinName="execute", ... LinkedTo=(K2Node_CallFunction_13 ...))`). La partie `LinkedTo` dit **à quelle broche de quel nœud** chaque broche est reliée. C'est le moyen le plus fiable de relever un graphe, et c'est exactement ce que les outils ont lu ici. À l'inverse, coller ce texte dans un graphe recrée les nœuds (pratique pour partager un graphe).

---

#### Point 1 — Classe parente de `BP_FirstPersonCharacter`

**Résultat : la classe parente est bien `PersonnageNettoyeur` (`APersonnageNettoyeur`, C++).**

| Blueprint | Dossier | Classe parente |
|---|---|---|
| `BP_FirstPersonCharacter` | `Content/FirstPerson/Blueprints/` | **`PersonnageNettoyeur`** (C++, module Nettoyeur) |
| `BP_FirstPersonCharacter_Backup` | `Content/FirstPerson/Blueprints/` | `Character` (classe moteur) |
| `BP_FirstPersonGameMode` | `Content/FirstPerson/Blueprints/` | `GameModeBase` |
| `BP_FirstPersonPlayerController` | `Content/FirstPerson/Blueprints/` | `PlayerController` |
| `BP_FirstPersonCameraManager` | `Content/FirstPerson/Blueprints/` | `PlayerCameraManager` |
| `BP_PreuveNettoyable` | `Content/Nettoyeur/Preuves/` | **`PreuveNettoyable`** (C++) |
| `BP_ObjetGrab_Test1` | `Content/Nettoyeur/Preuves/` | `Actor` |
| `BP_ObjetPull_Test` | `Content/Nettoyeur/Preuves/` | `Actor` |
| `WBP_Interaction` | `Content/Nettoyeur/` | `UserWidget` |

**Pour le vérifier toi-même :**
1. **Content Browser** → dossier `Content/FirstPerson/Blueprints`.
2. Double-clique sur `BP_FirstPersonCharacter`.
3. En haut à droite de la fenêtre du Blueprint : texte **Classe parente : PersonnageNettoyeur**.
4. (Variante) Bouton **Paramètres de classe (Class Settings)** dans la barre d'outils → panneau **Détails** → *Class Options* → **Parent Class** = `PersonnageNettoyeur`.

<a id="notion-blueprint-enfant"></a>
> **Notion nouvelle — Blueprint enfant (classe parente)**
> Un Blueprint est toujours une **classe enfant** d'une autre classe, sa **classe parente**. Il **hérite** de tout ce que le parent définit : composants, variables, fonctions. Ici `BP_FirstPersonCharacter` est l'enfant de la classe C++ `APersonnageNettoyeur` : tout ce que le C++ crée (le `PhysicsHandle`, l'`InteractionComponent`) apparaît automatiquement dans le Blueprint, marqué « hérité ». C'est la règle du projet : **la logique en C++ (le parent), la configuration en Blueprint (l'enfant)**. Changer la classe parente d'un Blueprint existant s'appelle le **reparentage** (Class Settings → Parent Class) ; c'est une opération risquée (on peut perdre des références), d'où la règle « commit propre avant ».

---

#### Point 2 — Composants de `BP_FirstPersonCharacter` (et ordre)

**Résultat : `ComposantNettoyage` ET `InteractionComponent` sont présents.**

Ordre tel qu'il apparaît dans le panneau **Composants** (arbre des composants « de scène » d'abord, puis les composants sans position) :

```
BP_FirstPersonCharacter (Self)
└─ CapsuleComponent            (hérité — Character)        ← composant racine
   ├─ ArrowComponent           (hérité — Character)
   └─ Mesh                     (hérité — Character)        SkeletalMesh = SKM_Manny_Simple
      └─ FirstPersonMesh       (ajouté dans le Blueprint)  SkeletalMesh = SKM_Manny_Simple
         └─ FirstPersonCamera  (ajouté dans le Blueprint)  FOV 90, Use Pawn Control Rotation = coché
CharacterMovement              (hérité — Character)
InteractionComponent           (hérité — C++ APersonnageNettoyeur)
PhysicsHandle                  (hérité — C++ APersonnageNettoyeur)
ComposantNettoyage             (ajouté dans le Blueprint, pas en C++)
```

Réglages lus sur ces composants :

| Composant | Propriété (catégorie dans Détails) | Valeur |
|---|---|---|
| `InteractionComponent` | `Trace Distance` (Interaction \| Pull) | 300 |
| | `Hold Distance` | 150 |
| | `Hold Interp Speed` | 6 |
| `PhysicsHandle` | `Linear Damping` / `Linear Stiffness` | 200 / 750 |
| | `Angular Damping` / `Angular Stiffness` | 500 / 1500 |
| | `Interpolation Speed` | 50 |
| | `Soft Linear Constraint` / `Soft Angular Constraint` / `Interpolate Target` | cochés |
| `ComposantNettoyage` | (aucune propriété éditable, seulement l'événement `On Progression Nettoyage Changed`) | — |

Point à noter : `InteractionComponent` et `PhysicsHandle` sont créés **en C++** (dans le constructeur d'`APersonnageNettoyeur`), alors que `ComposantNettoyage` a été **ajouté à la main dans le Blueprint** (bouton **+ Ajouter**). Les deux marchent, mais ce n'est pas homogène (à garder en tête pour T001).

Le socket exact sur lequel `FirstPersonCamera` est attachée à `FirstPersonMesh` n'a **pas** pu être lu par script. **Pour le relever :** sélectionne `FirstPersonCamera` dans **Composants** → panneau **Détails** → catégorie **Transform** / ligne **Parent Socket**.

**Pour le vérifier toi-même :**
1. Ouvre `BP_FirstPersonCharacter`, onglet **Fenêtre d'affichage (Viewport)** ou **Event Graph** (le panneau Composants est visible dans les deux).
2. Panneau **Composants** en haut à gauche : la liste ci-dessus. Les composants hérités ont la mention *(hérité)* / *(Inherited)* et ne peuvent pas être supprimés ni renommés.
3. Clique sur `InteractionComponent` → panneau **Détails** → catégorie **Interaction | Pull** : les trois valeurs ci-dessus.

<a id="notion-composants-herites"></a>
> **Notion nouvelle — Panneau Composants, composant hérité ou ajouté**
> Un **composant** est une brique attachée à un acteur (une caméra, un mesh, un comportement…). Dans le panneau **Composants** d'un Blueprint, on voit deux sortes de composants : les **hérités** (créés par la classe parente — en C++ avec `CreateDefaultSubobject`, ou par un Blueprint parent) et les **ajoutés** dans ce Blueprint avec **+ Ajouter (Add)**. Les composants « de scène » (qui ont une position : caméra, mesh, capsule) forment un **arbre** : un enfant suit son parent quand il bouge. Les composants « logiques » (sans position, comme `CharacterMovement` ou `ComposantNettoyage`) sont listés à part, sous l'arbre. Le **composant racine** (Root) est le sommet de l'arbre : c'est lui qui définit la position de l'acteur.

---

#### Point 3 — Graphe d'événements de `BP_FirstPersonCharacter` (Essuyer, Grab, Pull)

Le graphe **Event Graph** contient : les entrées du modèle First Person (Move, Look, MouseLook, Jump, contrôles tactiles), plus **4 blocs ajoutés pour Nettoyeur** : Essuyer, Pull, Grab, et l'initialisation du widget (point 4).

Noms de nœuds : je donne le nom **anglais** (nom interne, c'est aussi ce qu'on tape dans la recherche du menu clic droit). Si ton éditeur affiche les nœuds en français, l'intitulé peut être traduit (ex. *Event BeginPlay* → *Événement BeginPlay*). Option : **Modifier → Préférences de l'éditeur → Général → Région et langue → Utiliser des noms localisés pour les nœuds** (décoché = noms anglais).

##### 3.a — Essuyer (`IA_Essuyer`, touche E)
Nœuds :
1. **EnhancedInputAction IA_Essuyer** (événement d'entrée, clic droit → chercher « IA_Essuyer »).
2. **Get ComposantNettoyage** (variable, glissée depuis le panneau Composants).
3. **Tenter Nettoyage** (fonction de `ComposantNettoyage`, catégorie *Nettoyage*).

Connexions :
- broche **Started** de *IA_Essuyer* → broche **exec (entrée)** de *Tenter Nettoyage*
- broche **Composant Nettoyage** de *Get ComposantNettoyage* → broche **Target** de *Tenter Nettoyage*
- Les broches *Triggered, Ongoing, Canceled, Completed, Action Value* de *IA_Essuyer* ne sont **pas** reliées.

→ Un appui sur E appelle **une seule fois** `UComposantNettoyage::TenterNettoyage()` (celle du **composant**, pas la version vide `APersonnageNettoyeur::TenterNettoyage()`, qui n'est appelée par aucun Blueprint).

**Nœuds morts à côté (jamais exécutés)** — un bloc de test resté dans le graphe :
- **Get FirstPersonCamera** → **Get World Location** (`K2_GetComponentLocation`) et → **Get Forward Vector**
- **Get Forward Vector** (Return Value) → broche **A** d'un **Multiply** (vecteur × float, B = **300**)
- **Multiply** (Return Value) → **A** d'un **Add** (vecteur + vecteur) ; **Get World Location** (Return Value) → **B** de l'**Add**
- **Get World Location** (Return Value) → **Start** de **Line Trace By Channel** (`LineTraceSingle`) ; **Add** (Return Value) → **End**
- Réglages du Line Trace : Trace Channel = **Visibility** (TraceTypeQuery1), Draw Debug Type = **For Duration**, Draw Time = **5**, Ignore Self = coché
- **Out Hit** → **Break Hit Result** (aucune sortie utilisée)
- broche **exec sortie** du Line Trace → **Print String** (« Touche E détécter », bleu, 2 s, *Development Only*)
- **La broche exec d'entrée du Line Trace n'est reliée à rien** : ce bloc ne s'exécute **jamais**. C'est un ancien test, sans effet en jeu. Il ne faut pas le confondre avec le Line Trace permanent fait en C++ par `UComposantNettoyage` (dette n° 1).

##### 3.b — Pull (`IA_Pull`, touche R)
Nœuds : **EnhancedInputAction IA_Pull**, **Get InteractionComponent**, **Pull** (fonction d'`InteractionComponent`, catégorie *Interaction | Pull*), **Release Grab** (même composant).

Connexions :
- **Started** de *IA_Pull* → **exec** de *Pull*
- **Completed** de *IA_Pull* → **exec** de *Release Grab*
- **Canceled** de *IA_Pull* → **exec** de *Release Grab* (les deux broches vont au même nœud)
- **Interaction Component** de *Get InteractionComponent* → **Target** de *Pull* et → **Target** de *Release Grab*

→ R enfoncée : on commence à tirer ; R relâchée (ou action annulée) : on lâche.

##### 3.c — Grab (`IA_Grab`, clic gauche)
Nœuds : **EnhancedInputAction IA_Grab**, **Get InteractionComponent** (un second exemplaire), **Grab** (catégorie *Interaction | Grab*), **Release Grab**.

Connexions :
- **Started** de *IA_Grab* → **exec** de *Grab*
- **Interaction Component** de *Get InteractionComponent* → **Target** de *Grab* et → **Target** de *Release Grab*
- **Le nœud *Release Grab* de ce bloc n'a AUCUNE broche exec reliée** : il ne s'exécute jamais. Les broches *Completed* / *Canceled* de *IA_Grab* sont libres.

→ **Écart important :** un clic gauche saisit l'objet, mais **relâcher le clic ne le lâche pas**. Re-cliquer ne le lâche pas non plus (le C++ `Grab()` sort immédiatement si on tient déjà un objet). En l'état, le seul moyen de lâcher un objet saisi par Grab est de relâcher **R** (Completed de IA_Pull → Release Grab, qui agit sur le même PhysicsHandle). C'est cohérent avec le commit « début du Grab » : le Grab est inachevé.

##### 3.d — Le reste du graphe (modèle First Person, inchangé)
| Événement | Appelle | Remarque |
|---|---|---|
| IA_Move **Triggered** (Action Value X / Y) | fonction **Move** (Left/Right, Forward/Backward) | |
| IA_Look **Triggered** | fonction **Aim** (Yaw, Pitch) | manette |
| IA_MouseLook **Triggered** | fonction **Aim** | souris |
| IA_Jump **Started** / **Completed** | **Jump** / **Stop Jumping** | |
| Primary/Secondary Thumbstick, Touch Jump Start/End | Move / Aim / Jump / Stop Jumping | interface tactile `BPI_TouchInterface` (inutile sur PC) |

<a id="notion-event-graph"></a>
> **Notion nouvelle — Event Graph, broches d'exécution et de données**
> L'**Event Graph** est le graphe où l'on réagit à des **événements** (début de partie, touche pressée…). Un nœud a deux sortes de broches : les **broches d'exécution** (blanches, en forme de flèche) qui donnent **l'ordre** dans lequel les nœuds s'exécutent, et les **broches de données** (colorées) qui transportent des valeurs (un vecteur, un objet, un booléen). Règle à retenir : **un nœud dont la broche exec d'entrée n'est reliée à rien ne s'exécute jamais**, même s'il est rempli (c'est le cas du Line Trace de test et du Release Grab orphelin ci-dessus). Les nœuds « purs » (sans broche exec, ex. *Get Forward Vector*, *Multiply*) sont calculés seulement quand un nœud exécuté lit leur sortie. Un *Print String* marqué **Development Only** n'apparaît pas dans un jeu packagé en Shipping.

**Pour le vérifier toi-même :**
1. Ouvre `BP_FirstPersonCharacter` → onglet **Event Graph**.
2. Cherche les nœuds rouges **IA_Essuyer**, **IA_Pull**, **IA_Grab** (Ctrl+F dans le graphe, tape le nom).
3. Suis les fils blancs depuis les broches *Started* / *Completed* / *Canceled*.
4. Repère le nœud *Release Grab* sous *Grab* : aucun fil blanc n'y arrive.

---

#### Point 4 — `WBP_Interaction` : création, affichage, `DefinirWidgetInteraction`, `AfficherInteraction`, progression

##### 4.a — Création du widget (Event Graph de `BP_FirstPersonCharacter`)
Nœuds, dans l'ordre d'exécution :
1. **Event BeginPlay**
2. **Create Widget** — *Class* = **WBP_Interaction** ; *Owning Player* ← **Get Player Controller** (*Player Index* = 0)
3. **Set NewVar** — variable du Blueprint, type *WBP_Interaction* (référence objet). **Elle s'appelle encore `NewVar`** (nom par défaut d'Unreal, jamais renommée).
4. **Definir Widget Interaction** (fonction de `ComposantNettoyage`, catégorie *Interface*) — *Target* ← **Get ComposantNettoyage** ; *Widget* ← sortie de *Create Widget*
5. **Bind Event to On Progression Nettoyage Changed** — *Target* ← **Get ComposantNettoyage** ; *Event* ← événement personnalisé **RecevoirProgressionNettoyage** (voir 4.d)
6. **Add to Viewport** — *Target* ← sortie de *Create Widget* ; *ZOrder* = 0

Connexions d'exécution : *Event BeginPlay* **then** → *Create Widget* → *Set NewVar* → *Definir Widget Interaction* → *Bind Event to On Progression Nettoyage Changed* → *Add to Viewport*.
Connexions de données : **Return Value** de *Create Widget* → **NewVar** de *Set NewVar*, → **Widget** de *Definir Widget Interaction*, → **Target** de *Add to Viewport*. **Return Value** de *Get Player Controller* → **Owning Player** de *Create Widget*. **Output Delegate** (petit carré rouge) de *RecevoirProgressionNettoyage* → **Event** de *Bind Event to…*.

→ Le widget est créé **par le personnage** (pas par le PlayerController ni le HUD), au BeginPlay, puis donné au C++ par `DefinirWidgetInteraction`. Le C++ appelle ensuite la fonction Blueprint `AfficherInteraction` par son nom (`FindFunction` + `ProcessEvent`, dette n° 5).

##### 4.b — Contenu visuel de `WBP_Interaction` (onglet **Concepteur / Designer**)
Hiérarchie (panneau **Hiérarchie**) :
```
[WBP_Interaction]
└─ CanvasPanel (CanvasPanel_1058)
   ├─ TexteObjectifMission  (Text)        visible   texte « Mission 1 — Première maison »   ancre haut-gauche, position (40, 40)
   ├─ BarProgression        (ProgressBar) **Hidden** Percent = 0                               ancre haut-gauche, position (80, 112), taille 236 × 18
   └─ TexteInteraction      (Text)        **Hidden** texte « Appuie sur E pour nettoyer »       ancre au centre, position (5, 134), alignement 0.5
```
Les trois sont cochés **Is Variable** (sinon on ne pourrait pas les lire dans le graphe).

##### 4.c — Fonction `AfficherInteraction` (onglet **Graphe**, panneau **Mon Blueprint** → *Fonctions*)
Entrée : un paramètre **Afficher** (booléen).
Nœuds : **AfficherInteraction** (nœud d'entrée), **Branch**, 4 × **Set Visibility**, **Get TexteInteraction**, **Get BarProgression**.

Connexions :
- **then** de l'entrée → **exec** de *Branch* ; **Afficher** → **Condition** de *Branch*
- *Branch* **True** → *Set Visibility* (Target = TexteInteraction, In Visibility = **Visible**) → *Set Visibility* (Target = BarProgression, **Visible**)
- *Branch* **False** → *Set Visibility* (Target = TexteInteraction, **Hidden**) → *Set Visibility* (Target = BarProgression, **Hidden**)

→ Le texte et la barre apparaissent / disparaissent **ensemble**. Le texte est figé : il dit toujours « Appuie sur E pour nettoyer » (pas de texte différent pour Grab / Pull).

##### 4.d — Événement lié à `OnProgressionNettoyageChanged` (Event Graph du personnage)
Événement personnalisé **RecevoirProgressionNettoyage** (nœud *Custom Event*), une sortie **Progression** (float).
Nœuds : *RecevoirProgressionNettoyage*, **>=** (*Greater Equal*, B = **1.0**), **Branch** n° 1, **Get MissionTerminee** (booléen du Blueprint, défaut *false*), **NOT Boolean**, **Branch** n° 2, **Set MissionTerminee** (= **true**), **Afficher Mission Terminee** (fonction du widget), **Mettre A Jour Progression** (fonction du widget), **Get NewVar** (via la sortie de *Set NewVar*).

Connexions :
- **Progression** → **A** de *>=* ; **Progression** → **Progression** de *Mettre A Jour Progression*
- **then** de l'événement → **exec** de *Branch 1* ; **Return Value** de *>=* → **Condition** de *Branch 1*
- *Branch 1* **False** (progression < 1) → **exec** de *Mettre A Jour Progression*
- *Branch 1* **True** (progression ≥ 1) → **exec** de *Branch 2*
- **MissionTerminee** → **A** de *NOT* ; **Return Value** de *NOT* → **Condition** de *Branch 2*
- *Branch 2* **True** (mission pas encore terminée) → *Set MissionTerminee* (coché) → *Afficher Mission Terminee* → *Mettre A Jour Progression*
- *Branch 2* **False** : rien n'est relié
- **Target** de *Afficher Mission Terminee* et de *Mettre A Jour Progression* ← sortie **Output Get** de *Set NewVar* (donc le widget créé au BeginPlay)

Fonctions appelées dans le widget :
- **MettreAJourProgression** (entrée *Progression*, double) : *Print String* « (Fonction Appelee) » (*Development Only*) → *Set Percent* (Target = BarProgression, In Percent ← Progression) → *Print String* (texte ← *To String (Float)* de Progression, *Development Only*). Les deux Print String affichent du debug à l'écran à chaque essuyage.
- **AfficherMissionTerminee** : *Set Text* (Target = TexteObjectifMission, In Text = « MISSION TERMINÉE ! »).

→ **Écart à signaler :** la « mission terminée » est décidée **dans le Blueprint du personnage**, dès qu'**une** preuve atteint 100 % (pas quand toutes les preuves sont nettoyées). C'est une logique de jeu en Blueprint, contraire à la règle « logique en C++ » : à reprendre quand l'inspection existera (GDD).

<a id="notion-widget-blueprint"></a>
> **Notion nouvelle — Widget Blueprint (UMG) : Concepteur, Graphe, Create Widget, Add to Viewport**
> Un **Widget Blueprint** (`WBP_`) est un élément d'interface 2D (texte, barre, bouton). Il a deux onglets : **Concepteur (Designer)**, où l'on place les éléments visuellement dans une **Hiérarchie** (ici un *Canvas Panel* qui contient deux textes et une barre), et **Graphe (Graph)**, où l'on écrit ses fonctions. Un élément n'est accessible dans le graphe que si la case **Is Variable** est cochée dans ses Détails. Pour l'afficher en jeu, il faut deux étapes : **Create Widget** (crée l'objet en mémoire, il faut lui donner un *Owning Player*) puis **Add to Viewport** (le dessine par-dessus l'image du jeu). La **Visibilité** *Hidden* cache un élément tout en gardant sa place ; *Collapsed* le cache et libère la place.

<a id="notion-bind-event"></a>
> **Notion nouvelle — Bind Event to (s'abonner à un événement C++) et Custom Event**
> Le C++ de `UComposantNettoyage` expose un **événement diffusable** `OnProgressionNettoyageChanged` (un *delegate dynamique multicast*, expliqué côté C++ en partie B). En Blueprint, on s'y **abonne** avec le nœud **Bind Event to On Progression Nettoyage Changed** : on lui branche, dans sa broche **Event** (carré rouge), la broche **Output Delegate** d'un **Custom Event** (événement personnalisé, clic droit → *Add Custom Event*). À partir de là, chaque fois que le C++ « diffuse » l'événement (`Broadcast`), le Custom Event s'exécute avec la valeur reçue. Le Custom Event doit avoir **les mêmes paramètres** que l'événement C++ (ici un float *Progression*) ; le plus simple pour l'obtenir est de tirer un fil depuis la broche *Event* et de choisir **Add Custom Event**, Unreal crée alors la bonne signature.

**Pour le vérifier toi-même :**
1. `BP_FirstPersonCharacter` → **Event Graph** → cherche **Event BeginPlay** et suis la chaîne.
2. Double-clique sur `WBP_Interaction` (`Content/Nettoyeur`) → onglet **Concepteur** → panneau **Hiérarchie** ; clique sur chaque élément → **Détails** → *Visibility*, *Text*, *Is Variable*.
3. Onglet **Graphe** → panneau **Mon Blueprint** → *Fonctions* → double-clic sur `AfficherInteraction`, `MettreAJourProgression`, `AfficherMissionTerminee`.

---

#### Point 5 — Touches de `IMC_Default` (et de `IMC_MouseLook`)

`IMC_Default` (`Content/Input/IMC_Default`) :

| Input Action | Touche | Modificateurs | Déclencheurs (Triggers) |
|---|---|---|---|
| `IA_Jump` | Barre espace | — | — |
| `IA_Jump` | Manette : bouton bas (A / Croix) | — | — |
| `IA_Move` | **Z** | Swizzle Input Axis Values (YXZ) | — |
| `IA_Move` | **S** | Swizzle (YXZ) + Negate (X, Y, Z) | — |
| `IA_Move` | **Q** | Negate (X, Y, Z) | — |
| `IA_Move` | **D** | — | — |
| `IA_Move` | Flèche haut | Swizzle (YXZ) | — |
| `IA_Move` | Flèche bas | Swizzle (YXZ) + Negate | — |
| `IA_Move` | Flèche droite | — | — |
| `IA_Move` | Flèche gauche | Negate | — |
| `IA_Move` | Manette : stick gauche 2D | Dead Zone (radiale, 0.2 → 1) + Scalar (1, 1, 1) | — |
| `IA_Look` | Manette : stick droit 2D | Dead Zone (radiale, 0.2 → 1) | — |
| **`IA_Essuyer`** | **E** | — | — |
| **`IA_Pull`** | **R** | — | — |
| **`IA_Grab`** | **Clic gauche (Left Mouse Button)** | — | — |

`IMC_MouseLook` (`Content/Input/IMC_MouseLook`) : `IA_MouseLook` ← **Souris XY 2D-Axis**, modificateur **Negate sur Y seulement** (pour que pousser la souris vers l'avant fasse lever la tête).

Les touches de déplacement sont **ZQSD** (clavier AZERTY) au lieu du WASD du modèle.

Les Input Actions :

| Asset (`Content/Input/Actions/`) | Value Type | Triggers sur l'action |
|---|---|---|
| `IA_Essuyer` | Digital (bool) | aucun |
| `IA_Grab` | Digital (bool) | aucun |
| `IA_Pull` | Digital (bool) | aucun |
| `IA_Jump` | Digital (bool) | Pressed, Released |
| `IA_Move` | Axis2D (Vector2D) | aucun |
| `IA_Look` | Axis2D | aucun |
| `IA_MouseLook` | Axis2D | aucun |

**Qui active ces IMC ?** Ce n'est pas le personnage mais **`BP_FirstPersonPlayerController`** (Event Graph) :
1. **Event BeginPlay** → **Branch** (Condition ← **Is Local Player Controller**)
2. *True* → **Sequence**
3. Sequence **Then 0** → **Add Mapping Context** (Target ← **Enhanced Input Local Player Subsystem** ; *Mapping Context* = **IMC_Default** ; *Priority* = 0)
4. Sequence **Then 1** → **Branch** (Condition ← sortie *Use Mobile Controls* de la fonction **Should Use Touch Controls**)
   - *True* → **Create Widget** (Class ← variable *Touch Controls Widget Class* ; Owning Player ← **Self**) → **Add to Player Screen**
   - *False* → (nœud de réacheminement) → **Add Mapping Context** (*Mapping Context* = **IMC_MouseLook**, *Priority* = 0)

<a id="notion-enhanced-input"></a>
> **Notion nouvelle — Enhanced Input : Input Action (IA) et Input Mapping Context (IMC)**
> Le système d'entrées d'Unreal 5 sépare **le quoi** et **le comment** :
> - une **Input Action** (`IA_`) décrit une intention de jeu (« essuyer », « se déplacer ») et le **type de valeur** qu'elle produit : *Digital (bool)* pour un bouton, *Axis2D* pour un déplacement ou la souris ;
> - un **Input Mapping Context** (`IMC_`) associe des **touches** à ces actions (E → IA_Essuyer). On peut avoir plusieurs IMC actifs (ici IMC_Default + IMC_MouseLook) et changer de contexte selon la situation (à pied, dans un menu…).
>
> Un IMC ne fait rien tant qu'il n'est pas **ajouté** au joueur avec **Add Mapping Context** sur le **Enhanced Input Local Player Subsystem** (fait ici par le PlayerController au BeginPlay). Pour voir un IMC : double-clic dessus → liste **Mappings** ; chaque ligne = une action, en dessous ses touches, et pour chaque touche ses *Triggers* et *Modifiers*.

<a id="notion-evenements-input"></a>
> **Notion nouvelle — Les sorties d'un nœud d'Input Action (Started, Triggered, Completed…)**
> Le nœud rouge d'une Input Action dans un graphe a plusieurs broches exec :
> - **Started** : une fois, au moment où l'on commence à appuyer ;
> - **Triggered** : quand l'action est « déclenchée » — sans *Trigger* particulier, c'est **à chaque image** tant que la touche est tenue (d'où son usage pour Move/Look) ;
> - **Ongoing** : pendant qu'un Trigger est en cours d'évaluation (ex. un appui long pas encore validé) ;
> - **Completed** : quand l'action se termine normalement (touche relâchée) ;
> - **Canceled** : quand un Trigger abandonne (ex. appui long relâché trop tôt).
> Pour une action « une fois par appui » (essuyer, saisir) on utilise **Started** ; pour « tant que c'est enfoncé » on combine **Started** (début) et **Completed/Canceled** (fin), comme pour Pull.

<a id="notion-modifiers"></a>
> **Notion nouvelle — Modifiers Swizzle et Negate**
> Une touche du clavier ne produit qu'**une** valeur (1 quand elle est enfoncée), rangée dans l'axe **X**. Pour qu'une seule action *Axis2D* `IA_Move` serve aux 4 directions, on transforme cette valeur avec des **Modifiers** : **Negate** inverse le signe (1 → −1), **Swizzle Input Axis Values (YXZ)** échange X et Y (la valeur passe dans l'axe Y). Résultat : D = (+1, 0) droite, Q = (−1, 0) gauche, Z = (0, +1) avant, S = (0, −1) arrière. **Dead Zone** ignore les petits mouvements d'un stick (sous 20 % ici).

**Pour le vérifier toi-même :**
1. **Content Browser** → `Content/Input` → double-clic sur `IMC_Default`.
2. Dans la liste **Mappings**, déplie chaque action : la touche est affichée dans le sélecteur de touche, déplie la touche pour voir *Triggers* et *Modifiers* (et l'ordre du Swizzle).
3. Pour qui active l'IMC : `Content/FirstPerson/Blueprints/BP_FirstPersonPlayerController` → **Event Graph**.

---

#### Point 6 — Réglages des objets de test

##### `BP_PreuveNettoyable` (`Content/Nettoyeur/Preuves/`, parent `PreuveNettoyable` C++)
| Composant / niveau | Réglage | Valeur |
|---|---|---|
| `MeshPreuve` (racine, hérité C++) | Static Mesh | `SM_Cube` (`Content/LevelPrototyping/Meshes`) |
| | Matériau (élément 0) | **`M_Tache`** (`Content/Nettoyeur/Materiaux`) |
| | Échelle (Scale) | 0.8 × 0.8 × **0.02** (une plaque fine posée au sol / au mur) |
| | Mobilité | Movable |
| | **Simulate Physics** | **décoché** |
| | Collision Presets | **BlockAllDynamic** (Query and Physics, type WorldDynamic) |
| `ProfilInteractions` (hérité C++) | Allowed Interactions | **[Wipe]** |
| Class Defaults | `Progression Par Essuyage` | **0.25** (→ 4 essuyages) |

Placée dans `Lvl_FirstPerson` (position −40, 220, 210) et dans `L_Maison01`.

##### `BP_ObjetPull_Test` (`Content/Nettoyeur/Preuves/`, parent `Actor`)
| Composant | Réglage | Valeur |
|---|---|---|
| `StaticMesh` (racine, ajouté) | Static Mesh | **`Cube2`** — un mesh importé rangé dans `Content/Nettoyeur/Materiaux/` (dossier inattendu pour un mesh) |
| | Matériau | `M_Mur` |
| | Échelle | 1 × 1 × 1 |
| | **Simulate Physics** | **coché** |
| | Collision Presets | **PhysicsActor** (type PhysicsBody) |
| | Masse | non surchargée (calculée depuis le volume) |
| `InteractionProfile` (ajouté) | Allowed Interactions | **[Pull]** |

Placé dans `Lvl_FirstPerson` (position −20, 270, 260). Il remplit toutes les conditions du `Pull()` C++ (profil Pull + racine qui simule la physique).

##### `BP_ObjetGrab_Test1` (`Content/Nettoyeur/Preuves/`, parent `Actor`)
| Composant | Réglage | Valeur |
|---|---|---|
| `GeometryCollection` (**racine**, ajouté) | Rest Collection | **aucune (vide)** |
| | Simulate Physics | coché |
| `StaticMesh` (enfant de GeometryCollection) | Static Mesh | `SM_Cube` |
| | Simulate Physics | coché |
| | Collision Presets | PhysicsActor |
| `InteractionProfile` (ajouté) | Allowed Interactions | **[Pull]** — **pas Grab** |

**Écarts importants sur l'objet Grab :**
1. Son profil n'autorise **que Pull**. Le C++ `Grab()` vérifie `CanPerform(Grab)` → **un clic gauche dessus ne fait rien**.
2. Sa **racine est un Geometry Collection vide** (aucune *Rest Collection*). Or le C++ saisit le **composant racine** (`GetFocusPrimitive()` = racine de l'acteur), pas le cube visible. Même avec le bon profil, la saisie viserait un composant sans géométrie. (L'asset `GC_ObjectGrab_Test1` existe dans `Content/Nettoyeur/Materiaux/Cube2_Cube2/` mais n'est pas branché.)
3. Le cube enfant simule lui aussi la physique : en jeu il se détache de la racine et tombe seul.
4. **Il n'est placé dans aucun niveau** (aucune référence dans `Lvl_FirstPerson` ni `L_Maison01`).
→ Conclusion : **aucun objet du projet ne permet aujourd'hui de tester Grab.** La ligne « Saisie Grab — test à confirmer » de `00_etat-projet.md` ne peut pas être confirmée en l'état ; il faudra (tâche future, hors T000) un objet avec profil `[Grab]` et une racine Static Mesh qui simule la physique.

<a id="notion-collision-physique"></a>
> **Notion nouvelle — Simulate Physics et Collision Presets**
> **Simulate Physics** (Détails du composant → catégorie *Physique*) : si coché, c'est le moteur physique qui déplace l'objet (gravité, chocs) ; il faut que ce soit le cas pour qu'un **Physics Handle** puisse le tirer ou le porter. Une seule règle d'or : seul un composant qui simule peut être saisi, et s'il a un parent, il s'en **détache** en jeu.
> **Collision Presets** (catégorie *Collision*) : un réglage tout prêt qui dit « qui suis-je » (*Object Type* : WorldStatic, WorldDynamic, PhysicsBody…) et « comment je réagis à chaque canal » (bloquer, chevaucher, ignorer). **BlockAllDynamic** = objet qui bloque tout mais ne bouge pas tout seul (la preuve) ; **PhysicsActor** = objet physique qui bloque tout (les objets à tirer/saisir). Les traces (Line Trace, Sweep) ne « voient » que ce qui bloque leur canal.

**Pour le vérifier toi-même :**
1. Ouvre le Blueprint (ex. `BP_ObjetGrab_Test1`) → panneau **Composants** : la racine est en haut de l'arbre.
2. Clique sur un composant → **Détails** : catégories **Static Mesh**, **Matériaux**, **Physique** (*Simulate Physics*), **Collision** (*Collision Presets*), **Interactions** (*Allowed Interactions* sur `InteractionProfile`).
3. Pour `Progression Par Essuyage` : bouton **Class Defaults** de `BP_PreuveNettoyable` → **Détails**.

<a id="notion-class-defaults"></a>
> **Notion nouvelle — Class Defaults**
> Le bouton **Class Defaults** (Valeurs par défaut de la classe) de la barre d'outils d'un Blueprint affiche, dans le panneau **Détails**, les **valeurs de départ des variables de l'acteur lui-même** (ex. `Progression Par Essuyage` = 0.25). C'est différent de cliquer sur un composant, qui montre les réglages **de ce composant**. Toute copie placée dans un niveau part de ces valeurs ; on peut ensuite les changer pour **une seule instance** en la sélectionnant dans le niveau (le Détails du niveau), ce qui ne touche pas au Blueprint.

---

#### Point 7 — Game Mode, map par défaut, rôle de `L_Maison01`

- **Project Settings → Maps & Modes** :
  - *Editor Startup Map* = `Lvl_FirstPerson`, *Game Default Map* = `Lvl_FirstPerson`
  - *Default GameMode* = `BP_FirstPersonGameMode`
- **`BP_FirstPersonGameMode`** (Class Defaults, catégorie *Classes*) :
  - *Default Pawn Class* = **`BP_FirstPersonCharacter`** (donc le personnage C++ via son Blueprint enfant)
  - *Player Controller Class* = `BP_FirstPersonPlayerController` (qui utilise `BP_FirstPersonCameraManager`)
  - *HUD Class* = `HUD` (aucun HUD personnalisé), *Game State* = `GameStateBase`, *Player State* = `PlayerState`
  - Son Event Graph est vide.
- **`Lvl_FirstPerson`** (`Content/FirstPerson/`, carte du modèle + objets Nettoyeur) : **World Settings → Game Mode → GameMode Override** = `BP_FirstPersonGameMode` (identique au global). Acteurs Nettoyeur placés : `BP_PreuveNettoyable`, `BP_ObjetPull_Test`, un `PlayerStart` (0, 0, 302). Cette carte est en mode *One File Per Actor* (voir notion).
- **`L_Maison01`** (`Content/Nettoyeur/Maps/`) : niveau de test « première maison », créé le 2026-10-22 (commit « Nouveau niveaux de test de la première maison »), retouché le 2026-10-23, **plus modifié depuis**. Contenu : ~80 `StaticMeshActor` (cubes/cylindres moteur avec `M_Mur` et `M_Sol` : murs et sols), 1 `BP_PreuveNettoyable` (192, 113, 90), 1 `PlayerStart` (−530, −3280, 262), éclairage (DirectionalLight, SkyLight, SkyAtmosphere). *GameMode Override* = `BP_FirstPersonGameMode`. **Il n'est référencé par aucun réglage** (ni map par défaut ni startup) : on ne l'ouvre qu'à la main. Il ne contient **ni** objet Pull **ni** objet Grab.
  - Remarque : le `PlayerStart` est à plus de 3 km (en Y) de la preuve ; à vérifier en jeu que le joueur apparaît bien dans la maison.

<a id="notion-game-mode"></a>
> **Notion nouvelle — Game Mode (et Default Pawn, PlayerController)**
> Le **Game Mode** est l'acteur qui fixe les **règles** d'une partie et surtout **quelles classes utiliser** : *Default Pawn Class* (le corps que le joueur contrôle, ici `BP_FirstPersonCharacter`), *Player Controller Class* (le « cerveau » du joueur, qui reçoit les entrées et active les IMC), *HUD*, *Game State*… Au lancement, Unreal crée le Player Controller, puis fait apparaître le Pawn sur un **PlayerStart** et le donne au contrôleur. Changer de personnage ou de règles = changer de Game Mode, pas modifier la carte.

<a id="notion-maps-modes-world-settings"></a>
> **Notion nouvelle — Maps & Modes et World Settings (GameMode Override)**
> Deux endroits décident du Game Mode et de la carte :
> - **Project Settings → Projet → Maps & Modes** : réglages **globaux** du projet — la carte ouverte au démarrage de l'éditeur, la carte chargée au lancement du jeu, et le Game Mode **par défaut**.
> - **World Settings** d'un niveau (menu **Fenêtre (Window) → Paramètres du monde (World Settings)**, ou bouton de la barre d'outils) → catégorie **Game Mode** → **GameMode Override** : remplace le Game Mode global **pour ce niveau seulement**.
> Ordre de priorité : l'override du niveau gagne sur le réglage global. Ici les deux pointent vers le même `BP_FirstPersonGameMode`, donc pas d'ambiguïté.

<a id="notion-ofpa"></a>
> **Notion nouvelle — One File Per Actor (`__ExternalActors__`)**
> `Lvl_FirstPerson` utilise le mode **One File Per Actor** (activé par défaut sur les cartes World Partition) : chaque acteur placé est sauvegardé dans **son propre petit fichier** sous `Content/__ExternalActors__/FirstPerson/Lvl_FirstPerson/…` au lieu d'être dans le `.umap`. Avantage : deux personnes peuvent modifier des acteurs différents sans conflit Git. Conséquence : ajouter ou supprimer un acteur dans ce niveau crée/supprime un fichier `.uasset` dans ce dossier, qu'il faut commiter avec le reste. `L_Maison01`, lui, garde tout dans son `.umap`.

**Pour le vérifier toi-même :**
1. **Modifier → Paramètres du projet** → colonne de gauche **Projet → Cartes et modes** : les trois valeurs.
2. Ouvre `BP_FirstPersonGameMode` → **Class Defaults** → catégorie **Classes**.
3. Ouvre `L_Maison01` (double-clic dans `Content/Nettoyeur/Maps` — *n'enregistre pas* si l'éditeur le propose pour `Lvl_FirstPerson` sans savoir pourquoi) → **Fenêtre → Paramètres du monde** → *GameMode Override* ; panneau **Organiseur (Outliner)** pour la liste des acteurs.

---

#### Point 8 — Rôle de `BP_FirstPersonCharacter_Backup`

- Parent : **`Character`** (pas `PersonnageNettoyeur`).
- Composants : CapsuleComponent, ArrowComponent, Mesh, FirstPersonMesh, FirstPersonCamera, CharacterMovement, **ComposantNettoyage** ; **pas** d'`InteractionComponent` ni de `PhysicsHandle`.
- Event Graph : identique à celui du personnage actuel (Move, Look, MouseLook, Jump, Essuyer → Tenter Nettoyage, création de `WBP_Interaction`, `RecevoirProgressionNettoyage`, mission terminée, même bloc Line Trace mort) **sans les blocs `IA_Pull` et `IA_Grab`**.
- Ajouté dans le commit `0b65c4e` (2026-11-10, « Ajout de l'action Pull et début du Grab »), le même commit que le Grab/Pull.
- **Référencé par rien** (aucun Game Mode, aucun niveau). Seul lien sortant : il dépend de `WBP_Interaction` et des IA.

→ **Interprétation :** c'est une **copie de sauvegarde du personnage faite juste avant le reparentage** vers `APersonnageNettoyeur` et l'ajout de Grab/Pull. Il ne sert pas en jeu. Il est probablement **inutile maintenant** que le personnage reparenté fonctionne, mais **je ne le supprime pas** (hors périmètre, décision à prendre par l'orchestrateur/le développeur). Inconvénient à connaître : il apparaît dans les recherches (« Find References » de `WBP_Interaction`, de `IA_Essuyer`…) et devra être mis à jour si on renomme ces assets.

**Pour le vérifier toi-même :** clic droit sur `BP_FirstPersonCharacter_Backup` dans le **Content Browser** → **Visualiseur de références (Reference Viewer)** : rien à gauche (aucun asset ne l'utilise).

---

### 5. Compilation et tests
- Compilation : aucune (pas de C++ ni d'asset modifié).
- Test en jeu : **non réalisé** (partie en lecture seule, pas de Play lancé). Les comportements déduits ci-dessus (Grab impossible sur `BP_ObjetGrab_Test1`, Grab non relâché par le clic) sont **déduits de la lecture** du graphe et du C++, **pas observés en jeu**. Procédure pour les confirmer (développeur ou QA) :
  1. Ouvre `Lvl_FirstPerson`, **Jouer (Play)**.
  2. Vise `BP_ObjetPull_Test`, maintiens **R** : il doit suivre au sol ; relâche **R** : il est lâché. *Attendu : fonctionne.*
  3. Place un `BP_ObjetGrab_Test1` dans le niveau (sans sauvegarder), vise-le, **clic gauche**. *Attendu d'après la lecture : rien ne se passe* (profil sans Grab).
  4. Vise `BP_PreuveNettoyable`, appuie 4 fois sur **E** : la barre monte par quart, des messages de debug bleus s'affichent, puis « MISSION TERMINÉE ! » et la preuve disparaît.

### 6. Git
- Branche : `tache/T000-verification-existant` (déjà active, non changée).
- Commits : aucun (l'orchestrateur s'en charge).
- **À signaler :** un fichier **non suivi** est apparu pendant la session, hors de mon action : `Content/__ExternalActors__/FirstPerson/Lvl_FirstPerson/0/I6/M8RK6YKWGV6E3Q8C5UHZX3.uasset` (créé le 2026-11-11 à 23:41). D'après son contenu, c'est un **PointLight** ajouté à `Lvl_FirstPerson` dans l'éditeur. Il n'apparaît pas dans la liste des acteurs actuellement chargés (peut-être supprimé depuis sans sauvegarde). Je ne l'ai ni créé ni touché : au développeur de dire s'il doit être commité ou supprimé.

### 7. Résultat et points ouverts

**Écarts avec `doc/00_etat-projet.md` :**
1. ✅ Parent de `BP_FirstPersonCharacter` = `PersonnageNettoyeur` (la ligne ❓ peut passer à ✅).
2. ⚠️ **Grab non testable** : aucun objet n'a le profil `[Grab]` ; `BP_ObjetGrab_Test1` a le profil `[Pull]`, une racine Geometry Collection vide et n'est placé dans aucun niveau.
3. ⚠️ **Relâcher le clic gauche ne lâche pas l'objet** : le *Release Grab* du bloc Grab n'est pas relié. Seul relâcher R lâche (Release Grab de Pull).
4. ⚠️ La « mission terminée » est une logique **Blueprint** du personnage, déclenchée dès la **première** preuve nettoyée à 100 %.
5. ℹ️ `L_Maison01` est un niveau de test de maison, non utilisé par les réglages ; il ne contient que la preuve (pas de Pull/Grab).
6. ℹ️ `BP_FirstPersonCharacter_Backup` = sauvegarde d'avant reparentage, non utilisée — à garder ou supprimer sur décision.
7. ℹ️ La dette n° 4 (`TenterNettoyage` en double) : le Blueprint appelle bien la version du **composant** ; la version du personnage n'est appelée par aucun Blueprint.
8. ℹ️ Le prompt (`AfficherInteraction`) affiche un texte figé « Appuie sur E pour nettoyer » et montre la barre en même temps.

**Petites incohérences de rangement (à traiter plus tard, pas en T000) :** variable widget nommée `NewVar` ; bloc Line Trace + Print String mort dans l'Event Graph ; Print String de debug dans `MettreAJourProgression` ; mesh `Cube2` rangé dans `Materiaux/` ; `ComposantNettoyage` ajouté en Blueprint alors que les autres composants Nettoyeur sont créés en C++.

**Non vérifiable par script (à relever à la main si besoin) :**
- Le **socket** d'attache de `FirstPersonCamera` sur `FirstPersonMesh` (Détails → *Parent Socket*).
- La position exacte des nœuds à l'écran (sans importance fonctionnelle) ; les connexions, elles, ont été lues de façon fiable.
- Le comportement en jeu (aucun Play lancé) — voir la procédure du §5.
