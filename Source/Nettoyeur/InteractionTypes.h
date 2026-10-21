#pragma once

#include "CoreMinimal.h"
#include "InteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EInteractionType : uint8
{
    Interact UMETA(DisplayName = "Interagir"),
    Wipe     UMETA(DisplayName = "Essuyer"),
    Grab     UMETA(DisplayName = "Saisir"),
    Push     UMETA(DisplayName = "Pousser"),
    Pull     UMETA(DisplayName = "Tirer"),
    Drag     UMETA(DisplayName = "Traîner"),
    Launch   UMETA(DisplayName = "Lancer"),
    Open     UMETA(DisplayName = "Ouvrir"),
    Close    UMETA(DisplayName = "Fermer"),
    Hide     UMETA(DisplayName = "Cacher")
};