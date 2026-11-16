# État réel du projet — NETTOYEUR

> Tenu par l'orchestrateur. Seul ce fichier fait foi sur ce qui est **réellement** implémenté.
> Dernière mise à jour : 2026-11-16 — fin de T001 (visée unique, nettoyage par le profil, prompt par interaction).

## Position dans le plan du GDD

Entre les **étapes 2 et 3** (§11 : interaction générique + manipulation physique), avec une partie de l'**étape 6** (nettoyage) déjà réalisée en avance.

## Ce qui existe

| Élément | Fichiers | État |
|---|---|---|
| Projet UE 5.7.4, module C++ `Nettoyeur` | `Nettoyeur.uproject`, `Source/Nettoyeur/Nettoyeur.Build.cs` | ✅ Compile (Build.bat, 0 erreur / 0 warning — QA 2026-11-12) |
| Nettoyage progressif (4 essuyages → la preuve disparaît) | `ComposantNettoyage.h/.cpp`, `PreuveNettoyable.h/.cpp`, `BP_PreuveNettoyable` (profil [Wipe], 0.25 par essuyage) | ✅ Validé en jeu (2026-11-16) ; décidé par le profil (`CanFocusPerform(Wipe)`), utilise la visée de `UInteractionComponent` (plus de trace propre) |
| Prompt d'interaction (une ligne par interaction autorisée) | `WidgetInteraction.h/.cpp` (parent C++ de `WBP_Interaction`, écoute `OnFocusChanged` / `OnHoldChanged`), textes dans les Class Defaults (*Interaction \| Prompt*) ; `WBP_Interaction` créé au BeginPlay de `BP_FirstPersonCharacter` | ✅ Validé en jeu (2026-11-16) : « E : Nettoyer » + barre sur la preuve, « R (maintenir) : Tirer » sur l'objet Pull, masqué pendant qu'on tient un objet |
| « Mission terminée » | Logique **Blueprint** dans `BP_FirstPersonCharacter` (Custom Event `RecevoirProgressionNettoyage`) | 🟡 Se déclenche dès la **première** preuve à 100 % (contraire au GDD §10.5) |
| Pull (R maintenu → l'objet glisse au sol, relâcher R → lâché) | `InteractionComponent.h/.cpp`, `BP_ObjetPull_Test` (profil [Pull]) | ✅ Validé en jeu (2026-11-12) |
| Grab (clic gauche) | `InteractionComponent.h/.cpp` | 🟡 Codé mais **non testable** : aucun objet n'a le profil [Grab] ; relâcher le clic gauche ne lâche pas l'objet (le nœud `ReleaseGrab` n'est pas branché) |
| Profil d'interactions par objet | `InteractionProfileComponent.h/.cpp`, `InteractionTypes.h` | ✅ Utilisé par Grab, Pull, le nettoyage et le prompt |
| Personnage C++ | `PersonnageNettoyeur.h/.cpp` ; `BP_FirstPersonCharacter` en hérite. Composants : `InteractionComponent` (seule visée : sweep sphère, portée `TraceDistance` 300, debug `bAfficherDebug`) + `PhysicsHandle` (C++), `ComposantNettoyage` (ajouté **en Blueprint**) | ✅ |
| Inputs | `IMC_Default` : ZQSD + flèches, Espace, E → `IA_Essuyer`, R → `IA_Pull`, clic gauche → `IA_Grab`, manette ; `IMC_MouseLook`. Les deux IMC sont ajoutés par `BP_FirstPersonPlayerController` | ✅ |
| Game Mode / maps | `BP_FirstPersonGameMode` ; Startup et Game Default = `Lvl_FirstPerson` | ✅ |
| `L_Maison01` | Niveau de test « première maison » (1 preuve, pas d'objet Pull/Grab), référencé par rien | 🟡 Inutilisé pour l'instant |
| `BP_ObjetGrab_Test1` | Profil [Pull] (pas Grab), racine Geometry Collection vide, placé dans aucun niveau | ❌ Mal configuré |
| `BP_FirstPersonCharacter_Backup` | Sauvegarde d'avant le reparentage (parent `Character`), référencée par rien | ❓ À garder ou supprimer : **décision du développeur** |
| Plugin EngineWire (pilotage de l'éditeur par MCP) | `Nettoyeur.uproject` (voir `decisions.md` D006) | ✅ Actif |

Détail complet : `doc/agents/editeur-contenu.md` (T000 partie A) et `doc/agents/gameplay-cpp.md` (T000 partie B).

## Ce qui n'existe pas encore
Lancer, accroupissement, Evidence Component, registre/Subsystem, score, contenant, lampe UV, mission, inspection, écran de résultat.

## Dette technique connue (numérotation de `gameplay-cpp.md` §2.9)
| # | Problème | Traité dans |
|---|---|---|
| 1 | Deux systèmes de visée : `UComposantNettoyage` refait son Line Trace (Tick **et** `TenterNettoyage`, portée 300 en dur, caméra via `FindComponentByClass`) + `DrawDebugLine` permanent | T001 ✅ |
| 2 | Le nettoyage contourne le profil (`Cast<APreuveNettoyable>` au lieu de `CanPerform(Wipe)`) | T001 ✅ |
| 3 | État et apparence mélangés dans la preuve | T003 / T004 |
| 4 | `APersonnageNettoyeur::TenterNettoyage()` vide, `BlueprintCallable` — **confirmé non appelé** par les Blueprints (T000) | T001 ✅ |
| 5 | Prompt fragile : `FindFunction` + `ProcessEvent` à chaque frame (risque mémoire si la signature change) | T001 ✅ (plus de `ProcessEvent` : le prompt est un widget à parent C++) |
| 6 | ~~Encodage de `InteractionTypes.h`~~ | ✅ Corrigé en T000 |
| 7 | `GetAimDirection()` inutilisé ; calcul du point de vue réécrit 4 fois | T001 ✅ |
| 8 | Pull/Grab saisissent le composant **racine**, pas le composant touché | T002 |
| 9 | Recherche du sol en Pull : `LineTraceMulti` s'arrête au premier bloquant → l'objet est lâché au-dessus d'un autre objet physique | T002 |
| 10 | Hauteur lissée deux fois en Pull (`VInterpTo` puis `FInterpTo`) | T002 |
| 11 | Hauteur de pose calculée sur la boîte englobante monde (objet tourné : flotte ou frotte) | T002 |
| 12 | Le Physics Handle n'est fourni à `UInteractionComponent` que par le constructeur du personnage | T002 |
| 13 | `UComposantNettoyage` recalcule `Personnage`/`CameraJoueur` à chaque frame, exige un `ACharacter` | T001 ✅ |
| 14 | La progression diffusée est celle de la preuve essuyée, pas un total → « mission terminée » à la 1ʳᵉ preuve | T003 / T004 |
| 15 | Restes de template (includes, Tick/Setup vides, Clamp sur propriété non éditable, catégorie `Interaction|Pull` partagée, `WidgetInteraction` modifiable deux fois) | T001 🟡 partiel (reste : Clamp de `PreuveNettoyable.h`, includes du `.h` du personnage) |
| 16 | `UE_LOG(LogTemp, Warning)` à chaque essuyage ; `GetWorld()` non testé | T001 ✅ |
| B1 | Graphe de `BP_FirstPersonCharacter` : bloc mort Line Trace + Print String, variable widget `NewVar`, Print String de debug dans `MettreAJourProgression` | T001 ✅ (fait par le développeur) |
| B2 | Grab : aucun objet [Grab], `BP_ObjetGrab_Test1` mal configuré, relâcher le clic gauche ne lâche pas | T002 |

## Points en attente du développeur
- Nouvelles remarques QA T001 pour T003/T004 : la barre n'est jamais remise à 0 (deuxième preuve) ; un acteur [Wipe] qui n'est pas une `APreuveNettoyable` affiche le prompt sans effet ; la progression 1.0 peut être diffusée deux fois.
- ~~`Nettoyeur.uproject`~~ : commité séparément le 2026-11-12, voir `decisions.md` D006.
- Fichier non suivi `Content/__ExternalActors__/FirstPerson/Lvl_FirstPerson/0/I6/M8RK6YKWGV6E3Q8C5UHZX3.uasset` (un PointLight dans `Lvl_FirstPerson`) : garder ou supprimer ?
- `BP_FirstPersonCharacter_Backup` : garder ou supprimer ?

## Tâche en cours
**T002** — Ajouter le lancer d'objet (voir la fiche).

## Prochaine tâche
T003 — Composant de preuve + registre + score en debug (après validation de T002).

## Backlog
| Tâche | Titre | Agent | Statut |
|---|---|---|---|
| T000 | Vérifier la config éditeur + documenter l'existant | editeur-contenu, gameplay-cpp | ✅ Terminé (2026-11-12) |
| T001 | Unifier la visée et faire passer le nettoyage par le profil | gameplay-cpp | ✅ Terminé (2026-11-16) |
| T002 | Ajouter le lancer d'objet | gameplay-cpp + editeur-contenu | En cours |
| T003 | Composant de preuve + registre + score en debug | gameplay-cpp | À faire |
| T004 | Brancher le nettoyage sur l'intensité des preuves | gameplay-cpp + editeur-contenu | À faire |
