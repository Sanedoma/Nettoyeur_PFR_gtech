# Journal — qa

> Entrées selon `doc/CONVENTION.md`, la plus récente en haut.

---

## QA T001 — Unifier la visée et faire passer le nettoyage par le profil d'interaction — 2026-11-13
**Verdict :** ✅ Validé QA pour tout ce qui est vérifiable sans lancer le jeu (compilation, code, périmètre, doc). Les tests en jeu (prompt, 4 essuyages, objet sans Wipe, debug, Pull/Grab) **n'ont pas été exécutés** : l'éditeur était fermé et les outils MCP Unreal n'étaient pas utilisables. Ils restent à faire par le développeur (procédure en bas).

**Branche :** `tache/T001-unifier-visee` (pas de changement de branche, aucun commit).

### Compilation
- Éditeur fermé : aucun processus `Unreal*` / `LiveCoding*` en cours (`Get-Process`).
- Méthode : Build.bat (`CLAUDE.md` §6) depuis PowerShell : `NettoyeurEditor Win64 Development -Project=... -WaitMutex`.
- 1er lancement : `Target is up to date` / `Result: Succeeded` (les binaires avaient déjà été produits par `gameplay-cpp`). Cela ne prouve rien sur les avertissements, donc :
- 2e lancement après avoir seulement mis à jour la date de modification des `.cpp` de `Source/Nettoyeur/` (contenu inchangé) pour forcer la recompilation : `[1/10]` à `[7/10] Compile` (les 7 `.cpp` du module), `[8/10]`–`[9/10] Link` `.lib` / `.dll`, `[10/10] WriteMetadata`. **`Result: Succeeded`, code de sortie 0, 0 erreur, 0 ligne `warning`.**

### Périmètre
- `git status` : modifiés `Source/Nettoyeur/{InteractionComponent,ComposantNettoyage,PersonnageNettoyeur}.{h,cpp}` (= les 6 fichiers autorisés), `doc/agents/gameplay-cpp.md`, `doc/notions.md`, `doc/taches/T001-unifier-visee.md`, `doc/00_etat-projet.md`.
- `doc/00_etat-projet.md` : seulement le passage de T001 en « En cours » — fichier de l'orchestrateur, non mentionné dans le journal de `gameplay-cpp` ; à confirmer par l'orchestrateur que c'est bien lui.
- Non suivi : `Content/__ExternalActors__/FirstPerson/Lvl_FirstPerson/0/I6/M8RK6YKWGV6E3Q8C5UHZX3.uasset`, daté du 2026-11-11 : **antérieur à T001**, déjà signalé en QA T000. Ne doit pas partir dans le commit T001 sans décision.
- Rien sous `Saved/`, `Intermediate/`, `Binaries/` dans le suivi Git. Aucun `.uasset` / `.umap` modifié, aucune config modifiée.
- Encodage : les 6 sources en UTF-8 (ou ASCII) sans BOM, CRLF (`file`, 3 premiers octets `#in`).

### Critères d'acceptation
- [x] Un seul trace/sweep de visée par frame dans tout le projet — `grep -rnE "LineTrace|Sweep|Overlap" Source/` : seulement `SweepSingleByChannel` (`InteractionComponent.cpp:73`, `UpdateFocus`, appelé par `TickComponent` et une fois au `BeginPlay`) et `LineTraceMultiByChannel` (`InteractionComponent.cpp:241`, `UpdateHold`) qui cherche le sol sous un objet tiré, uniquement pendant un Pull : ce n'est pas une visée. `UComposantNettoyage` : plus de trace, `bCanEverTick = false`, plus de `TickComponent`.
- [ ] Viser une `BP_PreuveNettoyable` affiche le prompt ; ne plus la viser le masque — **test en jeu à faire par le développeur.** Lecture du code : `SetCurrentFocus` diffuse `OnFocusChanged` seulement au changement → `GererChangementCible` → `MettreAJourTexteInteraction(CanFocusPerform(Wipe))`. État initial poussé par `DefinirWidgetInteraction`. Profil : `AllowedInteractions` n'apparaît dans aucun asset sauf `BP_ObjetPull_Test` et `BP_ObjetGrab_Test1` (recherche de texte dans `Content/`) → `BP_PreuveNettoyable` et son instance dans le niveau héritent du constructeur C++ = [Wipe]. Non vérifié dans l'éditeur (MCP indisponible, éditeur fermé).
- [ ] Essuyer 4 fois → 0.25 → 0.5 → 0.75 → 1.0 et disparition — **test en jeu à faire.** Lecture : `APreuveNettoyable::Essuyer()` inchangé (+`ProgressionParEssuyage` = 0.25, plafonné à 1, à 1.0 : caché + collision coupée) ; `OnProgressionNettoyageChanged.Broadcast(Progression)` toujours appelé à chaque essuyage réussi, **sans condition de debug**.
- [ ] Un objet sans `Wipe` ne peut pas être essuyé et n'affiche pas le prompt — **test en jeu à faire.** Lecture : `TenterNettoyage` sort immédiatement si `!PeutNettoyerCible()` ; le prompt reçoit `false`.
- [ ] Pas de ligne de debug par défaut ; `bAfficherDebug = true` la réaffiche — **test en jeu à faire.** Lecture : aucun `DrawDebug*` hors du bloc `if (bAfficherDebug)` de `UpdateFocus` ; `bAfficherDebug = false` par défaut sur les deux composants.
- [ ] Grab et Pull inchangés — **test en jeu à faire (Pull ; Grab toujours non testable, dette B2).** Lecture du diff : seules les vérifications de profil sont factorisées dans `CanFocusPerform` (même condition qu'avant : profil présent et `CanPerform`), et le calcul du point de vue dans `GetViewPoint` (même valeur de secours qu'avant : position/rotation de l'acteur si pas de PlayerController). Ordre des tests, saisie, interpolation, recherche du sol, lâcher au bord du vide, refus du saut : identiques.
- [x] Compilation sans nouvel avertissement — voir Compilation : 0 erreur, 0 avertissement après recompilation forcée des 7 `.cpp`.
- [x] Journal conforme à `doc/CONVENTION.md` — voir Relecture documentation.

### Non-régression
Pas testée en jeu. Relecture du code uniquement :
- Nettoyage : même décision de fond (seule `APreuveNettoyable` sait s'essuyer), désormais conditionnée au profil [Wipe], que la preuve a par défaut. Même portée (300 cm, `TraceDistance`).
- **Différence voulue et documentée :** le prompt et l'essuyage utilisent le sweep sphère de 10 cm au lieu d'un rayon fin → au bord d'une preuve, le prompt peut apparaître un peu plus tôt qu'avant.
- **Différence mineure :** la ligne « Progression nettoyage : … » n'est plus écrite par défaut dans l'Output Log (avant : toujours, en jaune `Warning`) ; elle passe en `Log` derrière `ComposantNettoyage → bAfficherDebug`. Documenté (journal §2.3 d).
- Blueprints : `APersonnageNettoyeur::TenterNettoyage` supprimée, `WidgetInteraction` passé en `BlueprintReadOnly`. Recherche de texte dans `BP_FirstPersonCharacter.uasset` : `TenterNettoyage` apparaît une fois (T000 : c'est celui de `ComposantNettoyage`), `WidgetInteraction` n'apparaît que dans `DefinirWidgetInteraction` et `WidgetInteractionComponent` → aucun nœud *Set Widget Interaction*. Forte présomption que les Blueprints compilent, **à confirmer en ouvrant l'éditeur** (point 9 de la fiche).

### Cas limites testés (par lecture du code, pas en jeu)
- Pas de cible : `CanFocusPerform` renvoie `false` si `CurrentFocus` est nul → E sans effet, prompt masqué.
- Cible hors portée : pas de touche du sweep → `SetCurrentFocus(nullptr)` → prompt masqué.
- Cible incompatible (pas de profil, ou profil sans Wipe) : `false` → E sans effet, pas de prompt.
- Cible avec Wipe mais pas une `APreuveNettoyable` : prompt affiché mais E sans effet (le `Cast` échoue proprement). Aucun cas dans le projet aujourd'hui ; signalé par l'agent (§7), à régler en T003/T004.
- Preuve finie : cachée + collision coupée → l'image suivante le sweep ne la touche plus → `OnFocusChanged` → prompt masqué. E pressé une seconde fois **dans la même image** (pratiquement impossible) : `Essuyer()` ne fait rien (`EstNettoyee`), mais la progression 1.0 serait rediffusée une fois. Sans conséquence aujourd'hui, à garder en tête quand le score écoutera ce delegate.
- Personnage sans `InteractionComponent` : message `Warning` unique au `BeginPlay`, tout le reste testé contre `nullptr` → pas de plantage.
- Personnage pas encore possédé : `GetViewPoint` renvoie `false` → rien de visé (comme avant).
- Ordre d'initialisation : les `BeginPlay` des composants passent avant l'Event BeginPlay du Blueprint, donc l'abonnement existe avant `DefinirWidgetInteraction` ; un changement de cible manqué avant l'abonnement est rattrapé par l'état poussé dans `DefinirWidgetInteraction`.
- Désabonnement : `RemoveDynamic` dans `EndPlay`, mêmes arguments que `AddDynamic`, protégé par `if (ComposantInteraction)` ; abonnement unique (BeginPlay seulement). `GererChangementCible` est bien une `UFUNCTION()` avec la signature `AActor*` du delegate.
- `ProcessEvent` : plus sûr qu'avant (vérifie `NumParms == 1`, un `FBoolProperty` « Afficher » avec `CPF_Parm`, bloc de `ParmsSize` octets remis à zéro). `WidgetInteraction` nul → sortie immédiate. Fonction absente → sortie silencieuse (comme avant).

### Relecture code
- Lisible, commenté en français, identifiants sans accent, catégories `UPROPERTY` cohérentes, pas d'abstraction inutile (`CanFocusPerform` a 3 utilisateurs réels).
- Pas de Tick inutile (le nettoyage n'en a plus), pas de `DrawDebug` hors debug, `GetAimDirection` supprimé (aucun appel restant), plus de doublon de calcul du point de vue.
- Pointeurs vérifiés partout (`GetOwner`, `World`, `ComposantInteraction`, `CurrentFocus`, `WidgetInteraction`, `Fonction`, `Preuve`).
- Remarque (non bloquante) : deux booléens `bAfficherDebug` (un sur `InteractionComponent` pour le trait de visée, un sur `ComposantNettoyage` pour le journal de progression). La fiche n'en demandait qu'un, mais chacun est rangé là où le debug est produit et la doc explique les deux. Acceptable.
- Remarque (non bloquante, hors périmètre) : `PersonnageNettoyeur.h` inclut encore `InteractionComponent.h` / `PhysicsHandleComponent.h` au lieu de déclarations anticipées — signalé par l'agent.
- Conforme à `doc/decisions.md` d'après la fiche (logique en C++, pas de modification de Blueprint par l'agent ; le nettoyage du graphe B1 est décrit pour le développeur).

### Relecture documentation
- Entrée T001 de `doc/agents/gameplay-cpp.md` : les 7 sections du modèle sont présentes (Configuration = « Aucune »). Fichiers modifiés listés avec le pourquoi ; code expliqué fichier par fichier avec extraits ; « pourquoi supprimer le double trace » (§2.1), delegate (`OnFocusChanged`, lien vers la notion existante) et binding C++ (`AddDynamic` / `RemoveDynamic` / `EndPlay`) expliqués, comme demandé par la fiche.
- Étapes éditeur écrites comme un humain (B1 nœud par nœud dans `BP_FirstPersonCharacter` et `WBP_Interaction`, réglages de debug, vérification du profil). La vérification du profil faite par recherche de texte est clairement présentée comme telle, avec l'équivalent manuel.
- Notions nouvelles (source unique / événement vs sondage, paramètre de sortie par référence, `Transient`, `AddDynamic`/`EndPlay`, signature d'une `UFunction`) : expliquées dans le journal avec une ancre **et** ajoutées à `doc/notions.md`. Les liens vers des notions existantes (`delegate-dynamique-multicast`, `tobjectptr`, `declaration-anticipee`, `ue-log-drawdebug`, `tick`) pointent vers des ancres qui existent.
- Petite inexactitude : le §1 dit « 6 notions ajoutées », `doc/notions.md` en reçoit **5**. Sans impact.
- Le journal annonce honnêtement « non testé en jeu ».

### Procédure de validation pour le développeur
Préparation : ouvre le projet dans Unreal (le C++ a été recompilé, pas besoin de recompiler).
1. **Blueprints** : ouvre `Content/FirstPerson/Blueprints/BP_FirstPersonCharacter` → bouton **Compiler** : aucune erreur (pas de nœud « Tenter Nettoyage » cassé côté personnage, pas de *Set Widget Interaction* en erreur). Fais de même avec `Content/Nettoyeur/WBP_Interaction`. Ferme sans enregistrer si rien n'a changé.
2. **Profil** : ouvre `Content/Nettoyeur/Preuves/BP_PreuveNettoyable` → panneau **Composants** → `ProfilInteractions` → **Détails** → *Allowed Interactions* contient **Essuyer**.
3. Ouvre le niveau `Lvl_FirstPerson`, **Fenêtre (Window) → Journal de sortie (Output Log)** ouvert, puis **Play**.
4. **Output Log au lancement** : aucun message « ComposantNettoyage : aucun InteractionComponent… » ni « …AfficherInteraction doit avoir un seul parametre… ».
5. **Pas de debug par défaut** : aucun trait vert/rouge à l'écran en bougeant la souris.
6. **Prompt** : vise la preuve (à moins de 3 m) → « Appuie sur E pour nettoyer » + barre apparaissent. Détourne le regard → ils disparaissent. Recommence 2–3 fois. Éloigne-toi au-delà de 3 m en visant : le prompt disparaît.
7. **Nettoyage** : vise la preuve, appuie 4 fois sur **E** → la barre passe à 25 %, 50 %, 75 %, 100 % ; au 4e appui la preuve disparaît, le prompt se masque, « MISSION TERMINÉE ! » s'affiche (comme avant). Un 5e appui sur E dans le vide ne fait rien.
8. **Objet sans Wipe** : relance Play, vise `BP_ObjetPull_Test` → **pas** de prompt ; appuie sur E → rien ne se passe. Test complémentaire (sans enregistrer) : arrête Play, sélectionne la preuve dans l'**Organiseur (Outliner)** → composant `ProfilInteractions` → *Allowed Interactions* → supprime l'élément **Essuyer** → Play : plus de prompt, E sans effet. Arrête Play puis **Ctrl+Z** (ou flèche jaune « réinitialiser » à côté de la liste) pour remettre Essuyer ; ne pas enregistrer le niveau modifié.
9. **Debug de visée** : arrête Play. Dans `BP_FirstPersonCharacter` → **Composants** → `InteractionComponent` → **Détails** → **Interaction | Debug** → coche **Afficher Debug** → **Compiler** → Play : un trait vert (qui touche, avec une petite sphère) ou rouge (rien touché) suit la visée. Décoche ensuite, **Compiler**, **ne pas enregistrer coché**.
10. **Journal de progression** (facultatif) : composant `ComposantNettoyage` → **Nettoyage | Debug** → coche **Afficher Debug** → Play → 4 × E sur la preuve → 4 lignes « Progression nettoyage : 0.250000 … 1.000000 » dans l'Output Log. Décoche ensuite.
11. **Pull** : Play, vise `BP_ObjetPull_Test`, maintiens **R** → l'objet glisse au sol devant toi en suivant ta rotation horizontale (pas le haut/bas de la souris) ; appuie sur **Espace** en le tenant → pas de saut ; relâche **R** → l'objet est lâché. Approche-le d'un bord de vide → il est lâché et tombe.
12. **Grab** : toujours non testable (aucun objet avec [Saisir], dette B2 / T002) — vérifier seulement que le **clic gauche** sur la preuve ou sur `BP_ObjetPull_Test` ne saisit rien.
13. Si tout est conforme : dis-le à l'orchestrateur pour le commit `T001: unifier la visée et faire passer le nettoyage par le profil`. Décide aussi du sort du fichier non suivi `Content/__ExternalActors__/.../M8RK6YKWGV6E3Q8C5UHZX3.uasset` (antérieur à T001).

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
