#include "WidgetInteraction.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "InteractionComponent.h"

#define LOCTEXT_NAMESPACE "WidgetInteraction"

UWidgetInteraction::UWidgetInteraction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Valeurs par défaut, modifiables dans les Class Defaults de WBP_Interaction.
	// L'ordre d'ajout est l'ordre d'affichage des lignes.
	TextesInteractions.Add(EInteractionType::Wipe, LOCTEXT("Wipe", "E : Nettoyer"));
	TextesInteractions.Add(EInteractionType::Pull, LOCTEXT("Pull", "R (maintenir) : Tirer"));
	TextesInteractions.Add(EInteractionType::Grab, LOCTEXT("Grab", "Clic gauche : Saisir"));
}

void UWidgetInteraction::NativeConstruct()
{
	Super::NativeConstruct();

	// Le widget est créé par le personnage avec « Owning Player » = son PlayerController :
	// le pawn propriétaire est donc le personnage qui porte l'InteractionComponent.
	APawn* Pawn = GetOwningPlayerPawn();
	ComposantInteraction = Pawn ? Pawn->FindComponentByClass<UInteractionComponent>() : nullptr;

	if (!ComposantInteraction)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("WidgetInteraction : aucun InteractionComponent sur le pawn proprietaire (%s), le prompt reste masque."),
			*GetNameSafe(Pawn));
	}
	else
	{
		// AddUniqueDynamic : NativeConstruct peut être rappelé si le widget est retiré
		// puis remis à l'écran ; on ne veut pas être abonné deux fois.
		ComposantInteraction->OnFocusChanged.AddUniqueDynamic(
			this, &UWidgetInteraction::GererChangementCible);
		ComposantInteraction->OnHoldChanged.AddUniqueDynamic(
			this, &UWidgetInteraction::GererChangementTenue);
	}

	// Les événements ne partent qu'aux changements : on affiche tout de suite l'état courant.
	RafraichirPrompt();
}

void UWidgetInteraction::NativeDestruct()
{
	if (ComposantInteraction)
	{
		ComposantInteraction->OnFocusChanged.RemoveDynamic(
			this, &UWidgetInteraction::GererChangementCible);
		ComposantInteraction->OnHoldChanged.RemoveDynamic(
			this, &UWidgetInteraction::GererChangementTenue);
	}
	ComposantInteraction = nullptr;

	Super::NativeDestruct();
}

void UWidgetInteraction::GererChangementCible(AActor* NouvelleCible)
{
	RafraichirPrompt();
}

void UWidgetInteraction::GererChangementTenue(bool bTientObjet)
{
	RafraichirPrompt();
}

void UWidgetInteraction::RafraichirPrompt()
{
	// Une ligne par interaction présente dans la table ET autorisée par le profil
	// de la cible. CanFocusPerform renvoie faux si rien n'est visé ou sans profil.
	TArray<FText> Lignes;
	const bool bTientObjet = ComposantInteraction && ComposantInteraction->IsHoldingObject();
	if (ComposantInteraction && !bTientObjet)
	{
		for (const TPair<EInteractionType, FText>& Entree : TextesInteractions)
		{
			if (!Entree.Value.IsEmpty() && ComposantInteraction->CanFocusPerform(Entree.Key))
			{
				Lignes.Add(Entree.Value);
			}
		}
	}

	if (TexteInteraction)
	{
		if (Lignes.Num() > 0)
		{
			// Retour à la ligne entre deux interactions (séparateur non traduit).
			TexteInteraction->SetText(FText::Join(FText::AsCultureInvariant(TEXT("\n")), Lignes));
			TexteInteraction->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TexteInteraction->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (BarProgression)
	{
		const bool bAfficherBarre = ComposantInteraction && !bTientObjet
			&& ComposantInteraction->CanFocusPerform(EInteractionType::Wipe);
		BarProgression->SetVisibility(
			bAfficherBarre ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

#undef LOCTEXT_NAMESPACE
