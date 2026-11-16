# T002 — Lancer un objet tenu

**Statut :** ⏸️ En pause (2026-11-16) — graphe monté et testé par le développeur ; **problème signalé sur Launch** (détail à préciser par le développeur). Commité sur la branche, **non mergé**.
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
- [x] Journal conforme (notion d'impulsion, masse, `AddImpulse`, Input Action / Mapping Context). *(qa, 2026-11-16)*

*Note qa (2026-11-16) : les 5 critères non cochés sont conformes à la relecture du code mais n'ont pas été testés en jeu (graphe non monté, pas d'EngineWire) ; ils restent au développeur.*

## Hérité de T000 (ajouté par l'orchestrateur, 2026-11-12)
- Grab non testable aujourd'hui (B2) : aucun objet [Grab] ; `BP_ObjetGrab_Test1` a le profil [Pull] et une racine Geometry Collection vide ; relâcher le clic gauche ne lâche pas (nœud `ReleaseGrab` non branché). À régler avant de tester le lancer.
- Dette à évaluer ici : n° 8 à 12 (`doc/agents/gameplay-cpp.md` §2.9). Les n° 8 et 9 bloquent un vrai test de Grab/Pull.

## Hérité de T001 (ajouté par l'orchestrateur, 2026-11-16)
- La visée est désormais unique : `UInteractionComponent::GetCurrentFocus()` / `CanFocusPerform()` / `OnFocusChanged`. `Launch()` doit les réutiliser (pas de nouveau trace).
- `OnHoldChanged(bool)` existe : il doit rester diffusé correctement quand un lancer relâche l'objet (le prompt de `UWidgetInteraction` en dépend : masqué pendant la tenue, réaffiché après).
- Le prompt est masqué pendant qu'on tient un objet : pas de ligne « Lancer » à afficher dans cette tâche. `BP_ObjetGrab_Test1` avec [Grab, Launch] permettra en revanche de vérifier l'affichage multi-lignes non testé en T001 (critère de T001 resté ouvert) — ajouter une entrée `Launch` à la table du widget n'est **pas** demandé.
- Pour que Grab soit testable (B2) : brancher le relâchement du clic gauche sur `ReleaseGrab` (graphe décrit nœud par nœud au développeur), donner à `BP_ObjetGrab_Test1` un mesh simulant la physique et le profil [Grab, Launch].

## À vérifier par `qa` (code `gameplay-cpp`, 2026-11-16)
Après fermeture de l'éditeur et Build.bat (0 erreur, 0 nouvel avertissement), puis assets de `editeur-contenu` :
1. Grab de `BP_ObjetGrab_Test1` [Grab, Launch] : clic gauche saisit, relâcher le clic lâche (B2). Avant saisie, le prompt n'affiche qu'UNE ligne « Clic gauche : Saisir » : la table du widget n'a pas d'entrée *Lancer* (non demandée), donc ce profil ne permet PAS de vérifier le multi-lignes de T001 (voir point ouvert du rapport gameplay-cpp).
2. Saisir puis Lancer → l'objet part dans la direction de la caméra (viser haut / bas / côté), retombe ; le prompt réapparaît (`OnHoldChanged(false)`).
3. Lancer sans rien tenir → rien, aucun message dans l'Output Log.
4. Tirer `BP_ObjetPull_Test` (R) puis Lancer → rien, l'objet reste tiré.
5. Objet [Grab] sans Launch (retirer *Lancer* du profil sans enregistrer) : saisir puis Lancer → l'objet **reste tenu** (comportement choisi).
6. Contre un mur proche, `MaxLaunchSpeed` = 5000 (maximum) : l'objet ne traverse pas le mur.
7. Objet lourd (*Mass (kg)* override à 50) : part nettement moins loin (300 cm/s avec les valeurs par défaut).
8. Non-régression Pull : glisse au sol, lâché au bord d'un vide, plus lâché en passant contre / par-dessus une autre caisse physique (dette n° 9) ; montée sur plateforme (dette n° 10 : vérifier qu'il n'accroche pas).
9. Non-régression nettoyage (E ×4) et saut interdit pendant la tenue.
10. Output Log : aucun avertissement « InteractionComponent : aucun Physics Handle… » avec `BP_FirstPersonCharacter`.

## Fait par `editeur-contenu` (2026-11-16)
- `Content/Input/Actions/IA_Lancer` créé (Digital (bool)) ; ajouté à `IMC_Default` sur le **clic droit** (touche libre ; aucun autre mapping modifié).
- `BP_ObjetGrab_Test1` : `StaticMesh` (`SM_Cube`) devenu racine, *Geometry Collection* vide supprimée ; échelle 0,4 (cube de 40 cm) ; *Simulate Physics* coché ; *PhysicsActor* ; masse imposée **25 kg** (→ 600 cm/s avec les valeurs par défaut) ; profil **[Grab, Launch]**. Aucun asset ne le référençait avant.
- Instance placée dans `Lvl_FirstPerson` en (−600, 1650, 1) : au sol au pied de la rampe nord, à 1,3 m du muret nord (test mur proche).
- **Reste au développeur** : compiler, puis graphe (IA_Lancer Started → Launch ; IA_Grab Completed + Canceled → Release Grab existant), Compile + Save.

## À vérifier par `qa` (assets `editeur-contenu`)
1. `IMC_Default` : 16 mappings, `IA_Lancer` ← Right Mouse Button, les 15 autres inchangés.
2. `BP_ObjetGrab_Test1` : composants = `StaticMesh` (racine) + `InteractionProfile` ; Simulate Physics, PhysicsActor, Mass (kg) 25 coché ; Allowed Interactions = [Grab, Launch].
3. Graphe monté par le développeur conforme à `editeur-contenu.md` T002 §4.5 (capture ou texte des nœuds).
4. Puis les points 1 à 10 de la section précédente (la caisse est au pied de la rampe nord, devant le muret).

## Pause (orchestrateur, 2026-11-16)
- Le développeur a monté le graphe (`IA_Lancer` → `Launch`, `IA_Grab` Completed/Canceled → `Release Grab`) et testé dans l'éditeur.
- Un problème avec `Launch` reste à corriger : **description à recueillir auprès du développeur** à la reprise.
- Priorité donnée au jalon (tache de sang animée), voir la fiche correspondante. Décisions à inscrire dans `decisions.md` à la reprise : lancer dépendant de la masse avec vitesse plafonnée, CCD activée par le code, objet sans Launch = reste tenu.
