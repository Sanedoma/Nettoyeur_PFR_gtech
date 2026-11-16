#include "ComposantNettoyage.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "InteractionComponent.h"
#include "InteractionTypes.h"
#include "PreuveNettoyable.h"

UComposantNettoyage::UComposantNettoyage()
{
	// Pas de Tick : le nettoyage ne réagit qu'à l'appui sur E.
	PrimaryComponentTick.bCanEverTick = false;
}

void UComposantNettoyage::BeginPlay()
{
	Super::BeginPlay();

	AActor* Proprietaire = GetOwner();
	ComposantInteraction = Proprietaire
		? Proprietaire->FindComponentByClass<UInteractionComponent>()
		: nullptr;

	if (!ComposantInteraction)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ComposantNettoyage : aucun InteractionComponent sur %s, le nettoyage est inactif."),
			*GetNameSafe(Proprietaire));
	}
}

bool UComposantNettoyage::PeutNettoyerCible() const
{
	return ComposantInteraction
		&& ComposantInteraction->CanFocusPerform(EInteractionType::Wipe);
}

void UComposantNettoyage::TenterNettoyage()
{
	// La décision vient du profil (Wipe autorisé), pas du type de l'acteur.
	if (!PeutNettoyerCible())
	{
		return;
	}

	// Pour l'instant seule APreuveNettoyable sait s'essuyer (évolue en T003/T004).
	APreuveNettoyable* Preuve =
		Cast<APreuveNettoyable>(ComposantInteraction->GetCurrentFocus());
	if (!Preuve)
	{
		return;
	}

	Preuve->Essuyer();

	const float Progression = Preuve->ProgressionNettoyage;

	if (bAfficherDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("Progression nettoyage : %f"), Progression);
	}

	OnProgressionNettoyageChanged.Broadcast(Progression);
}

void UComposantNettoyage::DefinirWidgetInteraction(UUserWidget* Widget)
{
	// Simple mémorisation : le prompt est géré par UWidgetInteraction, qui écoute
	// lui-même OnFocusChanged. Plus aucun appel de fonction Blueprint par son nom.
	WidgetInteraction = Widget;
}
