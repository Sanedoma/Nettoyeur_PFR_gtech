
#include "InteractionProfileComponent.h"

UInteractionProfileComponent::UInteractionProfileComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UInteractionProfileComponent::CanPerform(EInteractionType Interaction) const
{
    return AllowedInteractions.Contains(Interaction);
}
