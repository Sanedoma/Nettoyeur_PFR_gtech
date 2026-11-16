# Index des notions déjà expliquées

> Avant d'expliquer une notion, chaque agent vérifie ici. Si elle y est : un lien suffit. Sinon : il l'explique dans son journal et l'ajoute ici.
> Format : `- **Nom** — une ligne de résumé → [lien vers l'explication](agents/<agent>.md#ancre)`

## C++ Unreal
- **Réflexion, Unreal Header Tool (UHT), `.generated.h`** — UHT lit les macros des `.h` et génère le code qui expose classes et variables ; ajout d'`UPROPERTY`/`UFUNCTION`/`UMETA` = fermer l'éditeur et recompiler → [explication](agents/gameplay-cpp.md#notion-reflexion-uht)
- **`UCLASS`, `UPROPERTY`, `UFUNCTION`** — annotations lues par UHT ; spécificateurs EditAnywhere / VisibleAnywhere / BlueprintReadOnly / BlueprintCallable / BlueprintPure / ClampMin ; BlueprintSpawnableComponent → [explication](agents/gameplay-cpp.md#notion-uclass-uproperty-ufunction)
- **`GENERATED_BODY()` et `NETTOYEUR_API`** — insertion du code généré ; export de la classe hors du module → [explication](agents/gameplay-cpp.md#notion-generated-body)
- **Préfixes `A`, `U`, `F`, `E`, `T`** — Actor, UObject, struct, enum, template → [explication](agents/gameplay-cpp.md#notion-prefixes)
- **`TObjectPtr<T>` et ramasse-miettes** — pointeur membre vers un UObject dans une UPROPERTY ; le GC ne voit que les UPROPERTY → [explication](agents/gameplay-cpp.md#notion-tobjectptr)
- **Déclaration anticipée** — `class UType;` dans le `.h`, `#include` dans le `.cpp` → [explication](agents/gameplay-cpp.md#notion-declaration-anticipee)
- **Module et `.Build.cs`** — dépendances du module `Nettoyeur` vers les modules du moteur (Engine, EnhancedInput, UMG…) → [explication](agents/gameplay-cpp.md#notion-modules-build-cs)
- **`UENUM`, `enum class`, `UMETA(DisplayName)`** — liste fixe de valeurs exposée aux Blueprints ; texte affiché ≠ nom dans le code → [explication](agents/gameplay-cpp.md#notion-uenum)
- **Actor Component (`UActorComponent` vs `USceneComponent`)** — brique logique sans position vs composant placé ; `GetOwner`, `FindComponentByClass` → [explication](agents/gameplay-cpp.md#notion-actor-component)
- **`TArray`** — tableau dynamique d'Unreal (`Add`, `Contains`, boucle) → [explication](agents/gameplay-cpp.md#notion-tarray)
- **`CreateDefaultSubobject` et constructeur (CDO)** — composants par défaut créés dans le constructeur ; le constructeur n'est pas le début de partie → [explication](agents/gameplay-cpp.md#notion-createdefaultsubobject)
- **`TEXT()`, `FName`, `NAME_None`** — chaînes littérales du moteur et identifiants rapides → [explication](agents/gameplay-cpp.md#notion-text-fname)
- **`virtual`, `override`, `Super::`** — redéfinir une fonction du parent et appeler sa version → [explication](agents/gameplay-cpp.md#notion-virtual-override-super)
- **`BlueprintNativeEvent` et `_Implementation`** — fonction C++ redéfinissable en Blueprint (ex. `CanJumpInternal`) → [explication](agents/gameplay-cpp.md#notion-blueprintnativeevent)
- **`BeginPlay()`** — initialisation de jeu, une fois à l'entrée en jeu → [explication](agents/gameplay-cpp.md#notion-beginplay)
- **Tick (`Tick`, `TickComponent`, `DeltaTime`, `bCanEverTick`)** — fonction appelée à chaque image ; pas de Tick si un événement suffit → [explication](agents/gameplay-cpp.md#notion-tick)
- **`Cast<T>()`** — conversion vérifiée d'un objet vers un type ; `nullptr` si ce n'est pas ce type → [explication](agents/gameplay-cpp.md#notion-cast)
- **Line Trace vs Sweep (`FHitResult`, `FCollisionQueryParams`, Single / Multi)** — rayon fin ou forme glissée ; Multi s'arrête au premier bloquant → [explication](agents/gameplay-cpp.md#notion-line-trace-sweep)
- **Canaux de collision (`ECC_Visibility`)** — chaque trace interroge un canal ; Block / Overlap / Ignore par objet → [explication](agents/gameplay-cpp.md#notion-canaux-collision)
- **Physics Handle (`UPhysicsHandleComponent`)** — tenir un objet simulé par un ressort : Grab…, SetTarget…, Release → [explication](agents/gameplay-cpp.md#notion-physics-handle)
- **Interpolation `FMath::VInterpTo` / `FInterpTo`** — rapprocher une valeur de sa cible d'une fraction par image, indépendamment des FPS → [explication](agents/gameplay-cpp.md#notion-vinterpto)
- **Point de vue du joueur (`GetPlayerViewPoint`, `FVector`, `FRotator`)** — position/orientation de la caméra du joueur ; Pitch / Yaw / Roll → [explication](agents/gameplay-cpp.md#notion-player-view-point)
- **Delegate dynamique multicast (`DECLARE_DYNAMIC_MULTICAST_DELEGATE_...`, `BlueprintAssignable`, `Broadcast`)** — événement diffusé par le C++ et écouté par plusieurs abonnés, dont les Blueprints → [explication](agents/gameplay-cpp.md#notion-delegate-dynamique-multicast)
- **`FindFunction` + `ProcessEvent`** — appeler une fonction (Blueprint) par son nom via la réflexion ; fragile si la signature change → [explication](agents/gameplay-cpp.md#notion-processevent)
- **`UE_LOG` et `DrawDebugLine`** — écrire dans le journal ; dessiner un trait de debug (derrière un `bDebug`) → [explication](agents/gameplay-cpp.md#notion-ue-log-drawdebug)
- **Encodage des sources (UTF-8 vs Windows-1252, BOM)** — pourquoi un accent s'affiche `�` et comment convertir → [explication](agents/gameplay-cpp.md#notion-encodage-utf8)
- **Paramètre de sortie par référence (`FVector& OutX`)** — rendre plusieurs valeurs en écrivant dans les variables de l'appelant → [explication](agents/gameplay-cpp.md#notion-parametre-sortie-reference)
- **`UPROPERTY(Transient)`** — variable vue par le ramasse-miettes mais jamais sauvegardée (pointeur trouvé en jeu, cache) → [explication](agents/gameplay-cpp.md#notion-transient)
- **Abonnement C++ à un delegate : `AddDynamic` / `RemoveDynamic`, `EndPlay`** — brancher au BeginPlay, débrancher à l'EndPlay ; la fonction abonnée est une `UFUNCTION` → [explication](agents/gameplay-cpp.md#notion-adddynamic-endplay)
- **Signature d'une `UFunction` (`FProperty`, `CastField`, `NumParms`, `ParmsSize`, `FMemory_Alloca`)** — vérifier les paramètres d'une fonction Blueprint avant `ProcessEvent` → [explication](agents/gameplay-cpp.md#notion-signature-ufunction)
- **`TMap<Clé, Valeur>`** — dictionnaire clé → valeur (`Add`, `Find`, boucle sur `TPair`) ; éditable dans les Details, ordre = ordre d'ajout → [explication](agents/gameplay-cpp.md#notion-tmap)
- **`FText`, `FString`, `LOCTEXT`** — texte affiché au joueur (traduisible) vs chaîne du code ; `LOCTEXT_NAMESPACE`, `FText::Join` → [explication](agents/gameplay-cpp.md#notion-ftext-loctext)
- **Fonction membre `static`** — fonction de classe sans `this`, ne dépend que de ses paramètres → [explication](agents/gameplay-cpp.md#notion-static-membre)
- **`constexpr`** — constante connue à la compilation, remplace un « nombre magique » → [explication](agents/gameplay-cpp.md#notion-constexpr)
- **`IsValid(Objet)`** — non nul **et** pas en cours de destruction ; pour un pointeur gardé d'une image à l'autre → [explication](agents/gameplay-cpp.md#notion-isvalid)
- **`Bounds` (`FBoxSphereBounds`)** — boîte englobante alignée monde : `Origin` (centre), `BoxExtent` (demi-taille) ; centre ≠ pivot → [explication](agents/gameplay-cpp.md#notion-bounds)

## Moteur et architecture (Actors, Components, Subsystems…)
- **Source unique de vérité ; piloté par événement vs sondage** — calculer une donnée à un seul endroit, ne prévenir qu'au changement → [explication](agents/gameplay-cpp.md#notion-source-unique-evenement)
- **Panneau Composants, composant hérité ou ajouté** — arbre des composants de scène, composants logiques, composant racine → [explication](agents/editeur-contenu.md#notion-composants-herites)
- **Game Mode (Default Pawn, PlayerController)** — l'acteur qui choisit les classes du joueur et les règles → [explication](agents/editeur-contenu.md#notion-game-mode)
- **Bind Event to + Custom Event** — s'abonner en Blueprint à un événement diffusé par le C++ → [explication](agents/editeur-contenu.md#notion-bind-event)

## Physique
- **Simulate Physics et Collision Presets** — objet piloté par la physique ; BlockAllDynamic vs PhysicsActor → [explication](agents/editeur-contenu.md#notion-collision-physique)
- **Impulsion, masse et `AddImpulse` (`bVelChange`)** — coup instantané vs force continue ; à impulsion égale un objet lourd part moins vite ; kg, cm/s → [explication](agents/gameplay-cpp.md#notion-impulsion-addimpulse)
- **CCD (Continuous Collision Detection)** — vérifier le trajet entre deux pas de physique pour qu'un objet rapide ne traverse pas un mur ; `SetUseCCD` / *Use CCD* → [explication](agents/gameplay-cpp.md#notion-ccd)
- **Masse d'un corps physique (calculée ou imposée, *Mass (kg)*)** — masse déduite du volume, ou imposée par la case *Mass (kg)* pour un comportement prévisible → [explication](agents/editeur-contenu.md#notion-masse-override)

## Éditeur (Content Browser, Blueprints, Details…)
- **Blueprint enfant (classe parente)** — un Blueprint hérite de sa classe parente C++ ; reparentage → [explication](agents/editeur-contenu.md#notion-blueprint-enfant)
- **Class Defaults** — valeurs de départ des variables de l'acteur, différent des réglages d'un composant → [explication](agents/editeur-contenu.md#notion-class-defaults)
- **Event Graph, broches d'exécution et de données** — ordre d'exécution, nœud non relié = jamais exécuté, nœuds purs → [explication](agents/editeur-contenu.md#notion-event-graph)
- **Maps & Modes et World Settings (GameMode Override)** — carte/Game Mode globaux vs par niveau → [explication](agents/editeur-contenu.md#notion-maps-modes-world-settings)
- **Changer le composant racine d'un Blueprint (Make New Root)** — la racine porte la position de l'acteur ; un mesh qui simule doit être racine → [explication](agents/editeur-contenu.md#notion-changer-racine)

## Entrées (Enhanced Input)
- **Input Action (IA) et Input Mapping Context (IMC)** — l'intention vs les touches ; Add Mapping Context → [explication](agents/editeur-contenu.md#notion-enhanced-input)
- **Sorties d'un nœud d'Input Action** — Started, Triggered, Ongoing, Completed, Canceled → [explication](agents/editeur-contenu.md#notion-evenements-input)
- **Modifiers Swizzle, Negate, Dead Zone** — transformer la valeur d'une touche pour un axe 2D → [explication](agents/editeur-contenu.md#notion-modifiers)

## Interface (UMG)
- **Widget Blueprint : Concepteur, Graphe, Create Widget, Add to Viewport** — créer et afficher une interface ; Is Variable ; visibilité → [explication](agents/editeur-contenu.md#notion-widget-blueprint)
- **`meta = (BindWidget)` / `BindWidgetOptional`** — pointeur C++ rempli avec l'élément du Concepteur de même nom ; obligatoire ou facultatif → [explication](agents/gameplay-cpp.md#notion-bindwidget)
- **`NativeConstruct` / `NativeDestruct`, `GetOwningPlayerPawn`, `AddUniqueDynamic`** — cycle de vie d'un widget en C++ (équivalent de BeginPlay / EndPlay) → [explication](agents/gameplay-cpp.md#notion-nativeconstruct)

## Rendu (matériaux, decals, Niagara…)
_(vide)_

## Outils et workflow (compilation, Live Coding, Git LFS…)
- **Copier des nœuds en texte (T3D)** — Ctrl+C sur des nœuds pour relever un graphe et ses connexions → [explication](agents/editeur-contenu.md#notion-copier-noeuds-texte)
- **One File Per Actor (`__ExternalActors__`)** — un fichier par acteur placé dans le niveau → [explication](agents/editeur-contenu.md#notion-ofpa)
