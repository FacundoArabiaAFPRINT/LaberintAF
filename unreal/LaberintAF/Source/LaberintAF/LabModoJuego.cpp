#include "LabModoJuego.h"
#include "LaberintAF.h"
#include "LabLaberinto.h"
#include "LabPersonaje.h"
#include "LabEnemigo.h"
#include "LabCofre.h"
#include "LabSalud.h"
#include "LabDatos.h"
#include "LabControlador.h"
#include "LabHUD.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ALabModoJuego::ALabModoJuego()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = ALabPersonaje::StaticClass();
	PlayerControllerClass = ALabControlador::StaticClass();
	HUDClass = ALabHUD::StaticClass();
}

ALabModoJuego* ALabModoJuego::Get(const UObject* Contexto)
{
	const UWorld* W = Contexto ? Contexto->GetWorld() : nullptr;
	return W ? Cast<ALabModoJuego>(W->GetAuthGameMode()) : nullptr;
}

ALabLaberinto* ALabModoJuego::Laberinto(const UObject* Contexto)
{
	ALabModoJuego* M = Get(Contexto);
	return M ? M->Mapa.Get() : nullptr;
}

void ALabModoJuego::Asegurar()
{
	if (Mapa) return;
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Mapa = GetWorld()->SpawnActor<ALabLaberinto>(ALabLaberinto::StaticClass(), FTransform::Identity, P);
	Mapa->Generar();
	Mapa->Construir();
	Ambiente();
}

void ALabModoJuego::Ambiente()
{
	UWorld* W = GetWorld();
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// luna: luz azulada y débil; el farol es lo que ilumina de verdad
	if (ADirectionalLight* Luna = W->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FTransform(FRotator(-55, 35, 0)), P))
	{
		UDirectionalLightComponent* C = Cast<UDirectionalLightComponent>(Luna->GetLightComponent());
		C->SetMobility(EComponentMobility::Movable);
		C->SetIntensity(.35f);
		C->SetLightColor(FLinearColor(.55f, .62f, 1.f));
		C->SetCastShadows(true);
	}
	// el cielo de noche: luz ambiente muy baja (sin atmósfera el cielo capturado es casi negro, que es lo que queremos)
	if (ASkyLight* Cielo = W->SpawnActorDeferred<ASkyLight>(ASkyLight::StaticClass(), FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		USkyLightComponent* C = Cielo->GetLightComponent();
		C->SetMobility(EComponentMobility::Movable);
		C->bRealTimeCapture = true;
		Cielo->FinishSpawning(FTransform::Identity);
		C->SetIntensity(.25f);
		C->SetLightColor(FLinearColor(.4f, .45f, .7f));
		C->RecaptureSky();
	}
	// niebla volumétrica: la luz del farol se ve en el aire
	if (AExponentialHeightFog* Niebla = W->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FTransform::Identity, P))
	{
		UExponentialHeightFogComponent* C = Niebla->GetComponent();
		C->SetFogDensity(.035f);
		C->SetFogHeightFalloff(.6f);
		C->SetFogInscatteringColor(FLinearColor(.05f, .06f, .09f));
		C->SetVolumetricFog(true);
		C->SetVolumetricFogScatteringDistribution(.6f);
		C->SetVolumetricFogExtinctionScale(1.4f);
	}
	if (APostProcessVolume* PP = W->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform::Identity, P))
	{
		PP->bUnbound = true;
		FPostProcessSettings& S = PP->Settings;
		S.bOverride_AutoExposureMethod = true; S.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
		S.bOverride_AutoExposureBias = true; S.AutoExposureBias = 9.5f;
		S.bOverride_VignetteIntensity = true; S.VignetteIntensity = .7f;
		S.bOverride_FilmGrainIntensity = true; S.FilmGrainIntensity = .2f;
		S.bOverride_ColorSaturation = true; S.ColorSaturation = FVector4(.85f, .85f, .85f, 1);
		S.bOverride_SceneFringeIntensity = true; S.SceneFringeIntensity = .6f;
	}
}

AActor* ALabModoJuego::ChoosePlayerStart_Implementation(AController* Player)
{
	Asegurar();
	const FVector Salida = Mapa->Salida;
	const FVector Inicio = Mapa->PuntoLibre([&](const FVector& P) {
		const FLabSala* S = Mapa->SalaEn(P);
		return FVector::Dist2D(P, Salida) > Lab::Cm(380) && !(S && (S->Tipo == TEXT("boss") || S->Tipo == TEXT("vault") || S->Tipo == TEXT("merchant")));
	});
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	return GetWorld()->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FTransform(Inicio + FVector(0, 0, 100)), P);
}

void ALabModoJuego::StartPlay()
{
	Asegurar();
	Super::StartPlay();
	Poblar();
}

ALabPersonaje* ALabModoJuego::Jugador() const
{
	const APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	return PC ? Cast<ALabPersonaje>(PC->GetPawn()) : nullptr;
}

FVector ALabModoJuego::LugarSalida() const { return Mapa ? Mapa->Salida : FVector::ZeroVector; }

ALabEnemigo* ALabModoJuego::CrearBestia(const FString& Id, const FVector& Lugar, bool bElite)
{
	const FLabBestia* B = FLabDatos::Get().Bestias.Find(FName(*Id));
	if (!B) { UE_LOG(LogLab, Warning, TEXT("No hay bestia «%s» en Bestias.csv"), *Id); return nullptr; }
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ALabEnemigo* E = GetWorld()->SpawnActor<ALabEnemigo>(ALabEnemigo::StaticClass(), FTransform(Lugar + FVector(0, 0, 100)), P);
	if (E) E->ConfigurarBestia(Id, *B, bElite);
	return E;
}

ALabCofre* ALabModoJuego::CrearCofre(const FVector& Lugar, uint8 Clase, ALabEnemigo* Jefe)
{
	FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALabCofre* C = GetWorld()->SpawnActor<ALabCofre>(ALabCofre::StaticClass(), FTransform(Lugar + FVector(0, 0, 25)), P);
	if (C) C->Configurar(ELabCofre(Clase), Jefe);
	return C;
}

void ALabModoJuego::Poblar()
{
	ALabPersonaje* J = Jugador();
	if (!J)
	{
		FTimerHandle H; GetWorldTimerManager().SetTimer(H, this, &ALabModoJuego::Poblar, .1f, false);
		return;
	}
	if (bPoblada) return;
	bPoblada = true;
	FRandomStream& R = Mapa->Azar();
	auto LejosDe = [&](const FVector& P, float Unidades) { return FVector::Dist2D(P, J->GetActorLocation()) > Lab::Cm(Unidades); };
	auto Sala = [&](const FVector& P, std::initializer_list<const TCHAR*> Tipos) { const FLabSala* S = Mapa->SalaEn(P); if (!S) return false; for (const TCHAR* T : Tipos) if (S->Tipo == T) return true; return false; };

	// salas especiales: jefes sin repetir con su cofre legendario sellado, bóvedas con cofre de oro y tres élites
	TArray<FString> Jefes = {TEXT("jefe"), TEXT("carnicero"), TEXT("bruja")};
	for (int32 i = Jefes.Num() - 1; i > 0; --i) Jefes.Swap(i, R.RandRange(0, i));
	int32 iJefe = 0;
	for (const FLabSala& S : Mapa->Salas)
	{
		if (S.Tipo == TEXT("boss") && iJefe < Jefes.Num())
		{
			ALabEnemigo* Jefe = CrearBestia(Jefes[iJefe++], S.Centro, false);
			CrearCofre(FVector(S.Centro.X, S.Min.Y + Lab::Cm(14), 0), uint8(ELabCofre::Leyenda), Jefe);
			CrearCofre(FVector(S.Centro.X - Lab::Cm(26), S.Min.Y + Lab::Cm(12), 0), uint8(ELabCofre::Rico));
			CrearCofre(FVector(S.Centro.X + Lab::Cm(26), S.Min.Y + Lab::Cm(12), 0), uint8(ELabCofre::Rico));
		}
		else if (S.Tipo == TEXT("vault"))
		{
			CrearCofre(S.Centro, uint8(ELabCofre::Oro));
			const TArray<FString> Elites = {TEXT("perro"), TEXT("zombi"), TEXT("demonio")};
			for (int32 k = 0; k < 3; ++k)
			{
				const float A = k / 3.f * 2 * PI;
				CrearBestia(Elites[R.RandRange(0, 2)], S.Centro + FVector(FMath::Cos(A), FMath::Sin(A), 0) * Lab::Cm(18), true);
			}
		}
		else if (S.Tipo == TEXT("treasure"))
			for (int32 k = 0; k < 3; ++k) CrearCofre(S.Centro + FVector(0, Lab::Cm(-16 + k * 16), 0), uint8(ELabCofre::Rico));
	}
	// cofres en callejones sin salida y algunos sueltos
	int32 N = 0;
	for (const FIntPoint& C : Mapa->CallejonesSinSalida()) if (R.FRand() < .5f && N < 40) { CrearCofre(Mapa->Centro(C.X, C.Y), uint8(ELabCofre::Comun)); ++N; }
	for (int32 k = 0; k < 6; ++k) CrearCofre(Mapa->PuntoLibre([&](const FVector& P) { return !Sala(P, {TEXT("exit"), TEXT("merchant")}); }), uint8(ELabCofre::Comun));
	// bestias comunes
	const TArray<FString> Tipos = {TEXT("perro"), TEXT("perro"), TEXT("perro"), TEXT("zombi"), TEXT("zombi"), TEXT("zombi"), TEXT("fantasma"), TEXT("fantasma"), TEXT("demonio"), TEXT("demonio")};
	for (int32 k = 0; k < CantidadBestias; ++k)
		CrearBestia(Tipos[R.RandRange(0, Tipos.Num() - 1)], Mapa->PuntoLibre([&](const FVector& P) { return LejosDe(P, 120) && !Sala(P, {TEXT("exit"), TEXT("merchant"), TEXT("boss"), TEXT("vault")}); }), false);
	// tu cazador: dormido (fuera del mapa) hasta que termina el farmeo
	const int32 Nivel = NivelCazador > 0 ? NivelCazador : R.RandRange(1, 10);
	const FLabCazador* Def = FLabDatos::Get().CazadorDeNivel(Nivel);
	FString Id;
	for (const auto& Par : FLabDatos::Get().Cazadores) if (Par.Value.Nivel == Nivel) Id = Par.Key.ToString();
	if (Def)
	{
		const FString Individuo = Id == TEXT("duendes") ? TEXT("duende") : Id == TEXT("jauria") ? TEXT("perro") : Id;
		const int32 Cuantos = Id == TEXT("jauria") ? 6 + R.RandRange(0, 4) : Def->Cantidad;
		for (int32 k = 0; k < Cuantos; ++k)
		{
			FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ALabEnemigo* H = GetWorld()->SpawnActor<ALabEnemigo>(ALabEnemigo::StaticClass(), FTransform(FVector(0, 0, -5000)), P);
			if (!H) continue;
			H->ConfigurarCazador(Individuo, *Def, Nivel, k == 0, J);
			H->bAlfa = Id == TEXT("jauria") && k == 0;
			H->Dormir();
			Cazadores.Add(H);
		}
		Anunciar(Def->Aviso, FString::Printf(TEXT("%s · Sale en %d:%02d"), *Def->Descripcion, int32(Farmeo) / 60, int32(Farmeo) % 60));
	}
	UE_LOG(LogLab, Log, TEXT("Partida armada: %d salas, cazador nivel %d (%s)"), Mapa->Salas.Num(), Nivel, *Id);
}

void ALabModoJuego::SoltarCazadores()
{
	ALabPersonaje* J = Jugador();
	if (!J) return;
	bCazadoresAfuera = true;
	const FVector Lugar = Mapa->PuntoLibre([&](const FVector& P) {
		const float D = FVector::Dist2D(P, J->GetActorLocation());
		const FLabSala* S = Mapa->SalaEn(P);
		return D > Lab::Cm(230) && D < Lab::Cm(520) && !(S && (S->Tipo == TEXT("exit") || S->Tipo == TEXT("boss") || S->Tipo == TEXT("vault")));
	});
	int32 k = 0;
	for (ALabEnemigo* H : Cazadores) if (H) { const int32 i = k++; H->Despertar(Lugar + FVector((i % 3 - 1) * 80.f, (i / 3 % 3 - 1) * 80.f, 0)); }
	Anunciar(TEXT("¡SALEN LOS CAZADORES!"), TEXT("Se terminó el farmeo. Tu cazador ya te está buscando."));
}

const ALabEnemigo* ALabModoJuego::CazadorConLlave() const
{
	for (const ALabEnemigo* H : Cazadores) if (H && H->bLlave) return H;
	return nullptr;
}

ALabEnemigo* ALabModoJuego::JefeCerca() const
{
	ALabPersonaje* J = Jugador();
	if (!J) return nullptr;
	for (TActorIterator<ALabEnemigo> It(GetWorld()); It; ++It)
		if (It->bJefe && It->Salud->Vivo() && FVector::Dist2D(It->GetActorLocation(), J->GetActorLocation()) < Lab::Cm(170)) return *It;
	return nullptr;
}

void ALabModoJuego::AlMorirEnemigo(ALabEnemigo* Muerto, AActor* Asesino)
{
	ALabPersonaje* J = Cast<ALabPersonaje>(Asesino);
	if (!J) return;
	J->Oro += Muerto->bCazador ? Muerto->Nivel * 12 + (Muerto->bLlave ? 10 + Muerto->Nivel * 6 : 0) : int32(Muerto->Oro);
	if (Muerto->bLlave) { J->bTieneLlave = true; J->Mensaje(TEXT("Tu cazador cayó. Tenés tu llave: buscá la salida en el centro.")); }
	if (Muerto->bJefe) J->Mensaje(FString::Printf(TEXT("¡Derrotaste a %s! Su cofre legendario ya se puede abrir."), *Muerto->Nombre));
	if (Muerto->bElite && FMath::FRand() < .5f) J->Pociones++;
}

void ALabModoJuego::UsarEscotilla(ALabPersonaje* P)
{
	if (EscotillaAbierta()) { Terminar(true, nullptr); return; }
	bAbriendo = true;
	P->Mensaje(TEXT("Abriendo la escotilla… no te alejes."));
}

void ALabModoJuego::Terminar(bool bEscapo, AActor* Asesino)
{
	if (bTerminada) return;
	bTerminada = true; bGano = bEscapo;
	ALabPersonaje* J = Jugador();
	if (J && bEscapo) { J->bEscapo = true; J->SetActorHiddenInGame(true); }
	const ALabEnemigo* E = Cast<ALabEnemigo>(Asesino);
	FinTitulo = bEscapo ? TEXT("ESCAPASTE") : TiempoRestante() <= 0 ? TEXT("SE CERRÓ") : TEXT("TE ALCANZÓ");
	FinTexto = bEscapo ? FString::Printf(TEXT("Salís a la niebla con %d de oro, pero el laberinto no deja sacar nada: la próxima empieza de cero."), J ? J->Oro : 0)
		: E ? FString::Printf(TEXT("Te mató %s."), *E->Nombre) : TEXT("Las paredes se juntan y el farol se apaga.");
	FTimerHandle H;
	GetWorldTimerManager().SetTimer(H, [this]() { UGameplayStatics::OpenLevel(this, TEXT("/Engine/Maps/Entry")); }, 8.f, false);
}

void ALabModoJuego::Tick(float Dt)
{
	Super::Tick(Dt);
	if (AnuncioT > 0) AnuncioT -= Dt;
	if (bTerminada || !bPoblada) return;
	Tiempo += Dt;
	if (!bCazadoresAfuera && Tiempo >= Farmeo) SoltarCazadores();
	if (TiempoRestante() <= 0) { Terminar(false, nullptr); return; }
	ALabPersonaje* J = Jugador();
	if (!J) return;
	if (EscotillaAbiertaT > 0) { EscotillaAbiertaT -= Dt; if (EscotillaAbiertaT <= 0) { Escotilla = 0; J->Mensaje(TEXT("La escotilla se cerró.")); } }
	else if (bAbriendo)
	{
		if (FVector::Dist2D(J->GetActorLocation(), LugarSalida()) > Lab::Cm(44)) { bAbriendo = false; J->Mensaje(TEXT("Te alejaste: la escotilla dejó de abrirse.")); }
		else if ((Escotilla += Dt) >= 10) { bAbriendo = false; EscotillaAbiertaT = 30; Anunciar(TEXT("¡LA ESCOTILLA ESTÁ ABIERTA!"), TEXT("Apretá E en la salida para escapar.")); }
	}
}
