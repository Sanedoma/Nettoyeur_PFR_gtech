#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UPhysicsHandleComponent;
class UPrimitiveComponent;

/** Gère la détection et la saisie physique Pull des objets qui l'autorisent. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NETTOYEUR_API UInteractionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Pull", meta = (ClampMin = "1.0"))
    float TraceDistance = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Pull", meta = (ClampMin = "1.0"))
    float HoldDistance = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Pull", meta = (ClampMin = "0.1"))
    float HoldInterpSpeed = 6.f;

    /** Le Physics Handle est créé par le personnage propriétaire. */
    void SetPhysicsHandle(UPhysicsHandleComponent* InPhysicsHandle);

    /** Saisit la cible actuelle si son profil autorise Pull et si elle simule la physique. */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Pull")
    void Pull();

    /** Saisit la cible actuelle devant la caméra si son profil autorise Grab. */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Grab")
    void Grab();

    /** Relâche simplement l'objet tenu, sans impulsion de lancer. */
    UFUNCTION(BlueprintCallable, Category = "Interaction|Pull")
    void ReleaseGrab();

    UFUNCTION(BlueprintPure, Category = "Interaction|Pull")
    bool IsHoldingObject() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    UPROPERTY()
    TObjectPtr<AActor> CurrentFocus = nullptr;

    UPROPERTY()
    TObjectPtr<UPhysicsHandleComponent> PhysicsHandle = nullptr;

    bool bIsGrabbing = false;

    FVector HeldTargetLocation = FVector::ZeroVector;
    FRotator HeldTargetRotation = FRotator::ZeroRotator;

    void UpdateFocus();
    void UpdateHold(float DeltaTime);
    FVector GetAimDirection() const;
    UPrimitiveComponent* GetFocusPrimitive() const;
};
