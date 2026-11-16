# T001 — Unifier la visée et faire passer le nettoyage par le profil d'interaction

**Statut :** Validé QA — en attente du développeur (2026-11-13 ; tests en jeu à faire, voir `doc/agents/qa.md`, QA T001)
**Agent :** `gameplay-cpp`
**Branche :** `tache/T001-unifier-visee`
**Étape du GDD :** §11 étape 2, §6.1 « Principe confirmé : interactions indépendantes et assignables »

## Objectif
Une seule source de vérité pour « ce que le joueur vise » : `UInteractionComponent`. Le nettoyage utilise cette cible et vérifie que la cible autorise l'interaction `Wipe`, au lieu de faire son propre rayon et un `Cast` direct.

## Travail attendu
1. `UInteractionComponent` expose la cible courante (ex. `GetCurrentFocus()`) et notifie son changement (ex. delegate `OnFocusChanged`), pour que le prompt ne soit plus recalculé à chaque frame par un autre composant.
2. `UComposantNettoyage` :
   - n'effectue plus de Line Trace propre ; il utilise la cible de `UInteractionComponent` ;
   - vérifie `CanPerform(EInteractionType::Wipe)` sur le profil de la cible avant d'essuyer ;
   - conserve exactement le comportement actuel : +0.25 par essuyage, delegate `OnProgressionNettoyageChanged`, mise à jour du prompt ;
   - le `DrawDebugLine` passe derrière un booléen réglable `bAfficherDebug` (faux par défaut).
3. Supprimer la fonction vide `APersonnageNettoyeur::TenterNettoyage()` **seulement si** T000 confirme qu'aucun Blueprint ne l'appelle ; sinon la signaler.
4. Retirer `GetAimDirection()` si inutilisé.

## Hors périmètre
- Le remplacement du `ProcessEvent` du prompt (sera traité avec l'UI de feedback).
- Le système de preuves (T003).

## Fichiers autorisés
`InteractionComponent.h/.cpp`, `ComposantNettoyage.h/.cpp`, `PersonnageNettoyeur.h/.cpp`.

## Critères d'acceptation
- [x] Un seul trace/sweep de visée par frame dans tout le projet. *(QA : grep de `Source/`)*
- [ ] Viser une `BP_PreuveNettoyable` affiche le prompt ; ne plus la viser le masque.
- [ ] Essuyer 4 fois fait passer la progression 0.25 → 0.5 → 0.75 → 1.0 et la preuve disparaît (comme avant).
- [ ] Un objet **sans** `Wipe` dans son profil ne peut pas être essuyé et n'affiche pas le prompt de nettoyage.
- [ ] Pas de ligne de debug à l'écran par défaut ; `bAfficherDebug = true` la réaffiche.
- [ ] Grab et Pull inchangés.
- [x] Compilation sans nouvel avertissement. *(QA : Build.bat, recompilation forcée des 7 `.cpp`, 0 erreur / 0 avertissement)*
- [x] Journal conforme à `doc/CONVENTION.md` (notamment : delegate, binding, pourquoi supprimer un double trace).

*Critères 2 à 6 : vérifiés par lecture du code uniquement ; test en jeu par le développeur (procédure : `doc/agents/qa.md`, QA T001).*

## Hérité de T000 (ajouté par l'orchestrateur, 2026-11-12)
- T000 confirme qu'**aucun Blueprint n'appelle** `APersonnageNettoyeur::TenterNettoyage()` : le point 3 peut être appliqué.
- Dette à traiter ici (voir `doc/agents/gameplay-cpp.md` §2.9 et `doc/00_etat-projet.md`) : n° 1, 2, 4, 5, 7, 13, 16 ; n° 15 au passage, sans élargir le périmètre.
- Côté Blueprint (B1) : décrire au développeur, nœud par nœud, le retrait du bloc mort Line Trace + Print String et des Print String de debug de `MettreAJourProgression` dans `BP_FirstPersonCharacter`.

## Livraison gameplay-cpp (2026-11-13)
Détail : `doc/agents/gameplay-cpp.md`, entrée T001. Compilation Build.bat : **Succeeded, 0 erreur, 0 avertissement**. **Non testé en jeu** par l'agent.

### À vérifier par `qa`
1. **Compilation** Build.bat éditeur fermé : 0 erreur / 0 avertissement.
2. **Un seul trace de visée** : dans `Source/Nettoyeur/*.cpp`, seuls `SweepSingleByChannel` (`UInteractionComponent::UpdateFocus`) et la trace vers le sol du Pull (`UpdateHold`, pas une visée) existent ; `UComposantNettoyage` n'a plus ni trace ni Tick.
3. **Profil** : `BP_PreuveNettoyable` → `ProfilInteractions` → *Allowed Interactions* = [Essuyer] (hérité du C++, aucun asset ne surcharge la liste d'après une recherche dans `Content/`).
4. **Prompt** : dans `Lvl_FirstPerson`, viser la preuve → « Appuie sur E pour nettoyer » + barre ; détourner le regard → masqués.
5. **Nettoyage** : 4 × E → barre 0.25 → 0.5 → 0.75 → 1.0, la preuve disparaît, le prompt se masque, « MISSION TERMINÉE ! » s'affiche (comme avant). Avec `ComposantNettoyage` → *Afficher Debug* coché : 4 lignes « Progression nettoyage » dans l'Output Log.
6. **Sans Wipe** : viser `BP_ObjetPull_Test` ([Tirer]) → pas de prompt, E ne fait rien. Test complémentaire (sans enregistrer) : sur l'instance de preuve du niveau, retirer *Essuyer* du profil → plus de prompt, E sans effet.
7. **Debug** : aucune ligne à l'écran par défaut ; `InteractionComponent` → **Interaction | Debug** → *Afficher Debug* coché → trait vert (touche) / rouge (rien) qui suit la visée.
8. **Grab / Pull inchangés** : R maintenu sur `BP_ObjetPull_Test` → glisse au sol, relâcher → lâché ; pas de saut en tenant. Grab : toujours non testable (aucun objet [Saisir], dette B2).
9. **Blueprints** : `BP_FirstPersonCharacter` et `WBP_Interaction` compilent sans erreur après recompilation C++ (suppression de `APersonnageNettoyeur::TenterNettoyage`, `WidgetInteraction` passé en lecture seule, catégories renommées).
10. **Output Log** au lancement : aucun message « ComposantNettoyage : aucun InteractionComponent… » ni « …AfficherInteraction doit avoir un seul parametre… ».
11. Journal et `doc/notions.md` conformes à `doc/CONVENTION.md`.

### Étape pour le développeur (B1)
Nettoyage du graphe de `BP_FirstPersonCharacter` (bloc mort Line Trace + Print String, renommage optionnel de `NewVar`) et des Print String de `MettreAJourProgression` : procédure nœud par nœud dans `doc/agents/gameplay-cpp.md`, T001 §4.b.

## Extension demandée par le développeur (orchestrateur, 2026-11-16)
**Demande :** « régler l'affichage pour afficher le message correspondant pour chaque interaction ». Le prompt ne connaît aujourd'hui que le nettoyage (texte figé « Appuie sur E pour nettoyer »).

**Statut de l'extension :** Validé QA (hors jeu) — en attente du développeur (2026-11-16). Compilation OK (0 erreur, 0 avertissement), code et doc relus ; reparentage de `WBP_Interaction` et tests en jeu à faire par le développeur (rapport « QA T001 (extension) » dans `doc/agents/qa.md`). La partie principale de T001 reste « Validé QA — en attente du développeur ».

### Travail attendu
1. Nouvelle classe C++ `UWidgetInteraction` (parent `UUserWidget`), fichiers `WidgetInteraction.h/.cpp`, qui deviendra le parent de `WBP_Interaction` :
   - widgets liés par nom (`meta = (BindWidget)`) : `TexteInteraction` (UTextBlock) et `BarProgression` (UProgressBar) — mêmes noms que dans le Designer actuel ;
   - table réglable `TMap<EInteractionType, FText> TextesInteractions` (`EditAnywhere`, catégorie « Interaction|Prompt ») avec valeurs par défaut : Wipe → « E : Nettoyer », Pull → « R (maintenir) : Tirer », Grab → « Clic gauche : Saisir » ;
   - au `NativeConstruct`, le widget trouve l'`UInteractionComponent` du pawn propriétaire et s'abonne à `OnFocusChanged` (désabonnement au `NativeDestruct`) ;
   - à chaque changement de cible : une ligne par interaction **autorisée par le profil de la cible ET présente dans la table**, dans l'ordre de la table ; rien à afficher → texte masqué ; la barre n'est visible que si la cible autorise Wipe ;
   - pendant qu'un objet est tenu (`IsHoldingObject()`), prompt masqué.
2. `UInteractionProfileComponent` / `UInteractionComponent` : exposer ce qu'il faut pour lire la liste des interactions de la cible sans dupliquer de logique (réutiliser `CanFocusPerform`/le profil existant).
3. `UComposantNettoyage` : ne pilote plus le prompt. Supprimer `MettreAJourTexteInteraction` et son `FindFunction`/`ProcessEvent` (dette n° 5 soldée). `DefinirWidgetInteraction` reste appelable par le Blueprint existant (pas de nœud cassé) mais ne sert plus qu'à garder la référence, ou devient inutile : le documenter. `OnProgressionNettoyageChanged` et `MettreAJourProgression` (barre, mission terminée) inchangés.
4. Décrire au développeur, pas à pas, le reparentage de `WBP_Interaction` vers `WidgetInteraction` (Graph → Class Settings → Parent Class) et ce qu'il devient de la fonction Blueprint `AfficherInteraction` (plus appelée : la laisser ou la supprimer). Commit propre avant le reparentage.

### Fichiers autorisés (extension)
Ceux de T001 + `WidgetInteraction.h/.cpp` (nouveaux), `InteractionProfileComponent.h/.cpp`.

### Critères d'acceptation (extension)
- [ ] Viser la preuve : « E : Nettoyer » + barre. Viser `BP_ObjetPull_Test` : « R (maintenir) : Tirer », pas de barre. Ne rien viser : rien.
- [ ] Un objet avec plusieurs interactions affiche une ligne par interaction.
- [ ] Les textes se modifient dans les Class Defaults de `WBP_Interaction` sans recompiler le C++.
- [ ] Prompt masqué pendant qu'on tient un objet (Pull).
- [ ] Nettoyage, barre et « mission terminée » inchangés.
- [x] Plus aucun `ProcessEvent` / `FindFunction` dans `Source/`. _(QA 2026-11-16)_
- [x] Compilation sans avertissement ; journal et notions à jour. _(QA 2026-11-16)_

### Livraison gameplay-cpp — extension (2026-11-16)
Détail : `doc/agents/gameplay-cpp.md`, entrée « T001 (extension) — Prompt par interaction ». **Compilation non faite** : l'éditeur était ouvert (nouvelle `UCLASS` → Build.bat éditeur fermé obligatoire). **Non testé en jeu.** Aucun commit.

Fichiers : `WidgetInteraction.h/.cpp` (créés), `InteractionComponent.h/.cpp` (delegate `OnHoldChanged`), `ComposantNettoyage.h/.cpp` (prompt retiré). `InteractionProfileComponent` et `Nettoyeur.Build.cs` non modifiés.

#### À vérifier par `qa`
1. **Compilation** Build.bat éditeur fermé : 0 erreur / 0 avertissement.
2. `grep` dans `Source/` : aucun `ProcessEvent`, `FindFunction`, `MettreAJourTexteInteraction`.
3. **Reparentage** de `WBP_Interaction` → `WidgetInteraction` (procédure journal §4.a) : le WBP compile sans erreur de *widget binding* ; `MettreAJourProgression` et `AfficherMissionTerminee` toujours présents et appelés par le personnage ; `BP_FirstPersonCharacter` compile (nœud *Definir Widget Interaction* intact).
4. **Class Defaults** de `WBP_Interaction` → **Interaction | Prompt** → *Textes Interactions* : Essuyer → « E : Nettoyer », Tirer → « R (maintenir) : Tirer », Saisir → « Clic gauche : Saisir ».
5. **En jeu** (procédure journal §4.d) : rien visé → rien ; preuve → « E : Nettoyer » + barre ; `BP_ObjetPull_Test` → « R (maintenir) : Tirer » sans barre ; R maintenu → prompt masqué, relâché → revient.
6. **Plusieurs interactions** : instance avec [Tirer, Saisir] (sans enregistrer) → deux lignes, ordre de la table.
7. **Texte modifié** dans les Class Defaults → affiché sans recompiler le C++.
8. **Nettoyage inchangé** : 4 × E → 0.25 → 1.0, disparition, « MISSION TERMINÉE ! ».
9. **Output Log** : pas d'avertissement « WidgetInteraction : aucun InteractionComponent… ».
10. Journal et `doc/notions.md` (4 notions : BindWidget, NativeConstruct/NativeDestruct, TMap, FText/LOCTEXT) conformes à `doc/CONVENTION.md`.

#### Pour le développeur
Fermer l'éditeur → compiler → commit (code + `BP_FirstPersonCharacter.uasset` modifié dans l'éditeur, à vérifier) → reparentage de `WBP_Interaction` et suppression conseillée de la fonction `AfficherInteraction` (journal §4.a à c).
