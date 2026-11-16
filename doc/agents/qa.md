# Journal — qa

> Entrées selon `doc/CONVENTION.md`, la plus récente en haut.

---

## QA T002 — Lancer un objet tenu — 2026-11-16
**Verdict :** ✅ Validé QA pour tout ce qui est vérifiable sans lancer le jeu (compilation, code, assets lus hors éditeur, périmètre, doc). Le graphe de `BP_FirstPersonCharacter` **n'est pas monté** (laissé au développeur, normal) et **aucun test en jeu n'a été fait** (éditeur fermé, pas d'EngineWire) : 5 critères sur 6 restent à valider par le développeur (procédure en bas).

### Compilation
- Méthode : éditeur fermé, PowerShell, `Build.bat NettoyeurEditor Win64 Development -Project=... -WaitMutex` (commande de `CLAUDE.md` §6).
- Résultat : **`Result: Succeeded`** en 17,3 s. UHT : 3 fichiers générés (`-WarningsAsErrors` actif, aucune erreur). 8 actions : `PersonnageNettoyeur.cpp`, `InteractionComponent.cpp`, `ComposantNettoyage.cpp`, `WidgetInteraction.cpp`, `Module.Nettoyeur.gen.cpp`, link `.lib` + `.dll`, metadata. **0 erreur, 0 avertissement** (la seule ligne en plus est le message normal de l'éditeur de liens « Création de la bibliothèque … .lib et de l'objet … .exp »).

### Critères d'acceptation
- [ ] Saisir `BP_ObjetGrab_Test1` puis lancer → part vers l'avant de la caméra et retombe — **non testé en jeu**. Code conforme : direction = `GetViewPoint` (rotation de la caméra du PlayerController) lue **avant** `ReleaseGrab()`, puis `AddImpulse(Direction × Vitesse, NAME_None, bVelChange = true)` sur le composant tenu. Asset : profil [Grab, Launch], Simulate Physics, masse imposée → 15000 / 25 = 600 cm/s attendus.
- [ ] Objet sans `Launch` → comportement documenté — **non testé en jeu**. Code : `ActorAllows(HeldComponent->GetOwner(), Launch)` faux → `return` avant `ReleaseGrab` ⇒ l'objet **reste tenu** (choix documenté `gameplay-cpp.md` T002 §2.1 point 2 ; à reporter dans `decisions.md` par l'orchestrateur).
- [ ] Lancer sans objet tenu → rien, aucune erreur — **non testé en jeu**. Code : première ligne `if (!IsHoldingObject() || !bIsGrabbing) return;` ; `IsHoldingObject()` vérifie `PhysicsHandle` avant de le déréférencer ; aucun `UE_LOG` dans `Launch`.
- [ ] Ne traverse pas les murs à force maximale — **non testé en jeu**. Code : vitesse plafonnée (`MaxLaunchSpeed`, `ClampMax = 5000`) + `SetUseCCD(true)` avant l'impulsion. Instance placée à 1,3 m du muret nord (lu dans le fichier d'acteur, voir Assets).
- [ ] Grab, Pull, Release, nettoyage inchangés — **non testé en jeu**. Relecture : voir « Non-régression » ; aucun risque bloquant trouvé, deux changements de sensation à surveiller (dette n° 9 et 10).
- [x] Journal conforme (impulsion, masse, `AddImpulse`, Input Action / Mapping Context) — `gameplay-cpp.md` T002 : notions *Impulsion, masse et AddImpulse*, *CCD*, *static*, *constexpr*, *IsValid*, *Bounds* expliquées avec ancre ; `editeur-contenu.md` T002 : *Changer la racine*, *Masse imposée* ; Input Action / Mapping Context déjà dans `notions.md` (T000) et liés ; les 9 nouvelles lignes de `notions.md` pointent vers des ancres existantes (vérifié par recherche des `id="notion-…"`).

### Non-régression (relecture, sans jeu)
- **Visée** : même sweep, même portée ; seul ajout `CurrentFocusComponent = bHit ? Hit.GetComponent() : nullptr`, remis à nul aussi dans la branche « pas de PlayerController ». **Aucun nouveau trace de visée** (`Launch` n'en fait aucun). `OnFocusChanged` toujours piloté par l'acteur.
- **Pull validé en T001** : `GetFocusPrimitive` préfère le composant touché **s'il est `IsValid`, appartient à la cible et simule** ; sinon la racine comme avant. Pour `BP_ObjetPull_Test` (racine = mesh simulé) le composant renvoyé est le même qu'avant.
- **Recherche du sol (dette n° 9)** : boucle de `LineTraceSingleByChannel` bornée par `constexpr MaxGroundTraces = 8`, `AddIgnoredComponent` sur chaque objet simulé rencontré ; sortie sur « rien touché » (→ lâché, bord de vide, comme avant) ou premier bloquant non simulé (sol). Toujours terminante. Cas courant : 1 trace.
- **Dette n° 10** : `FInterpTo` de Z supprimé, reste un seul `VInterpTo` ⇒ en montée de plateforme, l'objet tiré monte plus lentement qu'avant : **à vérifier en jeu** (point 8 ci-dessous).
- **Dette n° 11** : `OriginToBottom = Loc.Z − (Bounds.Origin.Z − Bounds.BoxExtent.Z)` ; pour un pivot centré, égal à l'ancien `BoxExtent.Z`. Pour `BP_ObjetGrab_Test1` (`SM_Cube`, pivot à la base), la valeur est ~0, ce qui est juste.
- **Dette n° 12** : `FindComponentByClass<UPhysicsHandleComponent>()` au `BeginPlay` seulement si aucun handle fourni ; avertissement sinon. `APersonnageNettoyeur` fournit toujours le sien ⇒ l'avertissement ne doit **pas** apparaître (point 10).
- **Relâchement** : `Launch` passe par `ReleaseGrab()` ⇒ `ReleaseComponent()`, `bIsGrabbing = false`, `OnHoldChanged(false)` ⇒ le prompt de `UWidgetInteraction` réapparaît comme pour un relâchement normal. Le pointeur local `HeldComponent` est pris avant (`GrabbedComponent` repasse à nul).
- **Nettoyage** : `ComposantNettoyage.cpp` recompilé (unity/adaptive build) mais **non modifié** par T002 (diff vide).

### Cas limites testés (par relecture du code)
- Pas de cible / rien tenu → `return` immédiat. Double appui clic droit → le 2ᵉ appel ne tient plus rien → `return`.
- Objet tiré en Pull → `bIsGrabbing` faux → rien, l'objet reste tiré.
- Visée qui change pendant la tenue → le profil lu est celui de l'objet **tenu**, pas de la cible visée (`ActorAllows` sur `HeldComponent->GetOwner()`).
- Masse ≈ 0 → `MaxLaunchSpeed` (pas de division par zéro). `LaunchImpulse = 0` → l'objet est seulement lâché (vitesse 0) ; acceptable.
- Pas de PlayerController → `GetViewPoint` renvoie la rotation de l'acteur (sorties toujours initialisées) ; impossible en pratique puisqu'un Grab demande déjà la visée.
- Composant visé détruit entre deux images → `IsValid(CurrentFocusComponent)` faux → retombe sur la racine.
- Pile de plus de 7 objets physiques sous l'objet tiré → aucun sol trouvé → objet lâché (borne assumée, documentée).

### Relecture code
- Lisible, commenté en français, identifiants sans accents, UTF-8. `UPROPERTY` avec `Category` et `ClampMin/ClampMax`. Pointeurs vérifiés avant usage. Pas de nouveau `Tick` ni `DrawDebug` (le debug existant reste derrière `bAfficherDebug`). Pas de doublon : `ActorAllows` factorise la lecture du profil, `CanFocusPerform` l'appelle. Respect de `decisions.md` (logique en C++, réglages éditables, visée unique de T001).
- Remarques (non bloquantes) :
  1. La CCD reste active sur un objet après son premier lancer (assumé et documenté §7 du journal ; coût négligeable aujourd'hui).
  2. Les trois choix (masse + plafond, CCD par code, profil sans Launch → reste tenu) sont proposés mais **pas encore inscrits** dans `doc/decisions.md` : à faire par l'orchestrateur.
- **Périmètre** : fichiers T002 modifiés = `InteractionComponent.h/.cpp`, `IA_Lancer` (nouveau), `IMC_Default`, `BP_ObjetGrab_Test1`, fichier d'acteur `…/3/6N/8BSGYZCK9EMQ2EXLYF0DEW.uasset` (nouveau), journaux, `notions.md`, fiche. `PersonnageNettoyeur.*` et `BP_FirstPersonCharacter` non touchés. Rien dans `Saved/`, `Intermediate/`, `Binaries/` n'est suivi. **Hors T002** (autre piste, ignorés) : `.gitattributes`, `.gitignore`, `CLAUDE.md`, `doc/CONVENTION.md`, `doc/decisions.md`, `.claude/agents/editeur-contenu.md`, `.claude/agents/qa.md`, `.claude/agents/rendu.md`, `.claude/agents/blender-*.md`, `doc/agents/blender-*.md`, `doc/art/`, `doc/taches/A001-A003`, PointLight `…/0/I6/`. À séparer au moment des commits.

### Assets (lus hors éditeur, comparaison des chaînes avec la version HEAD via Git LFS)
- `IA_Lancer` : nouvel asset `/Game/Input/Actions/IA_Lancer`, aucun `ValueType` explicite ⇒ valeur par défaut *Digital (bool)*. ✔
- `IMC_Default` : seules différences avec HEAD = `IA_Lancer` et `RightMouseButton` ajoutés ; `LeftMouseButton` (Grab) et toutes les autres touches toujours présentes. ✔ (Le nombre exact de 16 lignes n'est pas lisible ainsi : à confirmer à l'œil, point 1 de la procédure.)
- `BP_ObjetGrab_Test1` : `GeometryCollection*` et `/Script/GeometryCollectionEngine` disparus ; `StaticMeshComponent` `SM_Cube`, `bSimulatePhysics`, `PhysicsActor`, `bOverrideMass` + `MassInKgOverride`, `RelativeScale3D`, `EInteractionType::Grab` + `EInteractionType::Launch`, `Pull` disparu. ✔ Conforme à `editeur-contenu.md` §4.3.
- Fichier d'acteur `…/3/6N/…` : instance de `BP_ObjetGrab_Test1` dans `Lvl_FirstPerson`. ✔ Seul `BP_ObjetGrab_Test1` et ce fichier le référencent.

### Relecture documentation
- Les deux entrées T002 ont les 7 sections du modèle (`CONVENTION.md` §5), les sections vides écrites « Aucune ». Étapes éditeur écrites en clics (actions MCP retraduites, `editeur-contenu.md` §4.1 à 4.4), graphe décrit nœud par nœud (§4.5). Le *pourquoi* est expliqué pour chaque choix (masse, plafond, CCD, racine, 25 kg, emplacement). Les morceaux de code non triviaux (`KINDA_SMALL_NUMBER`, `AddIgnoredComponent`, `GetNameSafe`, formule de `OriginToBottom`) sont expliqués sur place.
- `CONVENTION.md` : la copie de travail (modifiée par la piste art) n'ajoute que des règles Blender et renomme la section 4 en « Étapes dans l'éditeur (Unreal et/ou Blender) » ; rien qui change l'évaluation de T002, qui est conforme aux deux versions.
- À mettre à jour après les tests : §5 des deux journaux disent « compilation non faite » (c'est maintenant fait, voir ci-dessus) ; le statut de la fiche.

### Procédure de validation pour le développeur
1. **Rouvrir l'éditeur** (le C++ vient d'être compilé : la DLL contient `Launch`). Vérifier dans l'**Output Log** (Fenêtre → Output Log) qu'il n'y a **aucune** ligne « InteractionComponent : aucun Physics Handle… » une fois en jeu (point 10 de la fiche).
2. Ouvrir `Content/Input/IMC_Default` : **16** lignes de *Mappings*, la dernière = `IA_Lancer` ← *Right Mouse Button* ; fermer sans enregistrer.
3. Ouvrir `Content/FirstPerson/Blueprints/BP_FirstPersonCharacter` → **Event Graph**, et monter le graphe de `editeur-contenu.md` T002 §4.5 :
   - a) clic droit → **EnhancedInputAction IA_Lancer** ; glisser **InteractionComponent** depuis le panneau Composants → **Get** ; depuis sa broche bleue, tirer un fil → **Launch**. Relier **Started** → exec de *Launch*, **Interaction Component** → **Target**.
   - b) bloc Grab existant (`Ctrl+F` « IA_Grab ») : **Completed** → exec d'entrée de **Release Grab** ; **Canceled** → la même broche ; vérifier que **Target** de *Release Grab* est relié à *Interaction Component*.
   - c) **Compile** (vert) puis **Save**. Faire une capture des deux blocs pour le dossier.
4. **Play** dans `Lvl_FirstPerson`. Descendre la rampe nord jusqu'à la caisse grise de 40 cm devant le muret.
5. Viser la caisse : prompt **« Clic gauche : Saisir »** (une seule ligne). *Attendu : oui.*
6. Tenir le clic gauche : la caisse est saisie et suit la caméra, le prompt **disparaît**. Relâcher le clic gauche : elle tombe, le prompt revient.
7. Saisir de nouveau, viser vers le haut au loin, **clic droit** : la caisse part dans la direction de la caméra (~6 m/s), retombe ; le prompt revient. Recommencer en visant à gauche, à droite, vers le bas.
8. Saisir, se placer face au muret à environ 1 m, **clic droit** : la caisse rebondit, **ne traverse pas**. Recommencer avec *InteractionComponent* (dans `BP_FirstPersonCharacter`) → *Interaction | Lancer* → `Max Launch Speed` = **5000** et `Launch Impulse` = **200000** (pour atteindre le plafond avec 25 kg), sans enregistrer : toujours pas de traversée. Remettre les valeurs (15000 / 1500) ou fermer sans enregistrer.
9. Sans rien tenir, **clic droit** plusieurs fois : rien ne se passe, aucune ligne rouge ou jaune dans l'Output Log.
10. Tirer `BP_ObjetPull_Test` (**R** maintenu) puis **clic droit** : rien, l'objet reste tiré. Continuer : il glisse au sol, est lâché au bord d'un vide, et monte sur une plateforme sans accrocher (dette n° 10 : il peut monter un peu plus lentement qu'avant).
11. Dette n° 9 : en tirant `BP_ObjetPull_Test`, le faire passer contre / au-dessus de la caisse `BP_ObjetGrab_Test1` : il **n'est plus lâché**, il la pousse.
12. Objet sans Launch : dans l'Organiseur, sélectionner l'instance `BP_ObjetGrab_Test1` → *InteractionProfile* → retirer *Lancer* (sans enregistrer), Play, saisir puis clic droit : la caisse **reste tenue**. Remettre / ne pas enregistrer.
13. Objet lourd (facultatif) : sur l'instance, *Mass (kg)* = 50 (sans enregistrer) → le lancer part nettement moins loin (300 cm/s).
14. Nettoyage : viser une `BP_PreuveNettoyable`, **E** ×4 → elle disparaît. Saut (Espace) impossible pendant qu'on tient un objet.
15. Si tout est conforme : cocher les critères restants de la fiche et donner ta validation à l'orchestrateur.

---

## QA T001 (extension) — Prompt par interaction — 2026-11-16
**Verdict :** ✅ Validé QA pour tout ce qui est vérifiable sans lancer le jeu (compilation, code, périmètre, doc). Le reparentage de `WBP_Interaction` et les tests en jeu **n'ont pas été faits** (éditeur fermé, pas de MCP) : ils restent au développeur (procédure en bas).

**Branche :** `tache/T001-unifier-visee` (pas de changement de branche, aucun commit). Partie principale déjà commitée (`914ffe9`) ; l'extension est non commitée.

### Compilation
- Éditeur fermé. Build.bat (`CLAUDE.md` §6) lancé depuis PowerShell.
- UBT : `Invalidating makefile for NettoyeurEditor (source file added)` → UHT `-WarningsAsErrors` : `7 files written`, aucun message → `[1/8]`–`[5/8] Compile` (`PersonnageNettoyeur.cpp`, `InteractionComponent.cpp`, `ComposantNettoyage.cpp`, `Module.Nettoyeur.gen.cpp`, `WidgetInteraction.cpp`) → `Link` `.lib` / `.dll` → `WriteMetadata`.
- **`Result: Succeeded`, 0 erreur, 0 ligne `warning`.** Les fichiers modifiés ont réellement été recompilés : la sortie prouve l'absence d'avertissement.

### Périmètre
- Modifiés / créés : `WidgetInteraction.h/.cpp` (nouveaux), `InteractionComponent.h/.cpp`, `ComposantNettoyage.h/.cpp` (tous autorisés) ; `InteractionProfileComponent` non touché (justifié au journal §2.2) ; `doc/agents/gameplay-cpp.md`, `doc/notions.md`, la fiche.
- Assets modifiés **par le développeur** dans l'éditeur (B1), comparés à la version `HEAD` (récupérée par `git lfs smudge`, puis comparaison des noms lisibles contenus dans les deux fichiers) :
  - `BP_FirstPersonCharacter.uasset` : disparition de `LineTraceSingle`, `PrintString`, `BreakHitResult`, `DrawDebugType`… (bloc mort supprimé) ; `NewVar` → **`WidgetInteraction`** (variable renommée). Aucun autre nom ajouté. `DefinirWidgetInteraction`, `MettreAJourProgression`, `AfficherMissionTerminee`, `K2Node_CreateWidget` toujours présents.
  - `WBP_Interaction.uasset` : seuls `PrintString`, `Conv_DoubleToString` et leurs pins ont disparu (les 2 Print String de `MettreAJourProgression`). Parent encore `UserWidget` (non reparenté, attendu).
- Non suivi : `Content/__ExternalActors__/.../I6/M8RK6YKWGV6E3Q8C5UHZX3.uasset` (daté 2026-11-11), **antérieur à T001**, déjà signalé : ne pas l'inclure dans le commit sans décision.
- Rien sous `Saved/`, `Intermediate/`, `Binaries/`. Encodage des nouveaux fichiers : UTF-8 sans BOM, CRLF.

### Critères d'acceptation
- [ ] Preuve → « E : Nettoyer » + barre ; `BP_ObjetPull_Test` → « R (maintenir) : Tirer » sans barre ; rien → rien — **test en jeu à faire.** Lecture : `RafraichirPrompt` parcourt la table et garde les entrées non vides pour lesquelles `CanFocusPerform` est vrai (faux si rien visé / sans profil) ; barre visible seulement si `CanFocusPerform(Wipe)` et rien tenu.
- [ ] Plusieurs interactions → une ligne chacune — **test en jeu à faire.** Lecture : `FText::Join("\n", Lignes)`, ordre = ordre de la TMap.
- [ ] Textes modifiables dans les Class Defaults sans recompiler — **à faire après reparentage.** Lecture : `TextesInteractions` est `EditAnywhere`, valeurs par défaut au constructeur.
- [ ] Prompt masqué pendant le Pull — **test en jeu à faire.** Lecture : `OnHoldChanged(true)` à la fin de `Pull()` / `Grab()` seulement si `IsHoldingObject()` ; `RafraichirPrompt` ne produit aucune ligne et masque la barre si un objet est tenu ; un changement de cible pendant la tenue repasse par le même test.
- [ ] Nettoyage, barre et « mission terminée » inchangés — **test en jeu à faire.** Lecture : `TenterNettoyage`, `PeutNettoyerCible`, `OnProgressionNettoyageChanged.Broadcast` identiques au commit validé (seuls le prompt, l'abonnement et `EndPlay` ont été retirés) ; `MettreAJourProgression` ne fait plus que *Set Percent* ; `AfficherMissionTerminee` touche `TexteObjectifMission`, pas les éléments liés.
- [x] Plus aucun `ProcessEvent` / `FindFunction` dans `Source/` — `grep -rn "ProcessEvent\|FindFunction" Source/` : aucun résultat ; `MettreAJourTexteInteraction` : aucun résultat.
- [x] Compilation sans avertissement ; journal et notions à jour — voir Compilation et Relecture documentation.

### Non-régression (lecture du code, pas de jeu)
- **Saisie Pull/Grab :** seules lignes ajoutées = les `Broadcast`. Dans `ReleaseGrab`, la condition `PhysicsHandle && PhysicsHandle->GrabbedComponent` est remplacée par `IsHoldingObject()`, qui teste exactement la même chose → relâchement identique ; `bIsGrabbing = false` toujours exécuté. Aucun autre `ReleaseComponent` dans `Source/` (les 3 relâchements automatiques de `UpdateHold` passent par `ReleaseGrab`, donc diffusent `false`).
- **Nettoyage :** voir critère ci-dessus. `UComposantNettoyage` garde `BeginPlay` (recherche de l'`InteractionComponent`), toujours sans Tick.
- **Blueprint du personnage :** `DefinirWidgetInteraction(UUserWidget*)` garde la même signature et reste `BlueprintCallable` → le nœud ne casse pas. `MettreAJourTexteInteraction` était `UFUNCTION()` non `BlueprintCallable` : aucun nœud Blueprint ne pouvait l'appeler, sa suppression ne casse rien.

### Cas limites testés (lecture du code)
- **Rien visé** : `CanFocusPerform` faux → texte `Hidden`, barre `Hidden`. Au démarrage, `RafraichirPrompt()` est appelé dans `NativeConstruct` → état initial correct.
- **Preuve détruite à 1.0** : le sweep ne la touche plus → `SetCurrentFocus(nullptr)` → `OnFocusChanged` → prompt et barre masqués (comme avant).
- **Pas de pawn / pas d'InteractionComponent** : avertissement dans l'Output Log, pointeur nul testé partout, pas de plantage.
- **Designer de l'éditeur** : `NativeConstruct` n'est appelé qu'en jeu (moteur, `UserWidget.cpp`, branche hors design time) → pas d'avertissement parasite à l'ouverture du WBP.
- **Double construction** (Remove from Parent puis Add to Viewport) : `AddUniqueDynamic` + `RemoveDynamic` au `NativeDestruct` → ni double abonnement, ni abonné mort.
- **Relâcher R sans rien tenir** : `bWasHolding` faux → pas de diffusion.
- **Objet tenu détruit sans `ReleaseGrab`** : `OnHoldChanged(false)` ne part pas ; le prompt reste masqué jusqu'au prochain changement de cible (signalé par l'agent, aucun cas actuel). Accepté.

### Relecture code
- **BindWidget ↔ WBP réel :** `TexteInteraction` (`UTextBlock`) et `BarProgression` (`UProgressBar`). Les deux noms et les classes `TextBlock` / `ProgressBar` figurent dans `WBP_Interaction.uasset` ; types confirmés par la Hiérarchie décrite dans `doc/agents/editeur-contenu.md` (T000 point 4.b : `TexteInteraction` = Text, `BarProgression` = ProgressBar). Le reparentage ne devrait pas lever d'erreur de *widget binding* — **non prouvé tant que le WBP n'a pas été compilé dans l'éditeur.**
- **Conflit avec la logique Blueprint restante (point de vigilance) :** dans le WBP, le seul `SetVisibility` restant est dans `AfficherInteraction`, qui n'est plus appelée nulle part (ni en C++, ni dans `BP_FirstPersonCharacter` : aucune référence dans ses noms). `MettreAJourProgression` = *Set Percent* uniquement ; `AfficherMissionTerminee` = *Set Text* sur `TexteObjectifMission`. **Aucun conflit** de visibilité avec le C++. La suppression d'`AfficherInteraction` est conseillée pour qu'aucun appel futur n'entre en concurrence.
- Pointeurs : `ComposantInteraction`, `TexteInteraction`, `BarProgression` testés avant usage ; `ComposantInteraction` en `UPROPERTY(Transient)`, remis à `nullptr` au `NativeDestruct`. Fonctions abonnées en `UFUNCTION()` avec la bonne signature (vérifiée par UHT).
- Pas de Tick / `NativeTick`, pas de `DrawDebug` ajouté, pas de doublon (réutilise `CanFocusPerform`, pas de deuxième lecture du profil). `UWidgetInteraction` n'entre pas en collision avec le moteur (`UWidgetInteractionComponent` est un autre nom).
- Remarques mineures (non bloquantes) :
  1. Texte et barre passent en `Visible`, comme l'ancienne fonction Blueprint. `HitTestInvisible` (« Non testable (soi et enfants) ») serait plus juste pour un prompt qui ne doit jamais capter la souris ; sans effet aujourd'hui (souris capturée par le jeu).
  2. Le remplissage de la barre (`Percent`) n'est jamais remis à 0 : avec une **deuxième** preuve, la barre s'afficherait pleine jusqu'au premier essuyage. Comportement **antérieur** (pas une régression), une seule preuve aujourd'hui ; à prévoir quand il y en aura plusieurs.
  3. La variable du personnage a été renommée `WidgetInteraction` : même nom que la classe C++ `UWidgetInteraction` et que la propriété `UComposantNettoyage::WidgetInteraction`. Aucun conflit technique (variable ≠ classe, objets différents), mais un nom comme `WidgetPrompt` serait plus lisible pour un débutant.

### Relecture documentation
- Entrée « T001 (extension) » de `doc/agents/gameplay-cpp.md` : sections 1 à 7 de `doc/CONVENTION.md` présentes ; le pourquoi est expliqué (§2.1) ; le tableau « qui fait quoi » (§2.7) est clair ; étapes éditeur écrites comme un humain (menus FR + EN, clics, cas d'erreur de binding et conduite à tenir) ; procédure de test en jeu détaillée ; compilation honnêtement notée « non faite ».
- 4 notions nouvelles (BindWidget, NativeConstruct / NativeDestruct / AddUniqueDynamic, TMap, FText / LOCTEXT) : expliquées dans le journal **et** ajoutées à `doc/notions.md` ; les 13 ancres `#notion-…` citées dans l'entrée existent.
- Seule mention devenue obsolète : journal §5 « Compilation : non faite » (état au moment de l'agent ; la compilation est faite, voir ce rapport).
- Un débutant peut refaire la tâche en lisant le journal : oui.

### Procédure de validation pour le développeur
**0. Commit de sécurité** (avant reparentage, `CLAUDE.md` §7) : code de l'extension + `BP_FirstPersonCharacter.uasset` + `WBP_Interaction.uasset` (nettoyage B1) + doc. Ne pas inclure `Content/__ExternalActors__/.../I6/`.

**1. Reparenter `WBP_Interaction`**
1. Ouvre l'éditeur (le C++ est déjà compilé). **Navigateur de contenu** → `Content/Nettoyeur` → double-clic sur **WBP_Interaction**.
2. Onglet **Graphe (Graph)** (en haut à droite) → bouton **Paramètres de classe (Class Settings)** dans la barre d'outils.
3. **Détails** → **Options de classe** → **Classe parente (Parent Class)** → ouvre la liste → tape `WidgetInteraction` → sélectionne-la → confirme.
4. **Compiler**. Attendu : coche verte. Si « A required widget binding … was not found » apparaît : note le message exact et signale-le.
5. Si des nœuds *Get TexteInteraction* / *Get BarProgression* sont en jaune : clic droit dans le vide du graphe → **Actualiser tous les nœuds (Refresh All Nodes)** → Compiler.

**2. Supprimer `AfficherInteraction` et vérifier les textes**
1. **Mon Blueprint** → *Fonctions* → clic droit sur **AfficherInteraction** → **Rechercher les références** : aucun appel attendu.
2. Clic droit → **Supprimer**. Ne touche pas à `MettreAJourProgression` ni à `AfficherMissionTerminee`.
3. **Valeurs par défaut de classe (Class Defaults)** → **Interaction | Prompt** → *Textes Interactions* : Essuyer → « E : Nettoyer », Tirer → « R (maintenir) : Tirer », Saisir → « Clic gauche : Saisir ».
4. **Compiler** → **Enregistrer**. Ouvre `BP_FirstPersonCharacter` → **Compiler** : aucune erreur attendue (nœud *Definir Widget Interaction* intact).

**3. Test en jeu** (`Lvl_FirstPerson`, **Jouer**) — note OK / pas OK pour chaque point :
1. Ne rien viser → aucun texte au centre, aucune barre.
2. Viser `BP_PreuveNettoyable` → « E : Nettoyer » + barre.
3. Viser `BP_ObjetPull_Test` → « R (maintenir) : Tirer », **pas** de barre.
4. Maintenir R sur `BP_ObjetPull_Test` → l'objet suit, le prompt **disparaît** ; relâcher → il revient tant que tu vises l'objet.
5. Revenir sur la preuve, appuyer 4 fois sur E → barre 0.25 → 0.5 → 0.75 → 1.0, la preuve disparaît, prompt et barre masqués, « MISSION TERMINÉE ! » en haut à gauche.
6. Aucun texte bleu de debug à l'écran pendant tout le test (Print String supprimés).
7. Quitte le Play → Class Defaults de `WBP_Interaction` → change « E : Nettoyer » en « E : Essuyer » → Compiler → Play → le nouveau texte s'affiche sur la preuve, sans recompiler le C++. Remets le texte d'origine → Compiler → Enregistrer.
8. (Facultatif, sans enregistrer) Instance `BP_ObjetPull_Test` → composant `ProfilInteractions` → *Allowed Interactions* → **+** *Saisir* → Play → deux lignes « R (maintenir) : Tirer » / « Clic gauche : Saisir ». Annule ensuite (Ctrl+Z).
9. **Journal de sortie (Output Log)** : aucun « WidgetInteraction : aucun InteractionComponent… ».

Si tout est OK : cocher les 5 critères restants de l'extension dans la fiche, puis commit `T001: reparenter WBP_Interaction vers UWidgetInteraction`.

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
