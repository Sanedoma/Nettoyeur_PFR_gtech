# Journal — gameplay-cpp

> Entrées selon `doc/CONVENTION.md`, la plus récente en haut.

---

## T001 — Unifier la visée et faire passer le nettoyage par le profil d'interaction — 2026-11-13

**Objectif :** une seule source de vérité pour « ce que le joueur vise » (`UInteractionComponent`). Le nettoyage n'a plus son propre rayon : il lit cette cible et vérifie que son profil autorise `Wipe`. Le comportement en jeu reste celui d'avant (prompt, 4 essuyages, barre, disparition).
**Étape du GDD :** §11 étape 2 ; §6.1 « interactions indépendantes et assignables ».
**Branche :** `tache/T001-unifier-visee`

### 1. Fichiers créés / modifiés
- `Source/Nettoyeur/InteractionComponent.h` — modifié — ajoute le delegate `FOnFocusChanged` / `OnFocusChanged`, `GetCurrentFocus()`, `CanFocusPerform()`, `bAfficherDebug` ; remplace `GetAimDirection()` (inutilisé) par `GetViewPoint()` ; renomme les catégories (`Interaction|Visee`, `Interaction|Tenir`, `Interaction|Debug`).
- `Source/Nettoyeur/InteractionComponent.cpp` — modifié — le sweep de visée devient **le seul** du projet ; il ne prévient les abonnés que quand la cible change ; le calcul du point de vue n'est plus écrit qu'une fois ; Pull/Grab utilisent `CanFocusPerform`. Logique de Pull/Grab/Hold inchangée.
- `Source/Nettoyeur/ComposantNettoyage.h` — modifié — retire `Personnage`, `CameraJoueur`, `TickComponent` ; ajoute `bAfficherDebug`, `EndPlay`, le pointeur privé `ComposantInteraction`, `GererChangementCible`, `PeutNettoyerCible` ; `WidgetInteraction` passe en `BlueprintReadOnly`.
- `Source/Nettoyeur/ComposantNettoyage.cpp` — modifié — plus de Line Trace ni de Tick ; nettoyage décidé par le profil ; prompt mis à jour sur événement ; `ProcessEvent` protégé par une vérification de signature ; log de progression derrière `bAfficherDebug`.
- `Source/Nettoyeur/PersonnageNettoyeur.h` / `.cpp` — modifiés — suppression de `TenterNettoyage()` (vide) et des `BeginPlay` / `Tick` / `SetupPlayerInputComponent` vides du modèle ; include inutile retiré.
- `doc/agents/gameplay-cpp.md` — cette entrée. `doc/notions.md` — 5 notions ajoutées. `doc/taches/T001-unifier-visee.md` — statut « Prêt pour QA ».

Dette traitée (numéros de §2.9 de T000) : **1, 2, 4, 5 (réduite), 7, 13, 16**, et **15 en partie** (voir §7 pour ce qui reste).

### 2. Implémentation du code

#### 2.1 Pourquoi supprimer le double trace
Avant, **trois** rayons pouvaient partir à chaque image ou à chaque appui (T000 §2.8) : le sweep sphère de 10 cm de `UInteractionComponent` (pour Pull/Grab), le Line Trace du Tick de `UComposantNettoyage` (pour le prompt) et un 3ᵉ Line Trace dans `TenterNettoyage` (pour essuyer). Problèmes :
- **Deux vérités possibles.** Les deux traces n'avaient ni la même origine (caméra trouvée par `FindComponentByClass` vs `GetPlayerViewPoint`), ni la même forme (rayon fin vs sphère), ni la même portée (300 en dur vs `TraceDistance` réglable). Au bord d'un objet, l'un pouvait le toucher et l'autre non : le joueur voit un prompt mais E ne fait rien, ou l'inverse.
- **Code copié-collé** : toute correction devait être faite à 2 ou 3 endroits.
- **Coût** : chaque trace interroge la physique ; c'est négligeable avec 2 traces, mais le GDD prévoit d'ajouter des interactions (ouvrir, cacher…) et chacune aurait ajouté la sienne.

Maintenant : **un seul sweep par image** (`UInteractionComponent::UpdateFocus`) ; les autres systèmes **lisent** son résultat (`GetCurrentFocus()`) ou **écoutent** son changement (`OnFocusChanged`). Vérification : `grep -n "Trace\|Sweep" Source/Nettoyeur/*.cpp` ne trouve plus que le sweep de visée et la trace **vers le sol** du Pull (qui n'est pas une visée : elle cherche le sol sous l'objet tenu, et seulement quand on tire un objet).

<a id="notion-source-unique-evenement"></a>
> **Notion nouvelle — Source unique de vérité et « piloté par événement » plutôt que « sondage »**
> Une **source unique de vérité** : une donnée (ici « ce que le joueur vise ») est calculée à **un seul endroit** ; tous les autres la lisent au lieu de la recalculer. Ainsi elle ne peut pas être différente d'un système à l'autre.
> **Sondage (polling)** : chaque image, on recalcule et on renvoie l'information, même si rien n'a changé (l'ancien prompt était renvoyé au widget 60 fois par seconde). **Piloté par événement** : l'émetteur prévient **seulement quand ça change** (`OnFocusChanged`), les abonnés réagissent à ce moment-là. Ici, le sweep reste fait à chaque image (il faut bien savoir si la cible a changé), mais **le reste** (prompt, appel du widget) n'est fait qu'au changement. C'est la règle du projet : « pas de Tick si un événement suffit ».

#### 2.2 `InteractionComponent.h` / `.cpp`

**a) Le delegate `OnFocusChanged`.** Déclaré en tête du `.h` :
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, AActor*, NewFocus);
...
UPROPERTY(BlueprintAssignable, Category = "Interaction|Visee")
FOnFocusChanged OnFocusChanged;
```
Même mécanisme que `OnProgressionNettoyageChanged` (voir [Delegate dynamique multicast](#notion-delegate-dynamique-multicast)). Un seul paramètre, la nouvelle cible (`nullptr` = on ne vise plus rien). Je n'ai pas ajouté l'ancienne cible : aucun abonné n'en a besoin aujourd'hui. `BlueprintAssignable` permet à un futur widget ou Blueprint de s'y abonner (nœud *Bind Event to On Focus Changed*), comme pour la progression.

**b) Ne prévenir qu'au changement — `SetCurrentFocus`.**
```cpp
void UInteractionComponent::SetCurrentFocus(AActor* NewFocus)
{
    if (NewFocus == CurrentFocus) { return; }   // même cible qu'à l'image d'avant : rien à dire
    CurrentFocus = NewFocus;
    OnFocusChanged.Broadcast(NewFocus);
}
```
`UpdateFocus()` (appelé chaque image par le Tick) fait le sweep puis appelle `SetCurrentFocus(bHit ? Hit.GetActor() : nullptr)`. Le `Broadcast` n'a donc lieu que quand on passe d'un acteur à un autre (ou à rien). Cas important pour le nettoyage : au 4ᵉ essuyage, la preuve se cache et **coupe sa collision** ; à l'image suivante le sweep ne la touche plus → la cible change (vers le mur derrière, ou rien) → `OnFocusChanged` → le prompt se masque. Même résultat qu'avant, sans calcul permanent.

**c) Lire la cible — `GetCurrentFocus()` et `CanFocusPerform()`.** `GetCurrentFocus()` (`BlueprintPure`) renvoie l'acteur visé. `CanFocusPerform(EInteractionType)` regroupe le test que Pull et Grab faisaient chacun de leur côté (« la cible a-t-elle un `UInteractionProfileComponent` qui autorise cette interaction ? ») ; le nettoyage l'utilise aussi pour `Wipe`. Trois utilisateurs concrets : ce n'est pas une abstraction prématurée. Pull et Grab vérifient toujours ensuite que la racine simule la physique : leur comportement est identique.

**d) `GetViewPoint()` remplace `GetAimDirection()` et 4 copies (dette 7).** `GetAimDirection()` n'était appelée nulle part (grep) : supprimée. Le calcul « point de vue du joueur, sinon position de l'acteur » était recopié dans `UpdateFocus`, `Grab` et deux fois dans `UpdateHold`. Il est maintenant dans une seule fonction privée :
```cpp
bool UInteractionComponent::GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
    // valeur de secours : position / rotation de l'acteur propriétaire
    OutLocation = OwnerActor ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
    OutRotation = OwnerActor ? OwnerActor->GetActorRotation() : FRotator::ZeroRotator;
    ... // si un PlayerController possède le pawn : GetPlayerViewPoint, return true
    return false;
}
```
Le `bool` renvoyé permet à `UpdateFocus` de garder son ancien comportement (pas de PlayerController → on ne vise rien), alors que `Grab` / `UpdateHold` utilisent la valeur de secours comme avant.

<a id="notion-parametre-sortie-reference"></a>
> **Notion nouvelle — Paramètre de sortie par référence (`FVector& OutLocation`)**
> Une fonction C++ ne renvoie qu'**une** valeur avec `return`. Pour en rendre plusieurs, on passe des variables **par référence** (le `&` après le type) : la fonction écrit directement dans la variable de l'appelant. Convention Unreal : ces paramètres commencent par `Out`. Exemple : `FVector Pos; FRotator Rot; GetViewPoint(Pos, Rot);` → après l'appel, `Pos` et `Rot` sont remplis, et la valeur de retour (`bool`) dit si c'est la vraie caméra. `GetPlayerViewPoint(OutLocation, OutRotation)` du moteur fonctionne pareil. Sans `&`, la fonction travaillerait sur une **copie** et l'appelant ne verrait rien. Un `const FVector&` (référence constante) sert, lui, à éviter une copie sans permettre la modification.

**e) Debug de la visée (`bAfficherDebug`, faux par défaut).** Quand il est coché, `UpdateFocus` dessine chaque image le trajet du sweep : **vert** jusqu'au point touché + une petite sphère de 10 cm (la taille réelle du sweep), **rouge** jusqu'à la portée maximale si rien n'est touché. Durée 0 = une image, donc le trait suit la caméra (voir [`DrawDebugLine`](#notion-ue-log-drawdebug)). C'est le remplaçant du trait vert permanent qu'affichait `UComposantNettoyage` : il est dessiné là où la trace est faite.

**f) Catégories (dette 15).** Les réglages partagés par Pull et Grab n'étaient rangés que sous « Interaction | Pull ». Dans **Détails** de `InteractionComponent`, ils apparaissent désormais sous : **Interaction | Visee** (`Trace Distance`), **Interaction | Tenir** (`Hold Distance`, `Hold Interp Speed`), **Interaction | Debug** (`Afficher Debug`). Les nœuds `Release Grab` et `Is Holding Object` passent dans la catégorie *Interaction | Tenir* du menu des nœuds. Changer une `Category` ne change **que l'affichage** : les valeurs réglées dans `BP_FirstPersonCharacter` et les nœuds déjà posés dans les graphes restent valides.

#### 2.3 `ComposantNettoyage.h` / `.cpp`

**a) Trouver la visée une fois, au BeginPlay (dette 13).** Avant, chaque image : `Cast<ACharacter>(GetOwner())` + `FindComponentByClass<UCameraComponent>()`, et le composant exigeait un `ACharacter` avec une caméra. Maintenant :
```cpp
ComposantInteraction = Proprietaire ? Proprietaire->FindComponentByClass<UInteractionComponent>() : nullptr;
if (!ComposantInteraction) { UE_LOG(LogTemp, Warning, TEXT("... nettoyage inactif.")); return; }
ComposantInteraction->OnFocusChanged.AddDynamic(this, &UComposantNettoyage::GererChangementCible);
```
Le composant marche sur **n'importe quel acteur** qui a aussi un `UInteractionComponent`. S'il n'y en a pas, un message jaune le dit une fois dans l'**Output Log** et tout le reste ne fait rien (chaque fonction teste `ComposantInteraction`) : pas de plantage. Pourquoi c'est sûr au BeginPlay : `InteractionComponent` est créé dans le **constructeur** du personnage, il existe donc déjà ; et l'ordre des BeginPlay des composants n'a pas d'importance ici, on ne lit pas encore sa cible.
`ComposantInteraction` est une `UPROPERTY(Transient)` **privée** : le ramasse-miettes la voit (voir [TObjectPtr](#notion-tobjectptr)), mais elle n'est ni sauvegardée ni visible en Blueprint. `Personnage` et `CameraJoueur` sont supprimés : ils n'étaient pas visibles en Blueprint (simple `UPROPERTY()`), aucun graphe ne pouvait les utiliser.

<a id="notion-transient"></a>
> **Notion nouvelle — `UPROPERTY(Transient)`**
> `Transient` = « ne jamais sauvegarder cette variable ». Une `UPROPERTY` normale est écrite dans le `.uasset` / la carte quand on enregistre (avec sa valeur par défaut ou réglée). Une donnée qui n'a de sens **qu'en jeu** (un pointeur trouvé au BeginPlay, un cache) est marquée `Transient` : elle repart toujours vide au chargement, et n'encombre pas les fichiers. On garde quand même `UPROPERTY` pour que le **ramasse-miettes** connaisse le pointeur.

<a id="notion-adddynamic-endplay"></a>
> **Notion nouvelle — S'abonner en C++ à un delegate : `AddDynamic` / `RemoveDynamic`, et `EndPlay`**
> En Blueprint on s'abonne avec *Bind Event to* (voir [Bind Event](editeur-contenu.md#notion-bind-event)). En C++, l'équivalent pour un delegate **dynamique** est
> `Emetteur->OnFocusChanged.AddDynamic(this, &UComposantNettoyage::GererChangementCible);`
> - `this` = l'objet qui écoute ; `&UComposantNettoyage::GererChangementCible` = l'**adresse de la fonction** à appeler (syntaxe `&Classe::Fonction`).
> - La fonction doit être une **`UFUNCTION()`** (sinon erreur de compilation ou abonnement qui ne marche pas : le delegate dynamique retrouve la fonction **par son nom** via la réflexion) et avoir **les mêmes paramètres** que le delegate (ici `AActor*`).
> - À chaque `Broadcast(Cible)` de l'émetteur, `GererChangementCible(Cible)` est appelée.
> - **`RemoveDynamic`** (mêmes arguments) **désabonne**. On le fait dans **`EndPlay`**, l'inverse de `BeginPlay` : appelée quand le composant quitte le jeu (fin de partie, acteur détruit, changement de niveau). Un delegate dynamique ne rappelle pas un objet détruit (il garde une référence « faible »), mais se désabonner proprement évite d'être appelé pendant la destruction et rend le code symétrique et lisible : *ce que BeginPlay branche, EndPlay le débranche*.
> - On ne s'abonne **qu'une fois**, au BeginPlay : un double abonnement au même couple objet/fonction est une erreur classique (le moteur le signale ou l'ignore, mais il ne faut jamais compter dessus).
> Doc : [Dynamic Delegates](https://dev.epicgames.com/documentation/en-us/unreal-engine/dynamic-delegates-in-unreal-engine), [Actor Lifecycle (EndPlay)](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-actor-lifecycle).

**b) Le prompt sur événement (dette 1, 5).** Le Tick est supprimé (`bCanEverTick = false`). `GererChangementCible` appelle `MettreAJourTexteInteraction(PeutNettoyerCible())`, avec
```cpp
bool UComposantNettoyage::PeutNettoyerCible() const
{
    return ComposantInteraction && ComposantInteraction->CanFocusPerform(EInteractionType::Wipe);
}
```
Comme la mise à jour n'a lieu qu'au changement de cible, il faut donner l'état de départ au widget quand il arrive : `DefinirWidgetInteraction` appelle aussi `MettreAJourTexteInteraction(PeutNettoyerCible())`. (Au BeginPlay, rien n'est visé : le prompt est masqué, comme son réglage par défaut *Hidden* dans `WBP_Interaction`.)

**c) Essuyer — décision par le profil (dette 2).**
```cpp
if (!PeutNettoyerCible()) { return; }                         // le profil dit « Wipe autorisé » ?
APreuveNettoyable* Preuve = Cast<APreuveNettoyable>(ComposantInteraction->GetCurrentFocus());
if (!Preuve) { return; }                                      // qui sait s'essuyer, aujourd'hui
Preuve->Essuyer();
... OnProgressionNettoyageChanged.Broadcast(Preuve->ProgressionNettoyage);
```
La **décision** vient du profil : décocher *Wipe* sur une preuve l'empêche d'être nettoyée et masque son prompt. Le `Cast` reste, mais seulement pour **exécuter** : `Essuyer()` n'existe que sur `APreuveNettoyable`. Il disparaîtra quand le composant de preuve (T003/T004) portera l'état de nettoyage. Comportement conservé : `Essuyer()` (+0.25, plafonné à 1, disparition à 1.0) puis `Broadcast` de la progression de la preuve, exactement comme avant. Plus de `GetWorld()` non testé : le composant n'en a plus besoin (dette 16).

**d) Le log de progression (dette 16).** `UE_LOG(LogTemp, Warning, ...)` à chaque essuyage devient `UE_LOG(LogTemp, Log, ...)` **seulement si `bAfficherDebug`** est coché sur `ComposantNettoyage` (catégorie *Nettoyage | Debug*). Niveau `Log` = ligne blanche normale au lieu de jaune « attention ».

**e) `ProcessEvent` moins fragile (dette 5, réduite).** Le remplacement par une vraie interface C++ du widget est hors périmètre (fiche). Deux protections :
1. l'appel n'a plus lieu 60 fois/s mais seulement au changement de cible ;
2. **avant** d'appeler, on vérifie que la fonction Blueprint a bien la signature attendue, et on fabrique le bloc de paramètres **à la taille exacte** que la fonction annonce, au lieu d'une `struct` écrite à la main :
```cpp
const FBoolProperty* ParametreAfficher =
    CastField<FBoolProperty>(Fonction->FindPropertyByName(TEXT("Afficher")));
if (Fonction->NumParms != 1 || !ParametreAfficher || !ParametreAfficher->HasAnyPropertyFlags(CPF_Parm))
{
    UE_LOG(LogTemp, Warning, TEXT("... AfficherInteraction doit avoir un seul parametre booleen 'Afficher'."));
    return;
}
uint8* Parametres = static_cast<uint8*>(FMemory_Alloca(Fonction->ParmsSize));
FMemory::Memzero(Parametres, Fonction->ParmsSize);
ParametreAfficher->SetPropertyValue_InContainer(Parametres, Afficher);
WidgetInteraction->ProcessEvent(Fonction, Parametres);
```
Résultat : si quelqu'un renomme le paramètre ou en ajoute un dans `WBP_Interaction`, le prompt ne s'affiche plus **et un message jaune l'explique** dans l'Output Log, au lieu d'une lecture mémoire hors limites. Renommer la **fonction** elle-même reste silencieux (`FindFunction` renvoie `nullptr`), comme avant.

<a id="notion-signature-ufunction"></a>
> **Notion nouvelle — Lire la signature d'une `UFunction` (`FProperty`, `CastField`, `NumParms`, `ParmsSize`, `FMemory_Alloca`)**
> Grâce à la réflexion, une `UFunction` (la description d'une fonction, renvoyée par `FindFunction`) connaît ses paramètres :
> - `NumParms` = nombre de paramètres (valeur de retour comprise) ; `ParmsSize` = taille en octets du bloc de paramètres que `ProcessEvent` attend.
> - Chaque paramètre est une **`FProperty`** (description d'une variable : nom, type, position dans le bloc). `FindPropertyByName(TEXT("Afficher"))` la retrouve par son nom ; `CastField<FBoolProperty>(...)` vérifie que c'est un booléen (c'est le `Cast` des `FProperty`, qui ne sont pas des `UObject`) ; le drapeau `CPF_Parm` confirme que c'est un paramètre et pas une variable locale de la fonction.
> - `SetPropertyValue_InContainer(Bloc, Valeur)` écrit la valeur **au bon endroit** du bloc, d'après la description de la propriété : on ne devine plus la disposition mémoire.
> - `FMemory_Alloca(Taille)` réserve une zone mémoire temporaire **sur la pile** (libérée automatiquement à la fin de la fonction, aucun `delete` à faire) ; `FMemory::Memzero` la remplit de zéros.
> C'est la façon dont le moteur lui-même prépare les appels de fonctions Blueprint. Ça reste un appel « par nom » : la solution propre (classe C++ parente du widget avec un `BlueprintImplementableEvent`) est prévue avec l'UI de feedback.
> Doc : [UFunction (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/UObject/UFunction), [FProperty (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/UObject/FProperty).

`GetNameSafe(Objet)` (utilisé dans les messages) renvoie le nom de l'objet, ou `"None"` s'il est nul, sans planter.

**f) Autres nettoyages (dette 15).** `WidgetInteraction` passe de `BlueprintReadWrite` à **`BlueprintReadOnly`** : on ne peut plus le changer que par `DefinirWidgetInteraction` (qui, elle, pousse l'état du prompt). J'ai vérifié qu'aucun Blueprint n'utilise un nœud *Set Widget Interaction* (le nom de la variable n'apparaît pas dans `BP_FirstPersonCharacter.uasset`, seul `DefinirWidgetInteraction` y est). `#include "Blueprint/UserWidget.h"` passe du `.h` au `.cpp` (déclaration anticipée `class UUserWidget;` dans le `.h`, voir [Déclaration anticipée](#notion-declaration-anticipee)) ; `StructOnScope.h`, `Character.h`, `CameraComponent.h`, `World.h`, `DrawDebugHelpers.h` retirés (plus utilisés) ; commentaires du modèle retirés.

#### 2.4 `PersonnageNettoyeur.h` / `.cpp`
- **`TenterNettoyage()` supprimée (dette 4)** : vide, et T000 a confirmé qu'aucun Blueprint ne l'appelle (le seul « TenterNettoyage » du graphe est celui de `ComposantNettoyage`). Effet visible : dans le menu des nœuds de `BP_FirstPersonCharacter`, il n'y a plus deux « Tenter Nettoyage » (piège T000 §2.9 n° 4).
- **`BeginPlay`, `Tick`, `SetupPlayerInputComponent` supprimées (dette 15)** : elles ne faisaient qu'appeler `Super::`. Sans redéfinition, c'est directement la version du parent `ACharacter` qui s'exécute : **même comportement**. Les entrées sont gérées par l'Event Graph (Enhanced Input), pas par `SetupPlayerInputComponent`. `#include "Components/InputComponent.h"` retiré.
- `CanJumpInternal_Implementation` (pas de saut en tenant un objet) inchangée.

#### 2.5 Nouvelle chronologie (remplace les lignes correspondantes de T000 §2.8)
| Moment | Qui | Fait quoi |
|---|---|---|
| `BeginPlay` des composants | `UComposantNettoyage` | trouve `InteractionComponent`, s'abonne à `OnFocusChanged` |
| `BeginPlay` du personnage (Blueprint) | Event Graph | `DefinirWidgetInteraction(widget)` → pousse l'état initial du prompt (masqué) |
| **Chaque image** | `UInteractionComponent` | **le** sweep → si la cible a changé : `OnFocusChanged.Broadcast(cible)` ; trait de debug si `bAfficherDebug` |
| Cible changée | `UComposantNettoyage::GererChangementCible` | `CanFocusPerform(Wipe)` → `AfficherInteraction(true/false)` du widget |
| Appui **E** | `UComposantNettoyage::TenterNettoyage` | profil `Wipe` ? → `Essuyer()` → `Broadcast(progression)` (log si `bAfficherDebug`) |
| R / clic gauche | `Pull()` / `Grab()` | inchangés (via `CanFocusPerform`) |

### 3. Configuration
Aucune (ni `.Build.cs`, ni `.ini`, ni `.uproject`). Le module dépend déjà de `UMG` pour `UUserWidget`.

### 4. Étapes dans l'éditeur

**a) Vérification du profil de `BP_PreuveNettoyable` (faite, sans éditeur).** L'éditeur était fermé (outils MCP indisponibles). J'ai cherché le texte `AllowedInteractions` dans tous les `.uasset` / `.umap` de `Content/` : il n'apparaît que dans `BP_ObjetPull_Test` et `BP_ObjetGrab_Test1`. Cela veut dire que `BP_PreuveNettoyable` et les preuves placées dans les niveaux **ne modifient pas** la liste : ils héritent de la valeur du constructeur C++ `APreuveNettoyable`, **[Wipe]** (Unreal n'enregistre dans un asset que ce qui diffère du parent). Le nettoyage continue donc de fonctionner. **Pour le vérifier toi-même :** ouvre `Content/Nettoyeur/Preuves/BP_PreuveNettoyable` → panneau **Composants** → `ProfilInteractions` → **Détails** → **Interactions** → *Allowed Interactions* : un élément **Essuyer**. Fais de même sur la preuve placée dans `Lvl_FirstPerson` (sélectionne-la dans l'**Organiseur (Outliner)**, composant `ProfilInteractions`) : pas de flèche jaune « réinitialiser » à côté de la liste = valeur héritée.

**b) B1 — Nettoyer le graphe de `BP_FirstPersonCharacter` (à faire par toi, je ne modifie pas les Blueprints).** Ces nœuds ne sont jamais exécutés ou n'affichent que du debug ; les retirer ne change pas le jeu. Fais un commit avant si tu veux pouvoir revenir en arrière.
1. Ouvre `Content/FirstPerson/Blueprints/BP_FirstPersonCharacter` → onglet **Event Graph**.
2. Trouve le nœud rouge **IA_Essuyer** (Ctrl+F → `IA_Essuyer`). **Ne touche pas** à lui, ni à *Get ComposantNettoyage*, ni à *Tenter Nettoyage* : c'est le nettoyage qui marche.
3. À côté se trouve le **bloc mort** : *Get FirstPersonCamera*, *Get World Location*, *Get Forward Vector*, *Multiply* (B = 300), *Add*, **Line Trace By Channel** (sa broche exec d'entrée, à gauche, n'a aucun fil), *Break Hit Result* et **Print String** « Touche E détécter ». Sélectionne ces 8 nœuds : maintiens **Ctrl** et clique sur chacun (ou trace un rectangle de sélection autour s'ils sont groupés, puis **Ctrl+clic** sur un nœud vivant pour le retirer de la sélection s'il a été pris).
4. Vérifie avant de supprimer : aucun des nœuds sélectionnés ne doit être *IA_Essuyer* / *Tenter Nettoyage* / *Get ComposantNettoyage*. Appuie sur **Suppr (Delete)**.
5. Variable `NewVar` (optionnel, recommandé) : panneau **Mon Blueprint (My Blueprint)** → *Variables* → clic droit sur **NewVar** → **Renommer (Rename)** → `WidgetInteraction` → Entrée. Les nœuds *Set* / *Get* qui l'utilisent sont renommés automatiquement.
6. **Compiler** puis **Enregistrer** (boutons en haut à gauche).
7. Ouvre `Content/Nettoyeur/WBP_Interaction` → onglet **Graphe** → **Mon Blueprint** → *Fonctions* → double-clic sur **MettreAJourProgression**.
8. La chaîne est : entrée *MettreAJourProgression* → **Print String** « (Fonction Appelee) » → *Set Percent* → **Print String** (texte ← *To String (Float)*). Il faut retirer les deux Print String **sans couper la chaîne** :
   - Tire un fil de la broche **then** (exec sortie) du nœud d'entrée *MettreAJourProgression* vers la broche **exec d'entrée** de *Set Percent* (ce nouveau fil remplace l'ancien, qui allait au premier Print String).
   - Sélectionne le premier **Print String** → **Suppr**.
   - Sélectionne le second **Print String** et le nœud **To String (Float)** qui l'alimente → **Suppr**. *Set Percent* devient le dernier nœud : c'est normal.
   - Vérifie que *Set Percent* a toujours **Target** ← *BarProgression* et **In Percent** ← *Progression*.
9. **Compiler** → **Enregistrer**.
10. Test rapide : Play, essuie une preuve 4 fois : la barre avance, plus aucun texte bleu en haut à gauche de l'écran.

**c) Réglages de debug (pour QA / toi, ne pas enregistrer cochés).**
- Ligne de visée : `BP_FirstPersonCharacter` → **Composants** → `InteractionComponent` → **Détails** → **Interaction | Debug** → cocher **Afficher Debug**.
- Journal de progression : composant `ComposantNettoyage` → **Détails** → **Nettoyage | Debug** → cocher **Afficher Debug**. Les lignes « Progression nettoyage : 0.250000 » apparaissent dans **Fenêtre (Window) → Journal de sortie (Output Log)**.

### 5. Compilation et tests
- **Compilation :** éditeur fermé, depuis PowerShell : `Build.bat NettoyeurEditor Win64 Development -Project=... -WaitMutex` (commande de `CLAUDE.md` §6). UHT : 7 fichiers générés ; 7 compilations + 2 liens. **Résultat : `Succeeded`, 0 erreur, 0 avertissement** (recherche de « warning » / « error » dans `%LOCALAPPDATA%\UnrealBuildTool\Log.txt` : aucune occurrence hors lignes d'information de la chaîne d'outils). Aucune erreur rencontrée.
- **Tests en jeu : non réalisés par moi** (l'éditeur était fermé, pas d'outil MCP). Procédure pour QA dans la fiche T001.
- Contrôles faits : `grep` — plus aucun `GetAimDirection`, `CameraJoueur`, `Personnage` (membre), `TenterNettoyage` côté personnage ; une seule trace de visée (`SweepSingleByChannel` dans `UpdateFocus`) ; fichiers en UTF-8 sans BOM, fins de ligne CRLF.

### 6. Git
- Branche : `tache/T001-unifier-visee` (déjà active). **Aucun commit** (consigne : l'orchestrateur commite après QA et validation). Message proposé : `T001: unifier la visée et faire passer le nettoyage par le profil`.

### 7. Résultat et points ouverts
- ✅ Compilation propre. Code prêt pour QA ; comportement en jeu **à vérifier** (non testé).
- ⚠️ Différence attendue mais à surveiller : le prompt et l'essuyage utilisent maintenant le **sweep sphère de 10 cm** au lieu d'un rayon fin : au bord d'une preuve, le prompt peut apparaître un peu plus tôt qu'avant. Même portée (300, désormais réglée par `Trace Distance`).
- ⚠️ Un acteur qui a *Wipe* dans son profil mais **n'est pas** un `APreuveNettoyable` affiche le prompt mais E ne fait rien (aucun cas aujourd'hui). Disparaîtra avec le composant de preuve (T003/T004).
- Dette 15 restante (hors fichiers autorisés) : `ClampMin/ClampMax` sur `ProgressionNettoyage` (`PreuveNettoyable.h`). Le `.h` du personnage inclut encore `InteractionComponent.h` / `PhysicsHandleComponent.h` au lieu de déclarations anticipées (sans effet, laissé pour ne pas élargir).
- Pour `editeur-contenu` / le développeur : étapes B1 du §4.b. Aucun asset à créer.
- Proposition pour l'orchestrateur (`00_etat-projet.md`) : dette 1, 2, 4, 7, 13, 16 → traitées ; 5 → réduite (reste le remplacement du `ProcessEvent`, avec l'UI de feedback) ; 15 → partiellement traitée.

---

## T000 (partie B) — Documenter le code C++ existant et corriger l'encodage de `InteractionTypes.h` — 2026-11-11

**Objectif :** expliquer, après coup, comment fonctionne chaque classe C++ du prototype et comment elles collaborent en jeu, pour que les tâches T001+ partent d'une base comprise. Amorcer la section C++ de `doc/notions.md`. Corriger l'affichage de « Traîner » dans l'éditeur.
**Étape du GDD :** préalable (§10.1 : « le code actuel doit être examiné avant de choisir les signatures définitives »).
**Branche :** `tache/T000-verification-existant`

> Cette partie complète la [partie A (editeur-contenu)](editeur-contenu.md#t000-partie-a--vérifier-la-config-éditeur-et-documenter-lexistant--2026-11-11), qui décrit **côté éditeur** comment les Blueprints utilisent ce code (composants, graphes, widget). Je ne la répète pas : je mets des liens vers ses points.
> **Aucune dette technique n'a été corrigée** (hors périmètre, T001+). La seule modification de code est la conversion d'encodage de `InteractionTypes.h`.

### 1. Fichiers créés / modifiés
- `Source/Nettoyeur/InteractionTypes.h` — modifié — **réencodé de Windows-1252 en UTF-8** (sans BOM). Seul le « î » de « Traîner » change d'octets ; aucun caractère de code ne change (détail §3).
- `doc/agents/gameplay-cpp.md` — modifié — cette entrée.
- `doc/notions.md` — modifié — section « C++ Unreal » remplie ; les autres sections (notions éditeur de la partie A) ne sont pas touchées.
- `doc/taches/T000-verification-et-doc-existant.md` — modifié — statut « Prêt pour QA » et liste des vérifications QA.

Fichiers **lus en entier** sans modification : `PersonnageNettoyeur.h/.cpp`, `InteractionComponent.h/.cpp`, `InteractionProfileComponent.h/.cpp`, `InteractionTypes.h/.cpp`, `ComposantNettoyage.h/.cpp`, `PreuveNettoyable.h/.cpp`, `Nettoyeur.Build.cs`.

### 2. Implémentation du code (documentation rétroactive)

#### 2.0 Vue d'ensemble : qui possède quoi

```
BP_FirstPersonCharacter  (Blueprint, parent C++ APersonnageNettoyeur)
├─ PhysicsHandle         UPhysicsHandleComponent   créé en C++ (constructeur du personnage)
├─ InteractionComponent  UInteractionComponent     créé en C++ ; reçoit le PhysicsHandle
└─ ComposantNettoyage    UComposantNettoyage       ajouté dans le Blueprint (pas en C++)
        │ reçoit WBP_Interaction au BeginPlay (DefinirWidgetInteraction)
        │ diffuse OnProgressionNettoyageChanged → Custom Event du Blueprint

BP_PreuveNettoyable      (Blueprint, parent C++ APreuveNettoyable)
├─ MeshPreuve            UStaticMeshComponent      racine, créé en C++
└─ ProfilInteractions    UInteractionProfileComponent  [Wipe], créé en C++

BP_ObjetPull_Test        (Blueprint, parent Actor)
├─ StaticMesh            racine, Simulate Physics
└─ InteractionProfile    UInteractionProfileComponent  [Pull], ajouté en Blueprint
```

Il y a **deux chaînes indépendantes** qui ne se parlent pas :
1. **Nettoyage** : `UComposantNettoyage` → `APreuveNettoyable` (reconnaît la preuve par sa **classe**).
2. **Manipulation physique** (Pull / Grab) : `UInteractionComponent` → `UInteractionProfileComponent` de la cible → `UPhysicsHandleComponent`.

Le seul point commun est l'enum `EInteractionType` : la preuve déclare `Wipe` dans son profil, mais **personne ne lit ce `Wipe`** (dette n° 2).

---

#### 2.1 Anatomie commune d'une classe Unreal (vaut pour les 6 fichiers)

Prenons l'en-tête de `PreuveNettoyable.h` :

```cpp
#pragma once                         // n'inclure ce fichier qu'une fois par compilation
#include "CoreMinimal.h"             // types de base d'Unreal (FVector, TArray, FString…)
#include "GameFramework/Actor.h"     // la classe parente AActor
#include "PreuveNettoyable.generated.h"  // code généré : TOUJOURS le dernier include

class UStaticMeshComponent;          // déclaration anticipée (voir notion)

UCLASS()                             // « cette classe fait partie du système de réflexion »
class NETTOYEUR_API APreuveNettoyable : public AActor
{
    GENERATED_BODY()                 // insère le code généré par UHT
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nettoyage",
        meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float ProgressionParEssuyage = 0.25f;

    UFUNCTION(BlueprintCallable, Category = "Nettoyage")
    void Essuyer();
};
```

<a id="notion-reflexion-uht"></a>
> **Notion nouvelle — Réflexion, Unreal Header Tool (UHT) et `.generated.h`**
> Le C++ ne sait pas, à l'exécution, quelles variables et fonctions possède une classe. Unreal ajoute ce savoir : c'est le **système de réflexion**. Avant le compilateur C++, un outil, **Unreal Header Tool (UHT)**, lit tous les `.h`, repère les macros `UCLASS`, `UPROPERTY`, `UFUNCTION`, `UENUM`… et **génère** du code C++ supplémentaire dans `Intermediate/.../NomDuFichier.generated.h` et `.gen.cpp`. C'est grâce à ce code que l'éditeur affiche tes variables dans **Détails**, que les Blueprints voient tes fonctions, que la sauvegarde et le ramasse-miettes fonctionnent.
> Conséquences pratiques : (1) `#include "X.generated.h"` doit être **le dernier include** du `.h` ; (2) ajouter/retirer une `UPROPERTY`, une `UFUNCTION` ou changer un `UMETA` oblige UHT à régénérer : **Live Coding ne suffit pas, il faut fermer l'éditeur et recompiler** (c'est le cas de la correction d'encodage de cette tâche) ; (3) UHT lit les fichiers en **UTF-8** : un accent mal encodé dans un `UMETA(DisplayName = "...")` se retrouve mal affiché dans l'éditeur.
> Doc : [Reflection System](https://dev.epicgames.com/documentation/en-us/unreal-engine/reflection-system-in-unreal-engine), [Unreal Header Tool](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-header-tool-for-unreal-engine).

<a id="notion-uclass-uproperty-ufunction"></a>
> **Notion nouvelle — `UCLASS`, `UPROPERTY`, `UFUNCTION`**
> Ce sont des **macros d'annotation** lues par UHT. Elles ne changent rien au C++ lui-même ; elles disent à Unreal quoi exposer.
> - **`UCLASS(...)`** devant une classe : la rend connue d'Unreal (Blueprint possible, apparaît dans les listes de classes). Exemples de spécificateurs vus ici : `ClassGroup = (Custom)` (rubrique « Custom » du menu **+ Ajouter**) et `meta = (BlueprintSpawnableComponent)` (le composant peut être ajouté dans un Blueprint avec **+ Ajouter** — c'est ce qui a permis d'ajouter `ComposantNettoyage` et `InteractionProfile` à la main).
> - **`UPROPERTY(...)`** devant une variable membre. Spécificateurs rencontrés :
>   - `EditAnywhere` : modifiable dans **Détails**, sur le Blueprint (Class Defaults) **et** sur chaque instance placée dans le niveau. `VisibleAnywhere` : affichée mais grisée (pour un pointeur de composant, cela permet quand même de cliquer le composant et de régler **ses** propriétés).
>   - `BlueprintReadOnly` : lisible dans un graphe (nœud *Get*), pas modifiable. `BlueprintReadWrite` : *Get* et *Set*.
>   - `BlueprintAssignable` : pour un delegate, autorise *Bind Event to…* en Blueprint (voir [delegate](#notion-delegate-dynamique-multicast)).
>   - `Category = "Interaction|Pull"` : rubrique dans **Détails** et dans le menu des nœuds ; `|` crée une sous-rubrique.
>   - `meta = (ClampMin = "1.0", ClampMax = "...")` : bornes imposées à la saisie dans l'éditeur (le C++ peut toujours écrire une autre valeur).
>   - `UPROPERTY()` **vide** (ex. `CurrentFocus`, `PhysicsHandle` privés de `UInteractionComponent`) : rien n'est exposé, mais le **ramasse-miettes** (garbage collector) connaît le pointeur — voir [TObjectPtr](#notion-tobjectptr).
> - **`UFUNCTION(...)`** devant une fonction membre : `BlueprintCallable` = nœud avec broches d'exécution (il « fait » quelque chose, ex. *Tenter Nettoyage*) ; `BlueprintPure` = nœud **sans** broche d'exécution, qui ne doit rien modifier (ex. *Can Perform*, *Is Holding Object*, *Est Nettoyee*) — voir les nœuds purs dans la [notion Event Graph](editeur-contenu.md#notion-event-graph). `UFUNCTION()` vide (ex. `MettreAJourTexteInteraction`) : connue de la réflexion mais pas visible en Blueprint.
> Doc : [Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-uproperties), [Property Specifiers](https://dev.epicgames.com/documentation/en-us/unreal-engine/property-specifiers), [UFunctions](https://dev.epicgames.com/documentation/en-us/unreal-engine/ufunctions-in-unreal-engine), [Class Specifiers](https://dev.epicgames.com/documentation/en-us/unreal-engine/class-specifiers), [Metadata Specifiers](https://dev.epicgames.com/documentation/en-us/unreal-engine/metadata-specifiers-in-unreal-engine).

<a id="notion-generated-body"></a>
> **Notion nouvelle — `GENERATED_BODY()` et la macro `NETTOYEUR_API`**
> `GENERATED_BODY()` est la **première ligne** de toute classe ou struct annotée : UHT la remplace par le code généré (constructeurs internes, informations de type, fonction `StaticClass()` qui permet d'écrire `UInteractionComponent::StaticClass()`…). Sans elle, la compilation échoue.
> `NETTOYEUR_API` (entre `class` et le nom) dit « cette classe est **exportée** par le module `Nettoyeur` » : un autre module (un plugin, un module d'éditeur) pourra l'utiliser. Le nom vient du module (`NOMMODULE_API`).
> Doc : [Objects](https://dev.epicgames.com/documentation/en-us/unreal-engine/objects-in-unreal-engine), [Gameplay Classes](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-classes-in-unreal-engine).

<a id="notion-prefixes"></a>
> **Notion nouvelle — Préfixes des types Unreal (`A`, `U`, `F`, `E`, `T`)**
> Unreal impose un préfixe selon la nature du type : **`A`** = Actor (objet placé dans le monde : `APreuveNettoyable`, `APersonnageNettoyeur`) ; **`U`** = UObject qui n'est pas un acteur (composants : `UInteractionComponent`, widget : `UUserWidget`) ; **`F`** = struct ou type « valeur » (`FVector`, `FHitResult`, `FRotator`, et notre delegate `FOnProgressionNettoyageChanged`) ; **`E`** = enum (`EInteractionType`) ; **`T`** = template (`TArray`, `TObjectPtr`). UHT **refuse** une `UCLASS` dérivée d'`AActor` qui ne commence pas par `A`.
> Doc : [Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine).

<a id="notion-tobjectptr"></a>
> **Notion nouvelle — `TObjectPtr<T>` et le ramasse-miettes**
> Les objets Unreal (`UObject`, donc acteurs et composants) sont détruits par un **ramasse-miettes** (garbage collector, GC) : périodiquement, Unreal supprime les objets que plus rien ne référence. Il ne « voit » une référence que si elle est dans une **`UPROPERTY`**. Un pointeur membre **sans** `UPROPERTY` vers un `UObject` peut donc pointer vers un objet détruit (pointeur « pendant » → plantage).
> `TObjectPtr<UInteractionComponent>` est, depuis UE5, la façon recommandée d'écrire un **pointeur membre** vers un `UObject` dans une `UPROPERTY`. En jeu packagé, c'est exactement un pointeur brut ; dans l'éditeur, il ajoute du suivi d'accès et du chargement différé. On l'utilise comme un pointeur : `InteractionComponent->SetPhysicsHandle(...)`, `if (!CurrentFocus)`. Pour les **paramètres et variables locales**, on garde des pointeurs bruts (`UPrimitiveComponent* Primitive`).
> Doc : [UE5 Migration Guide (TObjectPtr)](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-migration-guide), [Object Handling / GC](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-object-handling-in-unreal-engine).

<a id="notion-declaration-anticipee"></a>
> **Notion nouvelle — Déclaration anticipée (`class UStaticMeshComponent;`)**
> Dans un `.h`, écrire `class UStaticMeshComponent;` dit au compilateur « ce type existe » **sans inclure** son en-tête. C'est suffisant tant qu'on ne manipule qu'un **pointeur** vers lui. Le vrai `#include` est fait dans le `.cpp`, là où on appelle ses fonctions. Avantage : moins de fichiers recompilés quand un en-tête change. On le voit dans `PreuveNettoyable.h`, `InteractionComponent.h`, `ComposantNettoyage.h` ; `PersonnageNettoyeur.h` inclut au contraire directement `InteractionComponent.h` et `PhysicsHandleComponent.h` (ça marche, c'est juste plus lourd).

<a id="notion-modules-build-cs"></a>
> **Notion nouvelle — Module et `Nettoyeur.Build.cs`**
> Le code du jeu est un **module** nommé `Nettoyeur` (`Source/Nettoyeur/`). Son fichier `Nettoyeur.Build.cs` (en C#) dit à l'outil de compilation (Unreal Build Tool) **de quels autres modules du moteur** il dépend. Ici : `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `UMG`. `UMG` est là parce que `ComposantNettoyage.h` utilise `UUserWidget`. Si on inclut un en-tête d'un module absent de cette liste, on obtient une erreur d'édition de liens (« unresolved external symbol »). Pas de modification dans cette tâche.
> Doc : [Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules), [Module Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/module-properties-in-unreal-engine).

---

#### 2.2 `InteractionTypes.h` / `.cpp` — la liste des interactions

```cpp
UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Interact UMETA(DisplayName = "Interagir"),
    Wipe     UMETA(DisplayName = "Essuyer"),
    Grab     UMETA(DisplayName = "Saisir"),
    Pull     UMETA(DisplayName = "Tirer"),
    Drag     UMETA(DisplayName = "Traîner"),
    ...      // Push, Launch, Open, Close, Hide
};
```

- Une simple **liste de noms** : ce que le jeu sait reconnaître comme interaction. Aucune logique.
- Utilisées réellement aujourd'hui : `Wipe` (déclaré par la preuve, jamais testé), `Grab` et `Pull` (testés par `UInteractionComponent`). Les 7 autres ne sont lues par aucun code.
- `InteractionTypes.cpp` est vide (un simple `#include`) : il n'est pas nécessaire pour un enum, mais il est inoffensif.

<a id="notion-uenum"></a>
> **Notion nouvelle — `UENUM`, `enum class` et `UMETA(DisplayName)`**
> Un **enum** est un type qui ne peut prendre qu'une valeur parmi une liste fixe (`EInteractionType::Pull`). `enum class` (C++11) oblige à écrire le préfixe `EInteractionType::` et empêche de le confondre avec un entier. `: uint8` stocke la valeur sur 1 octet (obligatoire pour un enum exposé aux Blueprints).
> `UENUM(BlueprintType)` le rend utilisable en Blueprint et dans **Détails** (liste déroulante, ex. *Allowed Interactions* du profil). `UMETA(DisplayName = "Tirer")` change seulement le **texte affiché** dans l'éditeur : dans le code, la valeur s'appelle toujours `Pull`. C'est ce texte qui s'affichait « Tra�ner » à cause de l'encodage (§3).
> Doc : [Metadata Specifiers (UMETA)](https://dev.epicgames.com/documentation/en-us/unreal-engine/metadata-specifiers-in-unreal-engine), [Reflection System](https://dev.epicgames.com/documentation/en-us/unreal-engine/reflection-system-in-unreal-engine).

---

#### 2.3 `InteractionProfileComponent.h` / `.cpp` — « ce que cet objet accepte »

- Composant **sans position** (`UActorComponent`) qu'on attache à un acteur interactif.
- Une seule donnée : `TArray<EInteractionType> AllowedInteractions` (`EditAnywhere` → on coche les interactions dans **Détails**, catégorie *Interactions*).
- Une seule fonction : `CanPerform(Interaction)` = `AllowedInteractions.Contains(Interaction)`, `BlueprintPure`.
- `PrimaryComponentTick.bCanEverTick = false` : il ne fait rien à chaque image (bonne pratique : pas de Tick si inutile).
- C'est le « mécanisme équivalent » à `IInteractable` retenu par la décision **D002**.
- Utilisé par : `UInteractionComponent::Pull()` et `Grab()` (lu sur la **cible** via `FindComponentByClass`) ; créé en C++ dans `APreuveNettoyable` (avec `Wipe`) ; ajouté en Blueprint sur les objets de test ([partie A, point 6](editeur-contenu.md#point-6--réglages-des-objets-de-test)).

<a id="notion-actor-component"></a>
> **Notion nouvelle — Actor Component (`UActorComponent` vs `USceneComponent`)**
> Un **composant** est une brique de comportement réutilisable qu'on attache à un acteur (voir aussi le [panneau Composants](editeur-contenu.md#notion-composants-herites)). En C++ il y en a deux familles :
> - **`UActorComponent`** : pure logique, **sans position** dans le monde (`UInteractionComponent`, `UComposantNettoyage`, `UInteractionProfileComponent`, `UPhysicsHandleComponent`). Il apparaît sous l'arbre dans le panneau Composants.
> - **`USceneComponent`** (et ses enfants `UPrimitiveComponent`, `UStaticMeshComponent`, `UCameraComponent`…) : **a une position**, peut être attaché à un parent, et pour les « primitives », a une forme visible / une collision / de la physique.
> Un composant connaît son acteur par `GetOwner()`, et on retrouve un composant sur un acteur par `Acteur->FindComponentByClass<UTypeDuComposant>()` (renvoie le **premier** trouvé, ou `nullptr`). Avantage : la même brique (le profil) sert sur une preuve, une caisse, une porte… sans héritage commun.
> Doc : [Components](https://dev.epicgames.com/documentation/en-us/unreal-engine/components-in-unreal-engine).

<a id="notion-tarray"></a>
> **Notion nouvelle — `TArray`**
> Le tableau dynamique d'Unreal (équivalent de `std::vector`) : `Add(x)` ajoute, `Contains(x)` teste la présence, `Num()` donne la taille, on le parcourt avec `for (const FHitResult& Hit : GroundHits)`. Dans une `UPROPERTY`, l'éditeur l'affiche comme une liste avec un bouton **+**.
> Doc : [Array Containers](https://dev.epicgames.com/documentation/en-us/unreal-engine/array-containers-in-unreal-engine).

---

#### 2.4 `PreuveNettoyable.h` / `.cpp` — la tache à nettoyer

**Constructeur** (`APreuveNettoyable()`) :
1. `PrimaryActorTick.bCanEverTick = false` : la preuve ne « tique » pas, elle ne fait que réagir.
2. `MeshPreuve = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshPreuve"))` puis `SetRootComponent(MeshPreuve)` : le mesh devient la **racine** (c'est lui que la trace du nettoyage touche ; son mesh, son matériau et sa collision *BlockAllDynamic* sont réglés dans le Blueprint, [partie A point 6](editeur-contenu.md#point-6--réglages-des-objets-de-test)).
3. `ProfilInteractions = CreateDefaultSubobject<UInteractionProfileComponent>(...)` puis `AllowedInteractions.Add(EInteractionType::Wipe)` : la preuve **déclare** accepter l'essuyage.

**Données :** `ProgressionParEssuyage` (0.25, réglable, bornée 0.01–1) et `ProgressionNettoyage` (0 → 1, `VisibleAnywhere` : on la voit évoluer dans **Détails** pendant le jeu, sur l'instance sélectionnée).

**`Essuyer()`** : si déjà nettoyée, ne fait rien ; sinon `ProgressionNettoyage = FMath::Clamp(Progression + ParEssuyage, 0, 1)` puis `MettreAJourEtatNettoyage()`. `FMath::Clamp(v, min, max)` force `v` à rester entre `min` et `max` : 0.25 × 4 = 1.0, jamais 1.25.

**`MettreAJourEtatNettoyage()`** : à 100 %, `SetActorHiddenInGame(true)` (plus dessinée) et `SetActorEnableCollision(false)` (plus touchée par les traces ni par le joueur). L'acteur **existe toujours** (il n'est pas détruit) : c'est ce qui permettra plus tard de l'interroger pour le score — mais aujourd'hui l'« état » (nettoyée) et l'« apparence » (cachée) sont la même chose (dette n° 3, GDD §5.2).

**`EstNettoyee()`** : `ProgressionNettoyage >= 1.0f`, `BlueprintPure`.

Ce que la preuve **ne fait pas** : elle ne prévient personne quand elle change (pas d'événement propre). C'est `UComposantNettoyage` qui diffuse la progression à sa place.

<a id="notion-createdefaultsubobject"></a>
> **Notion nouvelle — `CreateDefaultSubobject` et le constructeur d'une classe Unreal**
> `CreateDefaultSubobject<UType>(TEXT("Nom"))` crée un **composant par défaut** de l'acteur. Il ne s'appelle **que dans le constructeur**. Le nom doit être unique dans l'acteur : c'est lui qui apparaît dans le panneau Composants (« MeshPreuve », « ProfilInteractions »), marqué *hérité* dans le Blueprint enfant.
> Attention, le constructeur C++ d'une classe Unreal **n'est pas le début de la partie** : Unreal l'exécute aussi au chargement de l'éditeur pour fabriquer l'**objet par défaut de la classe** (CDO, Class Default Object — c'est ce qu'on voit dans *Class Defaults*), puis chaque instance est **copiée** de ce modèle. On y met donc seulement la **construction** (composants, valeurs par défaut), jamais de logique de jeu (pas de trace, pas d'accès au monde) : celle-ci va dans `BeginPlay()`.
> Doc : [CreateDefaultSubobject (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/UObject/UObject/CreateDefaultSubobject), [Objects](https://dev.epicgames.com/documentation/en-us/unreal-engine/objects-in-unreal-engine).

<a id="notion-text-fname"></a>
> **Notion nouvelle — `TEXT("...")` et `FName`**
> `TEXT("MeshPreuve")` transforme un texte en chaîne de **caractères larges** (le format interne d'Unreal) ; on l'utilise pour toute chaîne littérale passée au moteur (`UE_LOG`, noms de composants). `FName` est un **identifiant** (nom de composant, de fonction, d'os) : stocké une fois dans une table et comparé très vite, mais insensible à la casse et non modifiable. `NAME_None` = « aucun nom » (vu dans `GrabComponentAtLocationWithRotation(Primitive, NAME_None, ...)` : « aucun os particulier »). Pour le texte affiché au joueur, on utilisera `FText` (traduisible).
> Doc : [String Handling](https://dev.epicgames.com/documentation/en-us/unreal-engine/string-handling-in-unreal-engine), [FName](https://dev.epicgames.com/documentation/en-us/unreal-engine/fname-in-unreal-engine).

---

#### 2.5 `PersonnageNettoyeur.h` / `.cpp` — le personnage

Parent C++ de `BP_FirstPersonCharacter` ([partie A, point 1](editeur-contenu.md#point-1--classe-parente-de-bp_firstpersoncharacter)). Il hérite d'`ACharacter` (capsule, mesh, `CharacterMovement` : marche, saut, gravité).

**Constructeur :**
```cpp
PrimaryActorTick.bCanEverTick = false;   // le personnage lui-même ne tique pas
PhysicsHandle        = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
InteractionComponent->SetPhysicsHandle(PhysicsHandle);   // « injection » du handle
```
Le personnage **possède** le Physics Handle et le **prête** au composant d'interaction. Ce lien est fait dans le constructeur, entre deux composants par défaut : Unreal recopie correctement ce genre de référence quand il crée chaque instance à partir du CDO, c'est pourquoi ça marche dans `BP_FirstPersonCharacter`.

**`CanJumpInternal_Implementation()`** : seule vraie logique du personnage.
```cpp
return (!InteractionComponent || !InteractionComponent->IsHoldingObject())
    && Super::CanJumpInternal_Implementation();
```
→ **on ne peut pas sauter en tenant un objet** (Pull ou Grab). `ACharacter::Jump()` appelle `CanJump()`, qui appelle `CanJumpInternal()`. Si on n'a pas de composant d'interaction, on retombe sur la règle normale du moteur (`Super::...` : pas en l'air, etc.).

**Le reste est vide** : `BeginPlay()` et `Tick()` appellent seulement `Super::` (et `Tick` n'est même jamais appelé, `bCanEverTick = false`) ; `SetupPlayerInputComponent()` ne lie aucune touche — **toutes les entrées sont branchées dans l'Event Graph du Blueprint** ([partie A, point 3](editeur-contenu.md#point-3--graphe-dévénements-de-bp_firstpersoncharacter-essuyer-grab-pull)). `TenterNettoyage()` est une fonction vide (« la logique du rayon sera ajoutée à l'étape suivante »), exposée en Blueprint, **jamais appelée** (dette n° 4).

<a id="notion-virtual-override-super"></a>
> **Notion nouvelle — `virtual`, `override` et `Super::`**
> Une fonction `virtual` de la classe parente peut être **remplacée** dans la classe enfant. `override` écrit à la fin demande au compilateur de vérifier qu'on remplace bien une fonction existante (faute de frappe = erreur, au lieu d'une nouvelle fonction jamais appelée). `Super::BeginPlay()` appelle **la version du parent** : on l'appelle presque toujours en premier, sinon le comportement du moteur (déclencher l'événement *BeginPlay* du Blueprint, enregistrer le Tick…) est perdu. `Super` est un alias généré par `GENERATED_BODY()`.

<a id="notion-blueprintnativeevent"></a>
> **Notion nouvelle — `BlueprintNativeEvent` et le suffixe `_Implementation`**
> Certaines fonctions du moteur sont déclarées `UFUNCTION(BlueprintNativeEvent)` : elles ont une version C++ **et** peuvent être redéfinies en Blueprint. Dans ce cas, le C++ ne remplace pas `CanJumpInternal()` directement mais **`CanJumpInternal_Implementation()`** : UHT génère `CanJumpInternal()` qui appelle la version Blueprint si elle existe, sinon `_Implementation`. C'est pour ça que le personnage écrit `virtual bool CanJumpInternal_Implementation() const override;`.
> Doc : [UFunctions (BlueprintNativeEvent)](https://dev.epicgames.com/documentation/en-us/unreal-engine/ufunctions-in-unreal-engine), [Characters](https://dev.epicgames.com/documentation/en-us/unreal-engine/characters-in-unreal-engine).

<a id="notion-beginplay"></a>
> **Notion nouvelle — `BeginPlay()`**
> Appelée **une fois**, quand l'acteur (ou le composant) entre réellement en jeu (début de partie, ou apparition). C'est le premier moment où le monde, le contrôleur et les autres acteurs existent : c'est là qu'on met l'initialisation de jeu (contrairement au [constructeur](#notion-createdefaultsubobject)). En Blueprint, c'est le nœud *Event BeginPlay*, déclenché par le `Super::BeginPlay()` du C++.

---

#### 2.6 `InteractionComponent.h` / `.cpp` — viser, tirer (Pull), saisir (Grab)

**Réglages** (catégorie *Interaction | Pull*, aussi utilisés par Grab) : `TraceDistance` = 300 cm (portée de visée), `HoldDistance` = 150 cm (distance de l'objet tenu devant le joueur), `HoldInterpSpeed` = 6 (vitesse de rattrapage, voir VInterpTo).

**État interne (privé) :** `CurrentFocus` (l'acteur visé, mis à jour à chaque image), `PhysicsHandle` (prêté par le personnage), `bIsGrabbing` (`true` = Grab, `false` = Pull ou rien), `HeldTargetLocation` / `HeldTargetRotation` (la cible lissée vers laquelle on tire l'objet).

##### a) Chaque image : `TickComponent` → `UpdateFocus()` puis `UpdateHold()`
`bCanEverTick = true` dans le constructeur : le composant est appelé à chaque image.

**`UpdateFocus()`** — « qu'est-ce que je regarde ? » :
1. Récupère le `APlayerController` du pawn propriétaire (`Cast<APawn>(GetOwner())->GetController()`). S'il n'y en a pas (pawn pas encore possédé), `CurrentFocus = nullptr`.
2. `PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation)` : position et orientation **de la caméra du joueur** (celles que calcule le Camera Manager).
3. Point d'arrivée : `ViewLocation + ViewRotation.Vector() * TraceDistance` (`Vector()` transforme une rotation en direction de longueur 1).
4. **Sweep** d'une **sphère de rayon 10 cm** sur le canal **Visibility**, en ignorant le personnage (`QueryParams.AddIgnoredActor(GetOwner())`).
5. `CurrentFocus = acteur touché` ou `nullptr`.
Remarque : `BeginPlay()` appelle aussi une fois `UpdateFocus()`.

**`UpdateHold(DeltaTime)`** — « si je tiens quelque chose, où doit-il aller ? » : ne fait rien si le handle ne tient rien. Sinon deux cas, détaillés en c) et d).

##### b) `Pull()` (touche R, *Started*) et `Grab()` (clic gauche, *Started*)
Mêmes vérifications, dans cet ordre — **chaque échec sort sans rien faire**, ce qui évite tout plantage :
1. on ne tient pas déjà un objet, on a un handle, on vise un acteur ;
2. l'acteur visé a un `UInteractionProfileComponent` **et** `CanPerform(Pull)` (resp. `Grab`) ;
3. `GetFocusPrimitive()` = le **composant racine** de l'acteur visé, converti en `UPrimitiveComponent` ; il doit **simuler la physique**.
Puis `PhysicsHandle->GrabComponentAtLocationWithRotation(Primitive, NAME_None, position actuelle, rotation)` : le handle « accroche » l'objet là où il est.
- **Pull** : `bIsGrabbing = false`, la cible de départ = position et rotation actuelles de l'objet.
- **Grab** : `bIsGrabbing = true`, la cible de départ = directement le point à `HoldDistance` devant la caméra ; la rotation de l'objet est mémorisée.

**`ReleaseGrab()`** (R *Completed/Canceled* ; le *Release Grab* du bloc Grab n'est pas relié, [partie A §3.c](editeur-contenu.md#3c--grab-ia_grab-clic-gauche)) : `PhysicsHandle->ReleaseComponent()` si on tient quelque chose, puis `bIsGrabbing = false`. L'objet retombe avec la gravité, sans impulsion. Comme Pull et Grab partagent le même handle, **relâcher R lâche aussi un objet saisi par Grab**.

**`IsHoldingObject()`** : `PhysicsHandle->GrabbedComponent != nullptr`. Utilisé par le personnage pour interdire le saut.

##### c) Tenir en mode Grab (`bIsGrabbing == true`)
À chaque image : point voulu = caméra + direction de visée × `HoldDistance` ; `HeldTargetLocation = FMath::VInterpTo(HeldTargetLocation, Voulu, DeltaTime, HoldInterpSpeed)` ; `PhysicsHandle->SetTargetLocationAndRotation(HeldTargetLocation, HeldTargetRotation)`. L'objet suit **tout** le regard (y compris haut/bas), avec la rotation qu'il avait au moment de la saisie.

##### d) Tenir en mode Pull (`bIsGrabbing == false`) — l'objet glisse au sol
1. On ne garde que le **lacet** (Yaw, rotation gauche/droite) de la caméra : `FRotator(0, Yaw, 0).Vector()` = direction horizontale. Regarder vers le haut ne soulève donc jamais l'objet.
2. Point voulu = position du **personnage** + direction horizontale × `HoldDistance`.
3. **Recherche du sol** sous ce point : `LineTraceMultiByChannel` de +500 cm à −1500 cm à la verticale, canal Visibility, en ignorant le joueur **et** l'objet tenu. On parcourt les impacts et on prend le premier qui **bloque** et qui **ne simule pas** la physique (le sol, un meuble fixe). Hauteur voulue = `ImpactPoint.Z + HeldComponent->Bounds.BoxExtent.Z + 2` (le point d'impact + la demi-hauteur de la boîte englobante de l'objet + 2 cm de marge, pour que l'objet soit posé *sur* le sol et pas dedans).
4. **Pas de sol trouvé** (bord d'un vide) → `ReleaseGrab()` : l'objet tombe.
5. Lissage : `VInterpTo` sur la position, puis `FInterpTo` (la même chose pour un seul nombre) sur Z ; envoi au handle avec la rotation d'origine.

<a id="notion-tick"></a>
> **Notion nouvelle — Tick (`Tick`, `TickComponent`, `DeltaTime`)**
> Le **Tick** est une fonction appelée **à chaque image** (60 fois par seconde à 60 FPS). Un acteur a `Tick(float DeltaTime)`, un composant `TickComponent(DeltaTime, ...)`. Il n'est appelé que si `PrimaryActorTick.bCanEverTick` (acteur) ou `PrimaryComponentTick.bCanEverTick` (composant) vaut `true` **dans le constructeur**. `DeltaTime` = temps écoulé depuis l'image précédente, en secondes : on s'en sert pour que les mouvements aient la même vitesse quel que soit le nombre d'images par seconde.
> Le Tick coûte à chaque image : la règle du projet est **pas de Tick si un événement suffit**. Ici, `UInteractionComponent` en a besoin (la visée change en permanence, l'objet tenu doit suivre) ; `APersonnageNettoyeur`, `APreuveNettoyable` et le profil l'ont désactivé ; `UComposantNettoyage` tique pour mettre à jour le prompt (dette n° 1).
> Doc : [Actor Ticking](https://dev.epicgames.com/documentation/en-us/unreal-engine/actor-ticking-in-unreal-engine).

<a id="notion-cast"></a>
> **Notion nouvelle — `Cast<T>()`**
> `Cast<APawn>(GetOwner())` demande « cet objet est-il un `APawn` (ou un enfant) ? ». Si oui, on obtient un pointeur du bon type ; **sinon `nullptr`**. On teste donc toujours le résultat (`if (OwnerPawn)`, ou l'opérateur `? :` comme dans `OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr`). C'est le même rôle que le nœud Blueprint *Cast To*. Le code l'utilise pour : propriétaire → `APawn`, contrôleur → `APlayerController`, composant racine → `UPrimitiveComponent`, acteur touché → `APreuveNettoyable`.
> Doc : [Objects](https://dev.epicgames.com/documentation/en-us/unreal-engine/objects-in-unreal-engine).

<a id="notion-line-trace-sweep"></a>
> **Notion nouvelle — Line Trace vs Sweep (et `FHitResult`, `FCollisionQueryParams`)**
> Une **trace** est une question posée au moteur physique : « entre le point A et le point B, qu'est-ce que je touche ? ».
> - **Line Trace** (`LineTraceSingleByChannel`) : un **rayon** infiniment fin. Précis, très peu coûteux, mais rate facilement un petit objet ou un bord. Utilisé par `UComposantNettoyage` (visée du nettoyage) et par la recherche du sol de Pull.
> - **Sweep** (`SweepSingleByChannel`) : on fait **glisser une forme** (ici une sphère de 10 cm, `FCollisionShape::MakeSphere(10.f)`) de A à B. Plus « tolérant » : viser à peu près suffit. Utilisé par `UInteractionComponent::UpdateFocus()`.
> - **Single** renvoie le premier objet **bloquant** ; **Multi** (`LineTraceMultiByChannel`) renvoie la liste des objets qui *chevauchent* **jusqu'au** premier bloquant inclus (attention : pas au-delà).
> Le résultat est un **`FHitResult`** : `bBlockingHit` (a-t-on touché ?), `GetActor()` (l'acteur), `GetComponent()` (le composant précis), `ImpactPoint` (le point d'impact). Les réglages passent par **`FCollisionQueryParams`** : `AddIgnoredActor(...)` (ne pas se toucher soi-même), et le 2e paramètre du constructeur `false` = utiliser la collision **simple** (boîtes, sphères) plutôt que les triangles du mesh. `SCENE_QUERY_STAT(PullFocus)` donne juste un nom à la trace pour les outils de profilage.
> L'équivalent Blueprint est le nœud *Line Trace By Channel* (le bloc mort décrit en [partie A §3.a](editeur-contenu.md#3a--essuyer-ia_essuyer-touche-e)).
> Doc : [Traces Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/traces-in-unreal-engine---overview), [Traces with Raycasts](https://dev.epicgames.com/documentation/en-us/unreal-engine/traces-with-raycasts-in-unreal-engine).

<a id="notion-canaux-collision"></a>
> **Notion nouvelle — Canaux de collision (Trace Channel, `ECC_Visibility`)**
> Chaque trace se fait **sur un canal** : `ECC_Visibility` (« ce qui se voit »), `ECC_Camera`, ou des canaux qu'on crée soi-même (Project Settings → Engine → Collision). Chaque objet dit, dans ses *Collision Presets* ([notion éditeur](editeur-contenu.md#notion-collision-physique)), comment il **répond** à chaque canal : *Block* (la trace s'arrête sur lui et le renvoie), *Overlap* (signalé mais la trace continue, seulement en Multi), *Ignore* (invisible pour la trace). `BlockAllDynamic` et `PhysicsActor` bloquent *Visibility* : c'est pour ça que la preuve et les objets Pull sont trouvés. Les **trois** traces du projet utilisent `ECC_Visibility` ; un canal dédié « Interaction » permettrait plus tard de choisir précisément ce qui est visable.
> Doc : [Collision Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine---overview).

<a id="notion-physics-handle"></a>
> **Notion nouvelle — Physics Handle (`UPhysicsHandleComponent`)**
> Un **Physics Handle** est une « poignée » invisible qui tient un objet **simulé** par un ressort, au lieu de le téléporter. On l'utilise en trois appels :
> 1. `GrabComponentAtLocationWithRotation(Composant, Os, Position, Rotation)` : accroche l'objet (il doit simuler la physique, sinon rien ne se passe) ;
> 2. `SetTargetLocationAndRotation(Position, Rotation)` à chaque image : déplace la **cible** ; l'objet est tiré vers elle par la physique, il reste donc soumis aux collisions (il bute contre un mur au lieu de le traverser) ;
> 3. `ReleaseComponent()` : lâche ; `GrabbedComponent` (non nul = on tient quelque chose).
> La raideur du ressort se règle sur le composant dans **Détails** (*Linear/Angular Stiffness*, *Damping*, *Interpolation Speed* — valeurs relevées en [partie A point 2](editeur-contenu.md#point-2--composants-de-bp_firstpersoncharacter-et-ordre)) : plus raide = l'objet colle à la cible, plus amorti = moins d'oscillation.
> Doc : [UPhysicsHandleComponent (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/PhysicsEngine/UPhysicsHandleComponent).

<a id="notion-vinterpto"></a>
> **Notion nouvelle — Interpolation `FMath::VInterpTo` / `FInterpTo`**
> `VInterpTo(Actuel, Cible, DeltaTime, Vitesse)` renvoie un point **entre** `Actuel` et `Cible` : à chaque image, on parcourt une fraction (≈ `Vitesse × DeltaTime`) de la distance **restante**. Résultat : un mouvement rapide quand on est loin, qui ralentit en approchant (« ease out »), et indépendant du nombre d'images par seconde. Avec `HoldInterpSpeed = 6` à 60 FPS, on comble environ 10 % de l'écart à chaque image. `FInterpTo` fait la même chose pour un nombre (`float`) ; il existe aussi `RInterpTo` pour une rotation. Si `Vitesse <= 0`, la fonction renvoie directement la cible.
> Exemple : objet à 100 cm de sa cible → environ 90 cm à l'image suivante, puis 81, 73…
> Doc : [FMath::VInterpTo (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Core/Math/FMath/VInterpTo).

<a id="notion-player-view-point"></a>
> **Notion nouvelle — Point de vue du joueur (`GetPlayerViewPoint`, `FVector`, `FRotator`)**
> `PlayerController->GetPlayerViewPoint(Position, Rotation)` remplit deux variables avec la position et l'orientation de **ce que voit le joueur** (la caméra active, gérée par le *Player Camera Manager*). C'est la source la plus fiable pour viser, quelle que soit la caméra utilisée. `FVector` = un point ou une direction (X, Y, Z en cm, Z vers le haut) ; `FRotator` = une orientation (Pitch = haut/bas, Yaw = gauche/droite, Roll = inclinaison, en degrés) ; `Rotation.Vector()` = la direction « devant » de longueur 1. Les fonctions `GetPlayerViewPoint` et `FindComponentByClass` prennent leurs résultats **par référence** (paramètres de sortie) : on déclare les variables avant l'appel.

---

#### 2.7 `ComposantNettoyage.h` / `.cpp` — viser une preuve, l'essuyer, informer l'interface

Composant **ajouté à la main** dans `BP_FirstPersonCharacter` (et dans le Backup), pas créé en C++ ([partie A point 2](editeur-contenu.md#point-2--composants-de-bp_firstpersoncharacter-et-ordre)).

**Membres :**
- `Personnage` (`ACharacter`) et `CameraJoueur` (`UCameraComponent`) : `UPROPERTY()` publics, **recalculés** à chaque appel (ce ne sont pas de vrais caches).
- `WidgetInteraction` (`UUserWidget`, `BlueprintReadWrite`) : le widget du prompt, donné par le Blueprint via `DefinirWidgetInteraction(Widget)` au BeginPlay ([partie A §4.a](editeur-contenu.md#4a--création-du-widget-event-graph-de-bp_firstpersoncharacter)).
- `OnProgressionNettoyageChanged` : l'événement diffusé (voir notion delegate). Déclaré en tête de fichier par
  ```cpp
  DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressionNettoyageChanged, float, Progression);
  ```

**`TickComponent` (chaque image) — mettre à jour le prompt :**
1. `Personnage = Cast<ACharacter>(GetOwner())`, `CameraJoueur = Personnage->FindComponentByClass<UCameraComponent>()` (la **première** caméra trouvée sur le personnage, ici `FirstPersonCamera`). Sortie si l'un manque.
2. **Line Trace** de la caméra vers l'avant, **300 cm écrits en dur**, canal Visibility, en ignorant le personnage.
3. Si l'acteur touché **est** un `APreuveNettoyable` (`Cast`) → `PeutNettoyer = true`.
4. `MettreAJourTexteInteraction(PeutNettoyer)` → appelle la fonction Blueprint `AfficherInteraction` du widget (voir ProcessEvent).
5. `DrawDebugLine(...)` vert, durée 0 (une image) : **trait de debug dessiné en permanence**, à chaque image.

**`TenterNettoyage()` (touche E, *Started*) — essuyer :**
1. Même préparation et **même Line Trace** que le Tick (le code est copié).
2. Si on touche un `APreuveNettoyable` : `Preuve->Essuyer()`, lecture de `Preuve->ProgressionNettoyage`, `UE_LOG(LogTemp, Warning, TEXT("Progression nettoyage : %f"), Progression)` (ligne jaune dans le journal, c'est elle qui apparaît dans `Saved/Logs/Nettoyeur.log` : 0.25 → 1.0), puis `OnProgressionNettoyageChanged.Broadcast(Progression)`.
3. `DrawDebugLine` vert de 2 secondes.
Le `Broadcast` déclenche le Custom Event `RecevoirProgressionNettoyage` du Blueprint, qui met la barre à jour et affiche « MISSION TERMINÉE ! » à 100 % ([partie A §4.d](editeur-contenu.md#4d--événement-lié-à-onprogressionnettoyagechanged-event-graph-du-personnage)).

**`MettreAJourTexteInteraction(bool Afficher)` — appeler le widget « par son nom » :**
```cpp
UFunction* Fonction = WidgetInteraction->FindFunction(FName("AfficherInteraction"));
if (Fonction)
{
    struct FParametresInteraction { bool Afficher; };   // doit copier exactement les paramètres de la fonction Blueprint
    FParametresInteraction Parametres;
    Parametres.Afficher = Afficher;
    WidgetInteraction->ProcessEvent(Fonction, &Parametres);
}
```
Le C++ ne connaît pas la classe `WBP_Interaction` (elle n'existe qu'en Blueprint) : il cherche donc une fonction **nommée** `AfficherInteraction` et l'exécute avec un bloc de paramètres fabriqué à la main. Si le widget est absent ou si la fonction n'existe pas, rien ne se passe.

<a id="notion-delegate-dynamique-multicast"></a>
> **Notion nouvelle — Delegate dynamique multicast (`DECLARE_DYNAMIC_MULTICAST_DELEGATE_...`, `Broadcast`)**
> Un **delegate** est une variable qui contient « des fonctions à appeler plus tard ». Il permet à un émetteur de **prévenir** sans connaître qui écoute : `UComposantNettoyage` ne sait rien du widget ni de la mission, il dit seulement « la progression a changé ».
> - `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProgressionNettoyageChanged, float, Progression)` **déclare un type** d'événement à **un** paramètre (`float Progression`). Le suffixe change avec le nombre de paramètres (`_TwoParams`…).
> - **Multicast** : plusieurs écouteurs peuvent s'abonner en même temps. **Dynamic** : il passe par la réflexion, donc il est utilisable en Blueprint et sauvegardable (un peu plus lent qu'un delegate C++ simple).
> - On déclare ensuite une **variable** de ce type marquée `UPROPERTY(BlueprintAssignable)` : c'est elle qui apparaît en Blueprint (*Bind Event to On Progression Nettoyage Changed*, voir la [notion Bind Event](editeur-contenu.md#notion-bind-event)) et dans **Détails** du composant (bouton **+** de l'événement).
> - Côté C++ : `OnProgressionNettoyageChanged.Broadcast(0.5f)` appelle **tous** les abonnés avec 0.5 ; s'il n'y en a aucun, il ne se passe rien (pas de plantage). Un abonnement C++ se ferait avec `AddDynamic(this, &UMaClasse::MaFonction)` (la fonction doit être une `UFUNCTION`).
> C'est le mécanisme prévu par le GDD §10.5 pour que l'interface **lise** les changements sans calculer elle-même.
> Doc : [Dynamic Delegates](https://dev.epicgames.com/documentation/en-us/unreal-engine/dynamic-delegates-in-unreal-engine), [Multicast Delegates](https://dev.epicgames.com/documentation/en-us/unreal-engine/multicast-delegates-in-unreal-engine), [Delegates](https://dev.epicgames.com/documentation/en-us/unreal-engine/delegates-and-lamba-functions-in-unreal-engine).

<a id="notion-processevent"></a>
> **Notion nouvelle — `FindFunction` + `ProcessEvent` (appeler une fonction par son nom)**
> Grâce à la réflexion, tout `UObject` peut chercher une de ses fonctions par son **nom** : `FindFunction(FName("AfficherInteraction"))` renvoie un `UFunction*` (une description de la fonction) ou `nullptr`. `ProcessEvent(Fonction, &Parametres)` l'**exécute**, en lui passant un bloc mémoire qui doit contenir les paramètres **exactement dans l'ordre, le type et la taille** attendus par la fonction. C'est ce qui permet au C++ d'appeler une fonction écrite en Blueprint sans connaître sa classe.
> Danger : **rien ne vérifie** que la `struct` fabriquée à la main correspond à la fonction. Si quelqu'un renomme `AfficherInteraction` dans le widget, le prompt cesse de marcher **sans aucune erreur** ; si quelqu'un **ajoute un paramètre** (par ex. un texte), `ProcessEvent` lira de la mémoire au-delà de la `struct` → comportement indéfini, plantage possible. La façon propre, prévue pour plus tard, est une classe C++ parente du widget avec une fonction `BlueprintImplementableEvent`, ou un delegate écouté par le widget (dette n° 5).
> Doc : [UObject::ProcessEvent (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/UObject/UObject/ProcessEvent), [UObject::FindFunction (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/CoreUObject/UObject/UObject/FindFunction).

<a id="notion-ue-log-drawdebug"></a>
> **Notion nouvelle — `UE_LOG` et `DrawDebugLine`**
> `UE_LOG(LogTemp, Warning, TEXT("Progression : %f"), Valeur)` écrit une ligne dans le **journal** (fenêtre **Output Log** de l'éditeur et `Saved/Logs/Nettoyeur.log`). `LogTemp` est une catégorie fourre-tout (on créera une catégorie `LogNettoyeur` plus tard) ; `Warning` l'affiche en jaune ; `%f` est remplacé par un `float` (`%d` entier, `%s` texte avec `*MaChaine`).
> `DrawDebugLine(World, Debut, Fin, Couleur, bPersistant, Duree, Priorite, Epaisseur)` dessine un trait 3D visible en jeu, pour comprendre une trace. Durée 0 = une seule image (donc, appelé dans un Tick, un trait **permanent**). Les dessins de debug sont retirés des builds *Shipping*, mais en développement ils coûtent et encombrent l'écran : la règle du projet est de les mettre **derrière un booléen `bDebug...`** (dette n° 1).
> Doc : [Logging](https://dev.epicgames.com/documentation/en-us/unreal-engine/logging-in-unreal-engine), [DrawDebugLine (API)](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/DrawDebugLine).

---

#### 2.8 Comment les classes collaborent en jeu (chronologie)

| Moment | Qui | Fait quoi |
|---|---|---|
| Chargement de l'éditeur / de la carte | constructeurs C++ | `APersonnageNettoyeur` crée `PhysicsHandle` + `InteractionComponent` et relie les deux ; `APreuveNettoyable` crée `MeshPreuve` (racine) + `ProfilInteractions` [Wipe]. `ComposantNettoyage` vient du Blueprint. |
| Début de partie | Game Mode | fait apparaître `BP_FirstPersonCharacter` ; le PlayerController active `IMC_Default` ([partie A point 5](editeur-contenu.md#point-5--touches-de-imc_default-et-de-imc_mouselook)). |
| `BeginPlay` du personnage (Blueprint) | Event Graph | crée `WBP_Interaction` → `ComposantNettoyage->DefinirWidgetInteraction(widget)` → abonne `RecevoirProgressionNettoyage` à `OnProgressionNettoyageChanged` → affiche le widget. |
| **Chaque image** | `UInteractionComponent::TickComponent` | Sweep sphère 10 cm → `CurrentFocus` ; si un objet est tenu, met à jour la cible du Physics Handle. |
| **Chaque image** | `UComposantNettoyage::TickComponent` | 2e trace (Line Trace) → si preuve visée : `AfficherInteraction(true)` via `ProcessEvent`, sinon `false` ; trait de debug vert. |
| Appui **E** | Blueprint → `UComposantNettoyage::TenterNettoyage()` | 3e trace → `APreuveNettoyable::Essuyer()` → +0.25 ; à 1.0 la preuve se cache et coupe sa collision → `Broadcast(progression)` → Blueprint : barre + « MISSION TERMINÉE ! ». |
| Appui **R** | Blueprint → `UInteractionComponent::Pull()` | `CurrentFocus` → profil `CanPerform(Pull)` → racine simulée → `GrabComponentAtLocationWithRotation`. Ensuite chaque image : glisse au sol devant le joueur. |
| Relâcher **R** | Blueprint → `ReleaseGrab()` | lâche l'objet (même s'il a été pris par Grab). |
| Clic gauche | Blueprint → `UInteractionComponent::Grab()` | idem avec `CanPerform(Grab)`. **Aucun objet n'a [Grab]** aujourd'hui ([partie A point 6](editeur-contenu.md#point-6--réglages-des-objets-de-test)) → ne fait rien. Relâcher le clic n'appelle rien. |
| Saut (Espace) | `ACharacter::Jump` → `CanJumpInternal_Implementation` | refusé tant que `IsHoldingObject()`. |

Qui connaît qui (dépendances `#include` / appels) :
- `APersonnageNettoyeur` → `UInteractionComponent` (crée, interroge `IsHoldingObject`) et `UPhysicsHandleComponent`.
- `UInteractionComponent` → `UInteractionProfileComponent` (lit le profil de la cible), `UPhysicsHandleComponent`, `APlayerController`. Il **ne connaît pas** `APreuveNettoyable`.
- `UComposantNettoyage` → `APreuveNettoyable` (par `Cast` direct), `ACharacter`, `UCameraComponent`, `UUserWidget` (par nom de fonction). Il **ne connaît pas** le profil ni `UInteractionComponent`.
- `APreuveNettoyable` → `UInteractionProfileComponent` (le crée). Il **ne connaît personne** d'autre.

---

#### 2.9 Dette technique observée (constat seulement, rien n'est corrigé)

Les 7 points de `doc/00_etat-projet.md` sont **confirmés**. Ci-dessous, des précisions sur ces points et des points **nouveaux** (marqués 🆕).

| # | Constat | Où | Conséquence |
|---|---|---|---|
| 1 (précisé) | Deux visées : Line Trace dans `UComposantNettoyage` (Tick **et** `TenterNettoyage`, code copié-collé) + Sweep 10 cm dans `UInteractionComponent`. Origine différente (caméra trouvée par `FindComponentByClass` vs `GetPlayerViewPoint`), forme différente, et portée **300 écrite en dur** côté nettoyage alors que `TraceDistance` est réglable côté interaction. `DrawDebugLine` permanent. | `ComposantNettoyage.cpp:55-93`, `:112-156` | Les deux systèmes peuvent désigner des cibles différentes (bord d'objet : le sweep touche, le rayon non). Changer `TraceDistance` ne change pas la portée du nettoyage. |
| 2 | Nettoyage par `Cast<APreuveNettoyable>`, le profil `[Wipe]` n'est jamais lu. | `ComposantNettoyage.cpp:74,130` ; `PreuveNettoyable.cpp:21` | Décocher *Wipe* sur une preuve ne l'empêche pas d'être nettoyée. |
| 3 | État = apparence (cachée + collision coupée à 100 %). | `PreuveNettoyable.cpp:45-52` | — |
| 4 (précisé) | `APersonnageNettoyeur::TenterNettoyage()` vide **et** `BlueprintCallable` dans la même catégorie « Nettoyage » que celle du composant. | `PersonnageNettoyeur.h:33-34` | Dans le menu des nœuds du personnage, deux « Tenter Nettoyage » apparaissent : choisir celui de *Self* donne un E qui ne fait rien, sans erreur. |
| 5 (précisé) | `FindFunction` + `ProcessEvent` avec une `struct` locale, **à chaque image** (recherche par nom + appel Blueprint 60 fois/s, même si l'état n'a pas changé). | `ComposantNettoyage.cpp:82,164-187` | Renommer la fonction du widget casse le prompt silencieusement ; **ajouter un paramètre** au widget = lecture mémoire hors limites (plantage possible). |
| 6 | Encodage de `InteractionTypes.h` | — | **Corrigé dans cette tâche** (§3). |
| 7 | `GetAimDirection()` déclaré et défini mais jamais appelé ; le même calcul « point de vue du joueur » est réécrit 4 fois (`UpdateFocus`, `Grab`, `UpdateHold` ×2). | `InteractionComponent.cpp:34-49` | Code en double. |
| 8 🆕 | Saisie du **composant racine** (`GetFocusPrimitive()` = `GetRootComponent()`), pas du composant réellement touché (`Hit.GetComponent()` n'est pas gardé). | `InteractionComponent.cpp:85-90` | Un objet dont la racine n'est pas le mesh physique (cas de `BP_ObjetGrab_Test1`, racine Geometry Collection vide) ne peut pas être tiré/saisi, même avec le bon profil. |
| 9 🆕 | Recherche du sol de Pull : `LineTraceMulti` sur Visibility **s'arrête au premier objet bloquant**. La boucle qui « saute » les objets physiques ne peut donc jamais trouver le sol *sous* un objet physique. | `InteractionComponent.cpp:239-263` | Tirer un objet par-dessus / contre un autre objet physique (une autre caisse) le fait **lâcher** comme au bord d'un vide. |
| 10 🆕 | En Pull, Z est lissé deux fois (`VInterpTo` sur le vecteur, puis `FInterpTo` sur Z). | `InteractionComponent.cpp:265-268` | La hauteur rattrape plus vite que le déplacement horizontal ; sans gravité, mais déroutant à la lecture. |
| 11 🆕 | Hauteur de pose = `Bounds.BoxExtent.Z` (boîte englobante **alignée sur le monde**). | `InteractionComponent.cpp:250` | Pour un objet tourné, la demi-hauteur change : l'objet peut flotter ou frotter le sol. |
| 12 🆕 | `UInteractionComponent` dépend d'un Physics Handle **injecté par le constructeur du personnage** (`SetPhysicsHandle`, non exposée en Blueprint, pas de recherche de secours en `BeginPlay`). | `InteractionComponent.h:29`, `PersonnageNettoyeur.cpp:13` | Ajouté sur un autre acteur via **+ Ajouter**, le composant serait muet (Pull/Grab sortent sans message). |
| 13 🆕 | `UComposantNettoyage` recalcule `Personnage` et `CameraJoueur` (Cast + `FindComponentByClass`) à chaque image ; ces `UPROPERTY` publiques ressemblent à un cache mais n'en sont pas. Il exige un `ACharacter` et prend **la première** caméra trouvée. | `ComposantNettoyage.cpp:41-53` | Coût inutile ; une 2e caméra sur le personnage pourrait être choisie à la place de la bonne. |
| 14 🆕 | La progression diffusée est celle de **la preuve essuyée**, pas un total ; la preuve n'émet aucun événement elle-même (c'est le composant de nettoyage qui le fait à sa place). | `ComposantNettoyage.cpp:138-142` | Avec plusieurs preuves, la barre saute d'une preuve à l'autre ; c'est la cause côté C++ du « mission terminée à la première preuve » ([partie A §4.d](editeur-contenu.md#4d--événement-lié-à-onprogressionnettoyagechanged-event-graph-du-personnage)). Contraire au GDD §10.5 (la trace notifie). |
| 15 🆕 | Restes de génération / nettoyage : `#include "UObject/StructOnScope.h"` et `"UObject/UnrealType.h"` inutiles ; `BeginPlay`/`Tick`/`SetupPlayerInputComponent` vides dans le personnage ; `ClampMin/ClampMax` sur `ProgressionNettoyage` qui n'est pas éditable (`VisibleAnywhere`) ; catégorie `Interaction|Pull` pour des réglages aussi utilisés par Grab ; `WidgetInteraction` modifiable de deux façons (`BlueprintReadWrite` + `DefinirWidgetInteraction`). | divers | Lisibilité seulement. |
| 16 🆕 | `UE_LOG(LogTemp, Warning, ...)` à chaque essuyage. | `ComposantNettoyage.cpp:140` | Bruit dans le journal (utile en debug, à mettre derrière `bDebug` ou en `Log`/`Verbose`). |

Aucun risque de plantage immédiat n'a été trouvé dans le chemin actuel : tous les pointeurs sont testés avant usage, sauf `GetWorld()` dans `UComposantNettoyage::TenterNettoyage()` (`ComposantNettoyage.cpp:120`) — sans conséquence en jeu (un composant actif a toujours un monde), mais incohérent avec le Tick qui, lui, le teste. Le seul risque réel de corruption mémoire est le n° 5 si la signature Blueprint change.

### 3. Configuration — conversion d'encodage de `InteractionTypes.h`

- **Fichier :** `Source/Nettoyeur/InteractionTypes.h`.
- **Avant → après :** Windows-1252 (« ISO-8859 text » selon `file`) → **UTF-8 sans BOM**, fins de ligne CRLF conservées, pas de saut de ligne final (comme avant).
- **Pourquoi :** UHT et le compilateur (Unreal passe `/utf-8` à MSVC) lisent les sources en UTF-8. En Windows-1252, « î » est l'octet seul `0xEE` ; en UTF-8 cet octet n'est pas un caractère valide, d'où « Tra�ner » dans la liste *Allowed Interactions*. Les autres fichiers accentués du module (`InteractionComponent.cpp`, `PersonnageNettoyeur.cpp`, `InteractionComponent.h`) sont déjà en UTF-8 sans BOM : on s'aligne sur eux.

**Vérification de l'octet avant conversion** (seul octet non-ASCII du fichier) :
```
offset 413 (0x19D) : 0xEE   → « î » en Windows-1252 / ISO-8859-1
00000190: 6179 4e61 6d65 203d 2022 5472 61ee 6e65  ayName = "Tra.ne
```
**Après conversion :**
```
00000190: 6179 4e61 6d65 203d 2022 5472 61c3 ae6e  ayName = "Tra..n
```
`0xEE` est devenu `0xC3 0xAE` (« î » en UTF-8). Taille 603 → 604 octets (le seul octet ajouté). `git diff --stat` : 1 ligne modifiée (ligne 14, `Drag`), aucune autre.

<a id="notion-encodage-utf8"></a>
> **Notion nouvelle — Encodage d'un fichier source (UTF-8 vs Windows-1252)**
> Un fichier texte n'est qu'une suite d'**octets** ; l'**encodage** dit quel caractère correspond à quels octets. Les lettres sans accent (ASCII) s'écrivent pareil partout. Les accents, non : « î » = `EE` en **Windows-1252** (encodage historique de Windows en Europe de l'Ouest), mais `C3 AE` en **UTF-8**. Un programme qui lit en UTF-8 un fichier écrit en Windows-1252 tombe sur un octet invalide et affiche `�`. Règle du projet (`CLAUDE.md` §5) : **tous les sources en UTF-8**. Le **BOM** est une signature optionnelle de 3 octets (`EF BB BF`) au début d'un fichier UTF-8 ; les fichiers du projet n'en ont pas. Pour connaître l'encodage d'un fichier : la barre d'état de Rider / VS Code / Notepad++, ou la commande `file NomDuFichier` dans Git Bash.
> Doc : [Coding Standard (encodage des sources)](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine).

**Comment c'est fait (outil) :** `iconv -f CP1252 -t UTF-8` sur une copie de sauvegarde du fichier, puis remplacement du fichier ; contrôle avec `file`, `xxd` (dump des octets), `cmp -l` et `git diff`.

**Équivalent manuel** (à refaire soi-même si un fichier s'affiche mal) :
- **Visual Studio 2022 :** ouvre `InteractionTypes.h` → vérifie que « Traîner » s'affiche bien **dans VS** (VS a deviné l'ancien encodage ; s'il affiche « Tra�ner », corrige d'abord le caractère à la main) → menu **Fichier (File) → Enregistrer InteractionTypes.h sous… (Save As…)** → petite flèche à droite du bouton **Enregistrer** → **Enregistrer avec encodage… (Save with Encoding…)** → accepte de remplacer → **Encodage : Unicode (UTF-8 sans signature) - Page de codes 65001** ; **Fin de ligne : Windows (CR LF)** → **OK**. (Dans certaines versions, l'option est aussi dans **Fichier → Options d'enregistrement avancées**.)
- **JetBrains Rider :** ouvre le fichier → en bas à droite de la fenêtre, clique sur l'encodage affiché (ex. `windows-1252`) → choisis **UTF-8** → dans la boîte de dialogue, choisis **Convert** (convertir, pas *Reload* qui relirait les mêmes octets autrement) → enregistre.
- **Notepad++ :** menu **Encodage → Convertir en UTF-8** (pas « Encoder en UTF-8 », qui réinterprète sans convertir) → enregistre.
- Ensuite : **fermer l'éditeur Unreal et recompiler** (le texte vient d'un `UMETA`, régénéré par UHT ; Live Coding ne suffit pas).

### 4. Étapes dans l'éditeur
Aucune action dans l'éditeur dans cette partie (l'éditeur est resté ouvert, rien n'a été compilé ni modifié).
Vérification à faire **après** recompilation (QA / développeur) :
1. Ouvre `Content/Nettoyeur/Preuves/BP_ObjetPull_Test`.
2. Panneau **Composants** → clique `InteractionProfile`.
3. **Détails** → catégorie **Interactions** → *Allowed Interactions* → ouvre la liste déroulante de l'élément 0.
4. *Attendu :* la ligne s'affiche **« Traîner »** (et non « Tra�ner »). **Ne change pas** la valeur (laisse *Tirer*), ferme sans enregistrer.

### 5. Compilation et tests
- **Compilation : non réalisée** (consigne : éditeur ouvert, pas de Build.bat ni de Live Coding). La conversion touche un `UMETA` → **fermer l'éditeur puis lancer Build.bat** (commande de `CLAUDE.md` §6). Attendu : compilation sans erreur ni avertissement ; le code C++ n'a pas changé, seul l'octet du « î ».
- **Test en jeu : non réalisé.** Procédure de non-régression : celle de la [partie A §5](editeur-contenu.md#5-compilation-et-tests) (Pull avec R, preuve avec 4 × E) + la vérification « Traîner » du §4 ci-dessus.
- Documentation : les liens vers la doc officielle Epic ont été contrôlés par requête HTTP, mais le site renvoie 200 même pour une page inexistante (application JavaScript) : le contenu des pages **n'a pas pu être vérifié automatiquement**. À cliquer lors de la relecture QA.

### 6. Git
- Branche : `tache/T000-verification-existant` (déjà active, non changée).
- Commits : **aucun** (consigne de l'orchestrateur).

### 7. Résultat et points ouverts
- ✅ Les 6 classes sont documentées (§2.2 à §2.7), leur collaboration aussi (§2.8).
- ✅ `doc/notions.md` : section « C++ Unreal » remplie (26 notions, chacune avec un lien vers son explication ici et, dans l'explication, vers la doc officielle).
- ✅ `InteractionTypes.h` converti en UTF-8, diff limité au seul caractère.
- ⏳ **À faire avec l'éditeur fermé** : compilation Build.bat + vérification de « Traîner » (§4) + non-régression (partie A §5).
- ❓ Pour l'orchestrateur : 9 points de dette **nouveaux** (n° 8 à 16 du §2.9) à ventiler dans T001+ ; les n° 8 (racine vs composant touché) et 9 (sol de Pull sous un objet physique) concernent directement le futur test de Grab/Pull. Le n° 12 rejoint la remarque de la partie A sur `ComposantNettoyage` ajouté en Blueprint : à homogénéiser en T001.
- Aucun asset éditeur nécessaire pour cette partie.
