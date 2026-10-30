#include "LabSalud.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ULabSalud::ULabSalud()
{
	PrimaryComponentTick.bCanEverTick = true;
	Miembros.Init(true, 4);
}

void ULabSalud::Iniciar(float NuevaVidaMax)
{
	VidaMax = Vida = NuevaVidaMax;
	Veneno = Fuego = Sangrado = Aturdido = Enredado = Lento = Ciego = Invulnerable = Intangible = 0;
	Miembros.Init(true, 4);
}

bool ULabSalud::RecibirGolpe(float Cantidad, AActor* Fuente, ELabGolpe Tipo, const FLabEfectos& E)
{
	if (!Vivo() || Intangible > 0) return false;
	if (bEsJugador && Invulnerable > 0 && Cantidad >= 1) return false;
	if (bEsJugador) Cantidad *= 1.f - FMath::Clamp(Armadura, 0.f, .5f);
	Vida -= Cantidad;
	if (Fuente) UltimoGolpe = Fuente;
	AlDanio.Broadcast(GetOwner(), Cantidad, Fuente);
	if (Vida <= 0) { Morir(Fuente); return true; }
	Aturdido = FMath::Max(Aturdido, E.Aturde);
	Enredado = FMath::Max(Enredado, E.Enreda);
	Lento = FMath::Max(Lento, E.Frena);
	Veneno = FMath::Max(Veneno, E.Veneno);
	Fuego = FMath::Max(Fuego, E.Fuego);
	Sangrado = FMath::Max(Sangrado, E.Sangra);
	Ciego = FMath::Max(Ciego, E.Ciega);
	if (E.Empuje > 0 && Fuente)
	{
		if (ACharacter* C = Cast<ACharacter>(GetOwner()))
		{
			const FVector Dir = (C->GetActorLocation() - Fuente->GetActorLocation()).GetSafeNormal2D();
			C->LaunchCharacter(Dir * Lab::Cm(E.Empuje) * 6.f, true, false);
		}
	}
	if (Cantidad >= 4) IntentarCortar(Cantidad, Fuente, Tipo);
	return true;
}

void ULabSalud::IntentarCortar(float Cantidad, AActor* Fuente, ELabGolpe Tipo)
{
	if (!bPuedePerderMiembros) return;
	const float K = Tipo == ELabGolpe::Filo ? 1.35f : Tipo == ELabGolpe::Contundente ? .55f : Tipo == ELabGolpe::Distancia ? .45f : Tipo == ELabGolpe::Explosion ? 2.f : .8f;
	float P = FMath::Min(.55f, FMath::Max(0.f, Cantidad / VidaMax * 1.7f - .04f)) * K;
	if (bEsJugador) P *= .5f;
	if (VidaMax >= 300) P *= .5f;
	if (FMath::FRand() > P) return;
	TArray<int32> Libres;
	for (int32 i = 0; i < Miembros.Num(); ++i) if (Miembros[i]) Libres.Add(i);
	if (!Libres.Num()) return;
	const int32 M = Libres[FMath::RandRange(0, Libres.Num() - 1)];
	Miembros[M] = false;
	Sangrado = FMath::Max(Sangrado, 7.f);
	AlCortar.Broadcast(GetOwner(), ELabMiembro(M));
}

void ULabSalud::Curar(float Cantidad, bool bCortaSangrado)
{
	if (!Vivo()) return;
	Vida = FMath::Min(VidaMax, Vida + Cantidad);
	if (bCortaSangrado) Sangrado = 0;
}

float ULabSalud::MultVelocidad() const
{
	if (!PuedeMoverse()) return 0;
	const int32 Piernas = int32(Miembros[2]) + int32(Miembros[3]);
	float M = Piernas == 2 ? 1.f : Piernas == 1 ? (bEsJugador ? .6f : .7f) : .35f;
	if (Lento > 0) M *= .5f;
	return M;
}

float ULabSalud::MultDanio() const
{
	const int32 Brazos = int32(Miembros[0]) + int32(Miembros[1]);
	return Brazos == 2 ? 1.f : Brazos == 1 ? .62f : .3f;
}

void ULabSalud::TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(Dt, TickType, ThisTickFunction);
	if (!Vivo()) return;
	for (float* S : {&Veneno, &Fuego, &Aturdido, &Enredado, &Lento, &Ciego, &Invulnerable, &Intangible}) if (*S > 0) *S -= Dt;
	float Danio = 0;
	if (Veneno > 0) Danio += 3 * Dt;
	if (Fuego > 0) Danio += 6 * Dt;
	if (Sangrado > 0) { Sangrado -= Dt; Danio += (bEsJugador ? .9f : 1.4f) * Dt; }
	if (Danio > 0)
	{
		Vida -= Danio;
		if (Vida <= 0) Morir(UltimoGolpe.Get());
	}
	if (ACharacter* C = Cast<ACharacter>(GetOwner()))
		if (!PuedeMoverse()) C->GetCharacterMovement()->StopMovementImmediately();
}

void ULabSalud::Morir(AActor* Asesino)
{
	Vida = 0;
	AlMorir.Broadcast(GetOwner(), Asesino);
}
