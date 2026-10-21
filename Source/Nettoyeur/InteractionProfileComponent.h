#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionTypes.h"
#include "InteractionProfileComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NETTOYEUR_API UInteractionProfileComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractionProfileComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactions")
    TArray<EInteractionType> AllowedInteractions;

    UFUNCTION(BlueprintPure, Category = "Interactions")
    bool CanPerform(EInteractionType Interaction) const;
};
