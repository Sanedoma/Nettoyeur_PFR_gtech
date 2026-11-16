#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionTypes.h"
#include "InteractionComponent.generated.h"

class UPhysicsHandleComponent;
class UPrimitiveComponent;

/** Diffusé quand l'acteur visé change (NewFocus vaut nullptr quand on ne vise plus rien). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusChanged, AActor*, NewFocus);

/** Diffusé quand le joueur commence (true) ou cesse (false) de tenir un objet (Pull ou Grab). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoldChanged, bool, bIsHolding);

/**
 * Seule source de la visée du joueur : un sweep par image désigne l'acteur visé
 * (CurrentFocus). Gère aussi la saisie physique Pull / Grab des objets qui l'autorisent.
 * Les autres composants (nettoyage…) lisent GetCurrentFocus() ou écoutent OnFocusChanged
 * au lieu de lancer leur propre rayon.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NETTOYEUR_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionComponent();

    /** Portée de la visée (cm), partagée par toutes les interactions. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Visee", meta = (ClampMin = "1.0"))
    float TraceDistance = 300.f;

    /** Distance devant le joueur à laquelle un objet tenu est maintenu (Pull et Grab). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Tenir", meta = (ClampMin = "1.0"))
    float HoldDistance = 150.f;

    /** Vitesse à laquelle l'objet tenu rejoint sa position voulue (Pull et Grab). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Tenir", meta = (ClampMin = "0.1"))
    float HoldInterpSpeed = 6.f;

    /**
     * Impulsion du lancer (kg·cm/s), appliquée dans la direction de la caméra.
     * Dépend de la masse : vitesse de départ = LaunchImpulse / masse (un objet lourd part moins vite).
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Lancer", meta = (ClampMin = "0.0"))
    float LaunchImpulse = 15000.f;

    /** Plafond de la vitesse de départ d'un objet lancé (cm/s), quelle que soit sa masse. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Lancer", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
    float MaxLaunchSpeed = 1500.f;

    /** Dessine la visée à chaque image (trait vert = touche, rouge = rien). Faux par défaut. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Debug")
    bool bAfficherDebug = false;

    /** Diffusé à chaque changement de cible visée. */
    UPROPERTY(BlueprintAssignable, Category = "Interaction|Visee")
    FOnFocusChanged OnFocusChanged;

    /** Diffusé quand un objet est saisi ou relâché (le prompt se masque pendant qu'on tient). */
    UPROPERTY(BlueprintAssignable, Category = "Interaction|Tenir")
    FOnHoldChanged OnHoldChanged;

    /**
     * Le Physics Handle est normalement donné par le constructeur du personnage.
     * Sinon, BeginPlay cherche un Physics Handle sur l'acteur propriétaire.
     */
    void SetPhysicsHandle(UPhysicsHandleComponent* InPhysicsHandle);

    /** Acteur actuellement visé (nullptr si rien). */
    UFUNCTION(BlueprintPure, Category = "Interaction|Visee")
    AActor* GetCurrentFocus() const;

    /** Vrai si l'acteur visé a un profil d'interactions qui autorise Interaction. */
    UFUNCTION(BlueprintPure, Category = "Interaction|Visee")
    bool CanFocusPerform(EInteractionType Interaction) const;

    /** Saisit la cible actuelle si son profil autorise Pull et si elle simule la physique. */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Pull")
    void Pull();

    /** Saisit la cible actuelle devant la caméra si son profil autorise Grab. */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Grab")
    void Grab();

    /** Relâche simplement l'objet tenu (Pull ou Grab), sans impulsion de lancer. */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Tenir")
    void ReleaseGrab();

    /**
     * Lance l'objet tenu en Grab si son profil autorise Launch : le relâche puis le pousse
     * dans la direction de la caméra. Sans effet (l'objet reste tenu) si rien n'est tenu,
     * si l'objet est tiré en Pull ou si son profil n'autorise pas Launch.
     */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Lancer")
    void Launch();

    UFUNCTION(BlueprintPure, Category = "Interaction|Tenir")
    bool IsHoldingObject() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TObjectPtr<AActor> CurrentFocus = nullptr;

    /** Composant précis touché par la visée (un acteur peut avoir plusieurs composants). */
    UPROPERTY()
    TObjectPtr<UPrimitiveComponent> CurrentFocusComponent = nullptr;

    UPROPERTY()
    TObjectPtr<UPhysicsHandleComponent> PhysicsHandle = nullptr;

    /** true = l'objet tenu l'a été par Grab ; false = par Pull, ou rien n'est tenu. */
    bool bIsGrabbing = false;

    FVector HeldTargetLocation = FVector::ZeroVector;
    FRotator HeldTargetRotation = FRotator::ZeroRotator;

    void UpdateFocus();
    void SetCurrentFocus(AActor* NewFocus);
    void UpdateHold(float DeltaTime);

    /**
     * Point de vue du joueur (caméra). Sans PlayerController, renvoie la position et
     * la rotation de l'acteur propriétaire et retourne false.
     */
    bool GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const;

    /**
     * Composant à saisir sur la cible : le composant touché s'il simule la physique,
     * sinon le composant racine (ancien comportement). nullptr si aucun ne convient.
     */
    UPrimitiveComponent* GetFocusPrimitive() const;

    /** Vrai si Actor a un profil d'interactions qui autorise Interaction. */
    static bool ActorAllows(const AActor* Actor, EInteractionType Interaction);
};
