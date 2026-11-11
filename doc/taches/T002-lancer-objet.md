# T002 — Lancer un objet tenu

**Statut :** À faire (dépend de T001)
**Agents :** `gameplay-cpp` (code), puis `editeur-contenu` (input + réglage des objets de test)
**Branche :** `tache/T002-lancer-objet`
**Étape du GDD :** §11 étape 3 (« Ajouter le lancer avec une impulsion limitée »), §6.2

## Objectif
Terminer l'étape 3 : pouvoir lancer un objet saisi avec Grab, de façon prévisible et limitée.

## Travail attendu
### `gameplay-cpp`
1. Fonction `Launch()` dans `UInteractionComponent` : si un objet est tenu en Grab **et** que son profil autorise `EInteractionType::Launch`, le relâcher et lui appliquer une impulsion dans la direction de la caméra.
2. Force réglable (`LaunchImpulse`), indépendante ou non de la masse (choix à justifier dans la doc), et plafonnée.
3. Aucun effet si rien n'est tenu, si l'objet est en Pull, ou si `Launch` n'est pas autorisé.

### `editeur-contenu`
1. Créer `IA_Lancer` (Input Action, booléen) et l'ajouter à `IMC_Default` (touche à proposer, ex. clic droit ou F — vérifier les conflits).
2. Relier `IA_Lancer` à `Launch()` dans `BP_FirstPersonCharacter` (graphe décrit nœud par nœud pour que le développeur le monte).
3. Ajouter `Launch` au profil de `BP_ObjetGrab_Test1`.

## Fichiers / assets autorisés
`InteractionComponent.h/.cpp`, `Content/Input/Actions/IA_Lancer`, `IMC_Default`, `BP_FirstPersonCharacter` (graphe d'entrée uniquement), `BP_ObjetGrab_Test1`.

## Critères d'acceptation
- [ ] Saisir `BP_ObjetGrab_Test1` puis lancer → l'objet part vers l'avant de la caméra et retombe.
- [ ] Un objet sans `Launch` dans son profil est seulement relâché ou rien ne se passe (comportement documenté).
- [ ] Appuyer sur Lancer sans objet tenu ne fait rien et ne génère pas d'erreur.
- [ ] L'objet lancé ne traverse pas les murs à force maximale (tester contre un mur proche).
- [ ] Grab, Pull, Release, nettoyage inchangés.
- [ ] Journal conforme (notion d'impulsion, masse, `AddImpulse`, Input Action / Mapping Context).

## Hérité de T000 (ajouté par l'orchestrateur, 2026-11-12)
- Grab non testable aujourd'hui (B2) : aucun objet [Grab] ; `BP_ObjetGrab_Test1` a le profil [Pull] et une racine Geometry Collection vide ; relâcher le clic gauche ne lâche pas (nœud `ReleaseGrab` non branché). À régler avant de tester le lancer.
- Dette à évaluer ici : n° 8 à 12 (`doc/agents/gameplay-cpp.md` §2.9). Les n° 8 et 9 bloquent un vrai test de Grab/Pull.
