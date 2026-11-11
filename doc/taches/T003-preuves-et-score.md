# T003 — Composant de preuve, registre global et score de détection (debug)

**Statut :** À faire (dépend de T002)
**Agent :** `gameplay-cpp`
**Branche :** `tache/T003-preuves-score`
**Étape du GDD :** §11 étapes 4 et 5, §5.1 à §5.5, §10.2 (`UEvidenceComponent`, `UEvidenceSubsystem`)

## Objectif
Le cœur du jeu : chaque preuve a des données (type, gravité, intensité, état), elles sont centralisées dans un registre, et un score de détection en % est recalculé automatiquement quand une preuve change.

## Travail attendu
1. **`UEvidenceComponent`** (Actor Component, dossier `Source/Nettoyeur/Evidence/`) :
   - `Type` (enum `EEvidenceType` minimal : Sang, Empreinte, ObjetDeplace, Corps, Ouverture), `Gravite` (float ≥ 0), `Intensite` (0..1), `Etat` (enum `EEvidenceState` : Active, Neutralisee, Masquee, Invalide) ;
   - fonctions `ReduireIntensite(float)`, `DefinirEtat(...)`, `GetContributionScore()` ;
   - s'enregistre au `BeginPlay` et se désenregistre à `EndPlay` auprès du Subsystem ;
   - notifie le Subsystem à chaque changement (pas de Tick).
2. **`UEvidenceSubsystem`** (`UWorldSubsystem`) :
   - registre des composants de la mission ;
   - score = 100 × Σ(gravité × intensité, preuves actives) / Σ(gravité × 1.0, toutes preuves enregistrées) — formule simple du GDD §5.5 ;
   - règle explicite pour Masquée : **contribue 0** en v1 (documenter que c'est provisoire) ;
   - delegate `OnScoreChanged(float)` ; score toujours borné 0..100, 0 si aucune preuve ;
   - affichage debug à l'écran (`GEngine->AddOnScreenDebugMessage`) derrière un booléen ou une commande console.
3. Pas de Data Asset de type de preuve dans cette tâche (`UEvidenceTypeDataAsset` viendra quand un 2e type concret le justifiera — GDD §10.2).

## Hors périmètre
Brancher le nettoyage (T004), objet déplacé par comparaison de transform (tâche ultérieure), UI finale.

## Critères d'acceptation (GDD §13.3)
- [ ] Une preuve neuve a une intensité entre 0 et 1.
- [ ] Le score affiché change dès qu'on modifie l'intensité d'une preuve (test via une fonction appelable en console ou un acteur de test).
- [ ] Une preuve neutralisée ne contribue plus au score.
- [ ] Score toujours entre 0 et 100 ; 0 % s'il n'y a aucune preuve ; pas de division par zéro.
- [ ] Un acteur détruit est retiré du registre et le score se met à jour.
- [ ] Tests : zéro preuve, une preuve complète, une partielle, une neutralisée, deux preuves de gravités différentes.
- [ ] Compilation sans nouvel avertissement ; journal conforme (notions : World Subsystem, cycle de vie BeginPlay/EndPlay, `TArray`, `TWeakObjectPtr`, delegate, formule commentée).

## Hérité de T000 (ajouté par l'orchestrateur, 2026-11-12)
- Dette n° 3 (état/apparence mélangés) et n° 14 (progression par preuve et non totale → « mission terminée » à la première preuve, logique en Blueprint dans `BP_FirstPersonCharacter`).
