#include "LabCofre.h"
#include "LabPersonaje.h"
#include "LabEnemigo.h"
#include "LabSalud.h"
#include "LabDatos.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

ALabCofre::ALabCofre()
{
	PrimaryActorTick.bCanEverTick = false;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Caja = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Caja"));
	RootComponent = Caja;
	Tapa = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tapa"));
	Tapa->SetupAttachment(Caja);
	if (Cubo.Succeeded()) { Caja->SetStaticMesh(Cubo.Object); Tapa->SetStaticMesh(Cubo.Object); }
	Caja->SetWorldScale3D(FVector(.8f, .55f, .45f));
	Tapa->SetRelativeLocation(FVector(0, 0, 60));
	Tapa->SetRelativeScale3D(FVector(1.04f, 1.04f, .3f));
	Brillo = CreateDefaultSubobject<UPointLightComponent>(TEXT("Brillo"));
	Brillo->SetupAttachment(Caja);
	Brillo->SetRelativeLocation(FVector(0, 0, 150));
	Brillo->SetIntensity(0); Brillo->SetCastShadows(false); Brillo->SetAttenuationRadius(300);
}

void ALabCofre::Configurar(ELabCofre NuevaClase, ALabEnemigo* Jefe)
{
	Clase = NuevaClase; Sello = Jefe;
	const float K = Clase == ELabCofre::Leyenda ? 1.35f : Clase == ELabCofre::Oro ? 1.15f : 1.f;
	SetActorScale3D(FVector(.8f, .55f, .45f) * K);
	const FLinearColor Madera = Clase == ELabCofre::Leyenda ? FLinearColor(.35f, .16f, .1f) : FLinearColor(.6f, .4f, .19f);
	if (UMaterialInterface* M = Caja->GetMaterial(0))
	{
		UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(M, this);
		D->SetVectorParameterValue(TEXT("Color"), Madera);
		Caja->SetMaterial(0, D);
		UMaterialInstanceDynamic* T = UMaterialInstanceDynamic::Create(M, this);
		T->SetVectorParameterValue(TEXT("Color"), Clase >= ELabCofre::Oro ? FLinearColor(.91f, .72f, .19f) : Madera * .8f);
		Tapa->SetMaterial(0, T);
	}
	if (Clase >= ELabCofre::Oro) { Brillo->SetIntensity(Clase == ELabCofre::Leyenda ? 2500 : 1500); Brillo->SetLightColor(Clase == ELabCofre::Leyenda ? FLinearColor(1, .55f, .16f) : FLinearColor(1, .82f, .27f)); }
}

bool ALabCofre::Sellado() const
{
	return Sello.IsValid() && Sello->Salud && Sello->Salud->Vivo();
}

FString ALabCofre::TextoAbrir() const
{
	if (Sellado()) return FString::Printf(TEXT("Cofre sellado: derrotá a %s"), *Sello->Nombre);
	return Clase == ELabCofre::Leyenda ? TEXT("E · Abrir el cofre legendario") : Clase == ELabCofre::Oro ? TEXT("E · Abrir el cofre de oro") : TEXT("E · Abrir cofre");
}

int32 ALabCofre::SortearRareza(bool bRico) const
{
	const float R = FMath::FRand();
	if (bRico) return R < .1f ? 3 : R < .35f ? 2 : R < .75f ? 1 : 0;
	return R < .03f ? 3 : R < .13f ? 2 : R < .4f ? 1 : 0;
}

FName ALabCofre::SortearArma(bool bRico) const
{
	const FLabDatos& D = FLabDatos::Get();
	float Total = 0;
	for (const auto& B : D.Botin) { const FLabArma* A = D.Armas.Find(B.Key); if (A && (A->Ranura == TEXT("melee") || A->Ranura == TEXT("ranged"))) Total += B.Value.Peso; }
	auto Uno = [&]() -> FName
	{
		float R = FMath::FRand() * Total;
		for (const auto& B : D.Botin)
		{
			const FLabArma* A = D.Armas.Find(B.Key);
			if (!A || (A->Ranura != TEXT("melee") && A->Ranura != TEXT("ranged"))) continue;
			R -= B.Value.Peso;
			if (R <= 0) return B.Key;
		}
		return TEXT("espada");
	};
	FName A = Uno();
	if (bRico) { const FName B = Uno(); const FLabArma* AB = D.Armas.Find(B); const FLabArma* AA = D.Armas.Find(A); if (AB && AA && AB->Nivel > AA->Nivel) A = B; }
	return A;
}

void ALabCofre::Abrir(ALabPersonaje* Quien)
{
	if (bAbierto || !Quien) return;
	if (Sellado()) { Quien->Mensaje(TextoAbrir()); return; }
	bAbierto = true;
	Tapa->SetRelativeRotation(FRotator(0, 0, -100));
	Tapa->SetRelativeLocation(FVector(0, -55, 90));
	Brillo->SetIntensity(0);
	const bool Rico = Clase != ELabCofre::Comun;
	int32 Oro = FMath::RandRange(10, 49) + (Rico ? 40 : 0);
	if (Clase == ELabCofre::Oro) Oro = FMath::RandRange(80, 139);
	if (Clase == ELabCofre::Leyenda) Oro = FMath::RandRange(160, 259);
	Quien->Oro += Oro;
	if (Clase >= ELabCofre::Oro) Quien->Pociones += Clase == ELabCofre::Leyenda ? 2 : 1;
	else if (FMath::FRand() < .3f) Quien->Pociones++;
	// el arma: si es mejor que la que tenés, te la ponés (el inventario completo está en la fase 4 de MIGRACION.md)
	const FName Arma = SortearArma(Rico);
	const int32 Rareza = Clase == ELabCofre::Leyenda ? 3 : Clase == ELabCofre::Oro ? FMath::Max(2, SortearRareza(true)) : SortearRareza(Rico);
	static const TCHAR* Rar[] = {TEXT(""), TEXT(" rara"), TEXT(" épica"), TEXT(" legendaria")};
	const FLabArma* Def = FLabDatos::Get().Armas.Find(Arma);
	const bool Mejor = Def && Def->Nivel + Rareza * .6f > Quien->ArmaActual().Nivel;
	if (Mejor) Quien->Equipar(Arma);
	Quien->Mensaje(FString::Printf(TEXT("%s: %d de oro%s%s."), Clase == ELabCofre::Leyenda ? TEXT("¡Cofre legendario!") : Clase == ELabCofre::Oro ? TEXT("¡Cofre de oro!") : TEXT("Cofre"), Oro,
		Def ? TEXT(" y ") : TEXT(""), Def ? *(Def->Nombre + Rar[Rareza] + (Mejor ? TEXT(" (te la pusiste)") : TEXT(" (la tuya es mejor)"))) : TEXT("")));
}
