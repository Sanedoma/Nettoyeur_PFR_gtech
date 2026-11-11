# T004 — Brancher le nettoyage existant sur l'intensité des preuves

**Statut :** À faire (dépend de T003)
**Agents :** `gameplay-cpp`, puis `editeur-contenu`
**Branche :** `tache/T004-nettoyage-preuves`
**Étape du GDD :** §11 étape 6, §6.3, §5.2 (séparer état et apparence)

## Objectif
Nettoyer une tache fait **réellement** baisser le score de détection, sans casser la mécanique de nettoyage qui fonctionne déjà.

## Travail attendu
### `gameplay-cpp`
1. `APreuveNettoyable` reçoit un `UEvidenceComponent` (type Sang par défaut, gravité réglable).
2. `Essuyer()` réduit l'intensité de la preuve de `ProgressionParEssuyage` ; `ProgressionNettoyage` devient une valeur dérivée (1 − intensité) pour rester compatible avec le widget actuel.
3. À intensité 0 : état **Neutralisée** (donnée), puis l'apparence suit (cacher le mesh) — l'ordre « donnée d'abord, visuel ensuite » est documenté.
4. Le delegate `OnProgressionNettoyageChanged` continue d'être émis avec les mêmes valeurs.

### `editeur-contenu`
1. Régler `BP_PreuveNettoyable` : gravité **2** (sang, GDD §5.4).
2. Placer dans `Lvl_FirstPerson` deux preuves de gravités différentes (ex. 2 et 7) pour tester le score.

## Fichiers / assets autorisés
`PreuveNettoyable.h/.cpp`, `ComposantNettoyage.h/.cpp` (si besoin), `BP_PreuveNettoyable`, `Lvl_FirstPerson`.

## Critères d'acceptation
- [ ] Au lancement, le score de détection est de 100 % (toutes les preuves intactes).
- [ ] Essuyer une preuve de gravité 2 une fois (sur deux preuves 2 et 7) fait baisser le score de la bonne valeur (calcul attendu écrit dans le rapport QA).
- [ ] Une preuve totalement nettoyée est Neutralisée, ne compte plus, et disparaît visuellement.
- [ ] Le widget affiche toujours la progression 0.25 → 1.0.
- [ ] Nettoyer les deux preuves → score 0 %.
- [ ] Grab, Pull, Lancer inchangés.
- [ ] Journal conforme.
