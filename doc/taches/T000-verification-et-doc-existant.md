# T000 — Vérifier la config éditeur et documenter l'existant

**Statut :** À faire
**Agents :** `editeur-contenu` (partie A), puis `gameplay-cpp` (partie B)
**Branche :** `tache/T000-verification-existant`
**Étape du GDD :** préalable (§10.1 : « le code actuel doit être examiné avant de choisir les signatures définitives »)

## Objectif
Savoir exactement comment le prototype actuel est câblé, et amorcer la documentation (`doc/notions.md`) à partir du code déjà écrit, pour que les tâches suivantes partent d'une base connue.

## Partie A — `editeur-contenu` (lecture seule, aucune modification d'asset)
Vérifier et documenter :
1. La classe parente de `BP_FirstPersonCharacter` (est-ce `APersonnageNettoyeur` ?).
2. Les composants présents dans `BP_FirstPersonCharacter` (y a-t-il `UComposantNettoyage` ? `UInteractionComponent` ?), et dans quel ordre.
3. Le graphe d'événements de `BP_FirstPersonCharacter` : comment `IA_Essuyer`, `IA_Grab`, `IA_Pull` sont reliés (nœud par nœud).
4. Comment `WBP_Interaction` est créé, ajouté à l'écran et passé à `DefinirWidgetInteraction` ; le contenu de sa fonction `AfficherInteraction` et de l'événement lié à `OnProgressionNettoyageChanged`.
5. Les touches associées dans `IMC_Default`.
6. Les réglages de `BP_PreuveNettoyable`, `BP_ObjetGrab_Test1`, `BP_ObjetPull_Test` (mesh, physique, profil d'interactions).
7. Le Game Mode et la map par défaut (Project Settings → Maps & Modes) ; à quoi sert `L_Maison01`.
8. Le rôle de `BP_FirstPersonCharacter_Backup` (à garder ? — ne pas supprimer, signaler).

## Partie B — `gameplay-cpp`
1. Documenter **rétroactivement** le code existant (`PersonnageNettoyeur`, `InteractionComponent`, `InteractionProfileComponent`, `InteractionTypes`, `ComposantNettoyage`, `PreuveNettoyable`) : comment chaque classe fonctionne et comment elles collaborent.
2. Expliquer toutes les notions rencontrées et remplir `doc/notions.md` (ex. `UCLASS`/`UPROPERTY`/`UFUNCTION`, `GENERATED_BODY`, `TObjectPtr`, `CreateDefaultSubobject`, Actor Component, Tick, Line Trace vs Sweep, canaux de collision, Physics Handle, interpolation `VInterpTo`, `UENUM`, delegate dynamique multicast, `ProcessEvent`).
3. **Seule modification de code autorisée :** réenregistrer `InteractionTypes.h` en UTF-8 pour corriger l'affichage de « Traîner ». Aucun autre changement.

## Hors périmètre
Toute correction de la dette technique (traitée en T001+).

## Critères d'acceptation
- [ ] `doc/agents/editeur-contenu.md` décrit les 8 points de la partie A, graphes nœud par nœud.
- [ ] `doc/agents/gameplay-cpp.md` explique chaque classe existante et leurs liens.
- [ ] `doc/notions.md` contient au moins les notions listées en B.2, chacune avec un lien.
- [ ] `InteractionTypes.h` est en UTF-8 ; le projet compile ; « Traîner » s'affiche correctement dans l'éditeur.
- [ ] Non-régression : nettoyage, Grab et Pull fonctionnent comme avant.
- [ ] L'orchestrateur a mis à jour `doc/00_etat-projet.md` (lignes ❓ / 🟡 levées).
