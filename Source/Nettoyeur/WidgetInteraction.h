#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionTypes.h"
#include "WidgetInteraction.generated.h"

class UInteractionComponent;
class UProgressBar;
class UTextBlock;

/**
 * Classe parente C++ de WBP_Interaction : le prompt d'interaction.
 * Écoute l'UInteractionComponent du pawn propriétaire et affiche une ligne par
 * interaction que la cible visée autorise (Wipe, Pull, Grab…), d'après la table
 * TextesInteractions réglable dans les Class Defaults du Widget Blueprint.
 * La barre de progression n'est visible que si la cible autorise Wipe ; son
 * remplissage reste piloté par la fonction Blueprint MettreAJourProgression.
 */
UCLASS()
class NETTOYEUR_API UWidgetInteraction : public UUserWidget
{
	GENERATED_BODY()

public:
	UWidgetInteraction(const FObjectInitializer& ObjectInitializer);

	/**
	 * Texte affiché pour chaque interaction autorisée par la cible, dans l'ordre de la table.
	 * Une interaction absente de la table n'affiche rien.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Prompt")
	TMap<EInteractionType, FText> TextesInteractions;

protected:
	/** Texte du prompt (élément « TexteInteraction » du Concepteur, obligatoire). */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Prompt", meta = (BindWidget))
	TObjectPtr<UTextBlock> TexteInteraction;

	/** Barre de nettoyage (élément « BarProgression » du Concepteur, obligatoire). */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|Prompt", meta = (BindWidget))
	TObjectPtr<UProgressBar> BarProgression;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	/** Composant de visée du pawn propriétaire, trouvé au NativeConstruct. */
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> ComposantInteraction;

	/** Abonné à UInteractionComponent::OnFocusChanged. */
	UFUNCTION()
	void GererChangementCible(AActor* NouvelleCible);

	/** Abonné à UInteractionComponent::OnHoldChanged. */
	UFUNCTION()
	void GererChangementTenue(bool bTientObjet);

	/** Recalcule le texte et la visibilité du prompt à partir de l'état actuel de la visée. */
	void RafraichirPrompt();
};
