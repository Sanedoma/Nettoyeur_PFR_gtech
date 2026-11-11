# État réel du projet — NETTOYEUR

> Tenu par l'orchestrateur. Seul ce fichier fait foi sur ce qui est **réellement** implémenté.
> Dernière mise à jour : 2026-11-11 — audit initial.

## Position dans le plan du GDD

Entre les **étapes 2 et 3** (§11 : interaction générique + manipulation physique), avec une partie de l'**étape 6** (nettoyage) déjà réalisée en avance.

## Ce qui existe

| Élément | Fichiers | État |
|---|---|---|
| Projet UE 5.7.4, module C++ `Nettoyeur` | `Nettoyeur.uproject`, `Source/Nettoyeur/Nettoyeur.Build.cs` | ✅ Compile |
| Nettoyage progressif (4 essuyages → la preuve disparaît) | `ComposantNettoyage.h/.cpp`, `PreuveNettoyable.h/.cpp`, `BP_PreuveNettoyable` | ✅ Vérifié en jeu (log 2026-11-11 : 0.25 → 1.0) |
| Prompt d'interaction (nettoyage uniquement) | `WBP_Interaction` (fonction `AfficherInteraction`) | ✅ Probable |
| Saisie Grab / Pull / relâcher | `InteractionComponent.h/.cpp` | 🟡 Codé, test à confirmer |
| Profil d'interactions par objet | `InteractionProfileComponent.h/.cpp`, `InteractionTypes.h` (enum `EInteractionType` : Interact, Wipe, Grab, Push, Pull, Drag, Launch, Open, Close, Hide) | 🟡 Codé ; utilisé par Grab/Pull, **pas** par le nettoyage |
| Personnage C++ avec PhysicsHandle | `PersonnageNettoyeur.h/.cpp` | ❓ Parent de `BP_FirstPersonCharacter` à vérifier |
| Inputs | `IA_Essuyer`, `IA_Grab`, `IA_Pull`, `IA_Move`, `IA_Look`, `IA_Jump`, `IMC_Default` | 🟡 Assets présents |
| Objets de test | `BP_ObjetGrab_Test1`, `BP_ObjetPull_Test` | 🟡 |
| Niveaux | `Lvl_FirstPerson` (utilisé), `L_Maison01` | 🟡 |
| Plugin EngineWire (pilotage de l'éditeur par MCP, ports 8090/8091) | `Nettoyeur.uproject` | ✅ Actif |

## Ce qui n'existe pas encore
Lancer, accroupissement, Evidence Component, registre/Subsystem, score, contenant, lampe UV, mission, inspection, écran de résultat.

## Dette technique connue
1. **Deux systèmes de visée** : `UComposantNettoyage` fait son propre Line Trace à chaque frame (+ `DrawDebugLine` permanent) en plus du sweep de `UInteractionComponent`.
2. **Le nettoyage contourne le profil** : `Cast<APreuveNettoyable>` direct au lieu de `CanPerform(Wipe)`.
3. **État et apparence mélangés** : à 100 % la preuve est cachée et sa collision coupée ; pas de type, gravité ni état.
4. **`TenterNettoyage()` en double** : version vide dans `APersonnageNettoyeur`.
5. **Prompt fragile** : `FindFunction("AfficherInteraction")` + `ProcessEvent`.
6. **Encodage** : `InteractionTypes.h` n'est pas en UTF-8 (« Traîner » s'affiche mal).
7. `GetAimDirection()` déclaré mais inutilisé dans `UInteractionComponent`.

## Tâche en cours
Aucune.

## Prochaine tâche
**T000** — Vérification de l'éditeur et documentation rétroactive de l'existant.

## Backlog
| Tâche | Titre | Agent | Statut |
|---|---|---|---|
| T000 | Vérifier la config éditeur + documenter l'existant | editeur-contenu, gameplay-cpp | À faire |
| T001 | Unifier la visée et faire passer le nettoyage par le profil | gameplay-cpp | À faire |
| T002 | Ajouter le lancer d'objet | gameplay-cpp + editeur-contenu | À faire |
| T003 | Composant de preuve + registre + score en debug | gameplay-cpp | À faire |
| T004 | Brancher le nettoyage sur l'intensité des preuves | gameplay-cpp + editeur-contenu | À faire |
