#include "ComposantNettoyage.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"
#include "InteractionComponent.h"
#include "InteractionTypes.h"
#include "PreuveNettoyable.h"
#include "UObject/UnrealType.h"

UComposantNettoyage::UComposantNettoyage()
{
	// Plus de Tick : le prompt est mis à jour sur l'événement OnFocusChanged.
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
		return;
	}

	ComposantInteraction->OnFocusChanged.AddDynamic(
		this, &UComposantNettoyage::GererChangementCible);
}

void UComposantNettoyage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ComposantInteraction)
	{
		ComposantInteraction->OnFocusChanged.RemoveDynamic(
			this, &UComposantNettoyage::GererChangementCible);
	}

	Super::EndPlay(EndPlayReason);
}

bool UComposantNettoyage::PeutNettoyerCible() const
{
	return ComposantInteraction
		&& ComposantInteraction->CanFocusPerform(EInteractionType::Wipe);
}

void UComposantNettoyage::GererChangementCible(AActor* NouvelleCible)
{
	MettreAJourTexteInteraction(PeutNettoyerCible());
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
	WidgetInteraction = Widget;

	// Le prompt n'est mis à jour que lorsque la cible change : on pousse donc
	// tout de suite l'état courant au nouveau widget.
	MettreAJourTexteInteraction(PeutNettoyerCible());
}

void UComposantNettoyage::MettreAJourTexteInteraction(bool Afficher)
{
	if (!WidgetInteraction)
	{
		return;
	}

	UFunction* Fonction = WidgetInteraction->FindFunction(
		FName(TEXT("AfficherInteraction")));
	if (!Fonction)
	{
		return;
	}

	// Garde-fou : on vérifie que la fonction Blueprint a bien UN seul paramètre,
	// un booléen nommé « Afficher », avant de lui passer un bloc de paramètres.
	// Si sa signature change, on prévient dans le journal au lieu de lire
	// de la mémoire au hasard.
	const FBoolProperty* ParametreAfficher =
		CastField<FBoolProperty>(Fonction->FindPropertyByName(TEXT("Afficher")));
	if (Fonction->NumParms != 1 || !ParametreAfficher
		|| !ParametreAfficher->HasAnyPropertyFlags(CPF_Parm))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ComposantNettoyage : %s::AfficherInteraction doit avoir un seul parametre booleen 'Afficher'."),
			*GetNameSafe(WidgetInteraction->GetClass()));
		return;
	}

	// Bloc de paramètres de la taille exacte attendue par la fonction.
	uint8* Parametres = static_cast<uint8*>(FMemory_Alloca(Fonction->ParmsSize));
	FMemory::Memzero(Parametres, Fonction->ParmsSize);
	ParametreAfficher->SetPropertyValue_InContainer(Parametres, Afficher);

	WidgetInteraction->ProcessEvent(Fonction, Parametres);
}
