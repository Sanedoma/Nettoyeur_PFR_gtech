# Journal — qa

> Entrées selon `doc/CONVENTION.md`, la plus récente en haut.

---

## QA T000 — Vérifier la config éditeur et documenter l'existant — 2026-11-12
**Verdict :** ✅ Validé QA pour ce qui est vérifiable sans éditeur. Les tests éditeur/jeu (critères 4 « Traîner » et 5 non-régression) **attendent la validation humaine** : je ne les ai pas exécutés.

**Branche :** `tache/T000-verification-existant` (pas de changement de branche, aucun commit).

### Compilation
- Éditeur fermé : `tasklist /FI "IMAGENAME eq UnrealEditor.exe"` → « aucune tâche en service ne correspond ».
- Méthode : Build.bat (`CLAUDE.md` §6), lancé depuis PowerShell : `NettoyeurEditor Win64 Development -Project=... -WaitMutex`.
  - Remarque : lancé depuis Git Bash, la commande échoue (« 'C:\Program' n'est pas reconnu… » : cmd.exe coupe le chemin à l'espace). Depuis PowerShell (`& "C:/Program Files/.../Build.bat" ...`) elle fonctionne.
- Résultat : **`Result: Succeeded`**, 16,5 s, code de sortie 0. UHT exécuté avec `-WarningsAsErrors` (donc 0 avertissement UHT). Étapes [1/8]…[8/8] : recompilation de `InteractionTypes.cpp`, `InteractionProfileComponent.cpp`, `PreuveNettoyable.cpp`, `InteractionComponent.cpp`, `Module.Nettoyeur.gen.cpp` (tous incluent `InteractionTypes.h`), liens `.lib` / `.dll`.
- **0 erreur, 0 ligne `warning`** dans la sortie.

### Périmètre (diff)
- `git diff --numstat Source/` → `1 1 Source/Nettoyeur/InteractionTypes.h` : **une seule ligne** modifiée (ligne 14, `Drag UMETA(DisplayName = "Traîner")`).
- Octets ligne 14 : avant (HEAD) `54 72 61 ee 6e 65 72` → après `54 72 61 c3 ae 6e 65 72` : **`EE` → `C3 AE`** ✔.
- `file` : « C source, Unicode text, UTF-8 text, with CRLF line terminators » ✔ ; 3 premiers octets `23 70 72` (`#pr`) → **pas de BOM** ✔.
- Docs modifiées : `doc/agents/editeur-contenu.md`, `doc/agents/gameplay-cpp.md`, `doc/notions.md`, la fiche — conformes au périmètre.
- Rien sous `Saved/`, `Intermediate/`, `Binaries/` dans le suivi Git.
- **⚠️ Hors périmètre, à trancher par l'orchestrateur (ne bloque pas T000 mais ne doit pas partir dans le commit T000 sans décision) :**
  1. `Nettoyeur.uproject` modifié : `EngineAssociation` `{527292F9-…}` → `"5.7"` et ajout du plugin `EngineWire` (Win64/Mac/Linux). Aucun des deux agents ne documente ce changement (ni avant → après, ni pourquoi), contrairement à `CONVENTION.md` §1 (ligne « Configuration … `.uproject` … plugins »). `00_etat-projet.md` annonce déjà EngineWire « ✅ Actif » : vraisemblablement fait par le développeur hors tâche. À commiter à part (ou documenter) sur décision.
  2. Fichier non suivi `Content/__ExternalActors__/FirstPerson/Lvl_FirstPerson/0/I6/M8RK6YKWGV6E3Q8C5UHZX3.uasset` (un PointLight d'après `editeur-contenu`) — déjà signalé par `editeur-contenu` §6. Au développeur de dire s'il le garde.

### Critères d'acceptation
- [x] `editeur-contenu.md` décrit les 8 points de la partie A, graphes nœud par nœud — points 1 à 8 tous présents ; graphes Essuyer / Pull / Grab / BeginPlay-widget / AfficherInteraction / RecevoirProgressionNettoyage / PlayerController décrits nœud par nœud avec chaque connexion exec et données ; chaque point a un « Pour le vérifier toi-même ».
- [x] `gameplay-cpp.md` explique chaque classe et leurs liens — §2.2 InteractionTypes, §2.3 InteractionProfileComponent, §2.4 PreuveNettoyable, §2.5 PersonnageNettoyeur, §2.6 InteractionComponent, §2.7 ComposantNettoyage ; collaboration : §2.0 (arbre) et §2.8 (chronologie + « qui connaît qui »).
- [x] `notions.md` contient les notions de B.2, chacune avec un lien — présentes : UCLASS/UPROPERTY/UFUNCTION, GENERATED_BODY, TObjectPtr, CreateDefaultSubobject, Actor Component, Tick, Line Trace vs Sweep, canaux de collision, Physics Handle, VInterpTo, UENUM, delegate dynamique multicast, ProcessEvent (+ 13 autres en C++, 14 notions éditeur). **40 liens d'ancre vérifiés par script : 40/40 résolus.**
- [ ] `InteractionTypes.h` UTF-8 ✔, compile ✔ — **« Traîner » dans l'éditeur : en attente de validation humaine** (procédure ci-dessous, étape A).
- [ ] Non-régression nettoyage / Pull — **en attente de validation humaine** (étapes B et C). Grab : **non testable en l'état** (aucun objet du projet n'a le profil `[Grab]`, constat de la partie A point 6) — à accepter comme tel ou à reporter en T001.
- [ ] Mise à jour de `00_etat-projet.md` par l'orchestrateur — hors périmètre QA, non fait à ce jour.

### Non-régression
Non exécutée par moi (pas d'éditeur lancé). Raisonnement : le seul changement de code est un octet dans une chaîne `UMETA(DisplayName)` ; aucune logique ni aucun nom d'enum ne change, donc aucun effet attendu sur le nettoyage ni sur Pull/Grab. **Cela reste une déduction, pas une observation** — à confirmer avec la procédure.

### Cas limites testés
Aucun en jeu (tâche de documentation, pas de nouveau comportement). Les cas limites du code existant sont **relus** : chaque `Pull()`/`Grab()` sort proprement sans cible, sans profil, sans physique (`InteractionComponent.cpp:94-110`, `:128-144`) ; `TenterNettoyage()` appelle `GetWorld()` sans test (`ComposantNettoyage.cpp:120`) — déjà signalé par `gameplay-cpp` §2.9, sans conséquence en jeu.

### Relecture code
- Diff : 1 octet → 2 octets, rien d'autre. Pas de Tick/DrawDebug ajoutés, pas de doublon.
- Contrôle par échantillon **doc ↔ code réel** (lu dans `Source/Nettoyeur/`), tout concorde :
  - `ComposantNettoyage.cpp` : trace 300 en dur (l. 57 et 114), Line Trace Visibility, `Cast<APreuveNettoyable>` (l. 74, 130), `UE_LOG` l. 140, `Broadcast` l. 142, `DrawDebugLine` durée 0 dans le Tick (l. 84-93) et 2 s dans `TenterNettoyage`, `FindFunction`+`ProcessEvent` avec struct locale (l. 164-187), `GetWorld()` non testé l. 120.
  - `InteractionComponent.cpp` : Sweep sphère 10 cm (l. 73-80), `GetFocusPrimitive` = racine (l. 85-90), `GetAimDirection` jamais appelé (l. 34-49), `LineTraceMulti` +500/−1500 et boucle sur le premier bloquant non simulé (l. 237-255), double lissage Z (l. 265-268), `Bounds.BoxExtent.Z` (l. 250), Grab = cible à `HoldDistance` d'emblée.
  - `InteractionComponent.h` : `TraceDistance 300`, `HoldDistance 150`, `HoldInterpSpeed 6`, catégorie `Interaction|Pull`.
  - `PreuveNettoyable.cpp` : Tick désactivé, `MeshPreuve` racine, profil `[Wipe]`, `FMath::Clamp`, cachée + collision coupée à 1.0.
  - `PersonnageNettoyeur.cpp/.h` : `SetPhysicsHandle` dans le constructeur, `CanJumpInternal_Implementation`, `TenterNettoyage()` vide et `BlueprintCallable` catégorie « Nettoyage ».
  - `InteractionProfileComponent.cpp` : Tick désactivé, `CanPerform` = `Contains`.
- Les numéros de ligne cités dans la table de dette (§2.9) sont exacts sur les points vérifiés.

### Relecture documentation
- Les deux entrées suivent le modèle `CONVENTION.md` §5 (sections 1 à 7 présentes, « Aucune » quand vide).
- Inspection faite par Python à distance (`editeur-contenu`) : **retraduite en équivalent manuel** à chaque point ✔. Conversion d'encodage faite avec `iconv` : équivalents manuels VS 2022 / Rider / Notepad++ donnés ✔.
- Noms d'interface en français avec l'anglais entre parenthèses ✔.
- Ancres de titres croisées (`editeur-contenu.md#point-6--…`, `#3c--grab-…`, `#4d--…`, `#t000-partie-a--…`) : vérifiées à la main selon la règle de génération d'ancres de GitHub/VS Code → correctes. Les 40 ancres `<a id>` vérifiées par script.
- Liens doc officielle Epic, contrôle par échantillon : *Traces with Raycasts* → page réelle ✔ ; *FName in Unreal Engine* → page réelle ✔ ; *Property Specifiers* → contenu non lisible automatiquement (page JavaScript) : **à cliquer par le développeur**.
- Remarques mineures (non bloquantes, pour `gameplay-cpp`) :
  1. Quelques notions n'ont pas de lien vers la doc officielle dans leur explication (Déclaration anticipée, `virtual`/`override`/`Super`, `BeginPlay`, Point de vue du joueur) ; `gameplay-cpp` §7 affirme pourtant « chacune avec … vers la doc officielle ». Le critère de la fiche (lien dans `notions.md`) reste rempli.
  2. `gameplay-cpp` §5 indique « Compilation : non réalisée » : c'est vrai au moment de l'écriture ; la compilation est désormais faite par QA (ce rapport).
  3. La fiche dit « 14 notions de B.2 » alors que la liste B.2 en compte 13 (ou 15 si on sépare UCLASS/UPROPERTY/UFUNCTION). Sans effet : toutes sont présentes.

### Procédure de validation pour le développeur
Prérequis : la compilation vient d'être faite (éditeur fermé). Ouvre le projet normalement (double-clic sur `Nettoyeur.uproject`). **N'enregistre rien** pendant ces tests.

**A. Affichage de « Traîner » (critère 4)**
1. **Content Browser** → dossier `Content/Nettoyeur/Preuves` → double-clic sur `BP_ObjetPull_Test`.
2. Panneau **Composants (Components)** en haut à gauche → clique sur `InteractionProfile`.
3. Panneau **Détails (Details)** → catégorie **Interactions** → *Allowed Interactions* → élément **0** (valeur actuelle *Tirer*) → clique sur la liste déroulante.
4. *Attendu :* une ligne **« Traîner »** (avec un î correct), et non « Tra�ner ».
5. **Ne change pas** la valeur : appuie sur Échap, ferme le Blueprint ; si on te demande d'enregistrer, réponds **Ne pas enregistrer**.

**B. Non-régression du nettoyage (critère 5)**
1. Ouvre `Content/FirstPerson/Lvl_FirstPerson` (carte par défaut) → **Jouer (Play)**.
2. Vise la plaque `BP_PreuveNettoyable` (proche du point d'apparition) : le texte « Appuie sur E pour nettoyer » et la barre apparaissent.
3. Appuie **4 fois sur E** : la barre monte par quarts (0,25 / 0,5 / 0,75 / 1), des messages bleus de debug s'affichent, puis « MISSION TERMINÉE ! » et **la preuve disparaît**.
4. (Facultatif) Fenêtre **Output Log** : 4 lignes jaunes `Progression nettoyage : 0.250000` … `1.000000`.

**C. Non-régression du Pull (critère 5)**
1. Toujours en jeu, vise le cube `BP_ObjetPull_Test`, **maintiens R** : il glisse au sol devant toi quand tu te déplaces.
2. Pendant que tu le tiens, Espace ne doit **pas** te faire sauter.
3. **Relâche R** : l'objet est lâché et reste au sol.

**D. Grab** : non testable (aucun objet `[Grab]` dans le projet). Si tu veux le constater : clic gauche sur `BP_ObjetPull_Test` → *attendu : rien ne se passe* (profil `[Pull]` seulement).

**E. Liens doc** : ouvre dans ton navigateur 2-3 liens « Doc : » de `doc/agents/gameplay-cpp.md` (ex. *Property Specifiers*, *Physics Handle*, *Dynamic Delegates*) et vérifie qu'ils mènent à la bonne page.

Renvoie à l'orchestrateur : A ok/ko, B ok/ko, C ok/ko (et ce que tu as vu si ko). Ensuite, l'orchestrateur décide du sort de `Nettoyeur.uproject` et du `.uasset` non suivi avant le commit `T000: ...`.
