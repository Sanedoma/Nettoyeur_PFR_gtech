#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InteractionComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "PersonnageNettoyeur.generated.h"

UCLASS()
class NETTOYEUR_API APersonnageNettoyeur : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

	APersonnageNettoyeur();

protected:
	/** Refuse le saut tant qu'un objet est tenu. */
	virtual bool CanJumpInternal_Implementation() const override;
};
