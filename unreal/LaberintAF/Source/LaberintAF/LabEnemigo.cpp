#include "LabEnemigo.h"
#include "LabSalud.h"
#include "LabDatos.h"
#include "LabPersonaje.h"
#include "LabAviso.h"
#include "LabProyectil.h"
#include "LabCharco.h"
#include "LabLaberinto.h"
#include "LabModoJuego.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EngineUtils.h"
#include "AIController.h"

namespace
{
	FLinearColor ColorDe(const FString& T)
	{
		if (T == TEXT("perro")) return FLinearColor(.43f, .32f, .22f);
		if (T == TEXT("zombi")) return FLinearColor(.58f, .69f, .49f);
		if (T == TEXT("fantasma")) return FLinearColor(.85f, .9f, .88f);
		if (T == TEXT("demonio") || T == TEXT("diablo")) return FLinearColor(.69f, .19f, .12f);
		if (T == TEXT("gallina")) return FLinearColor(.95f, .95f, .9f);
		if (T == TEXT("jefe")) return FLinearColor(.51f, .52f, .55f);
		if (T == TEXT("carnicero")) return FLinearColor(.48f, .31f, .25f);
		if (T == TEXT("bruja") || T == TEXT("verase")) return FLinearColor(.24f, .18f, .3f);
		if (T == TEXT("roberto")) return FLinearColor(.48f, .37f, .22f);
		if (T == TEXT("duende") || T == TEXT("vibora")) return FLinearColor(.42f, .54f, .24f);
		if (T == TEXT("arana") || T == TEXT("cria")) return FLinearColor(.18f, .14f, .19f);
		if (T == TEXT("ninja") || T == TEXT("bob")) return FLinearColor(.16f, .16f, .2f);
		if (T == TEXT("gigante")) return FLinearColor(.66f, .52f, .37f);
		return FLinearColor(.5f, .5f, .5f);
	}
	FLinearColor ColorHab(const FLabHabilidad& H)
	{
		if (H.Veneno > 0 || H.Tiene(TEXT("Charco"))) return FLinearColor(.47f, .9f, .27f);
		if (H.Fuego > 0 || H.Tiene(TEXT("CharcoFuego"))) return FLinearColor(1, .35f, .08f);
		if (H.Tiene(TEXT("Mirada"))) return FLinearColor(.75f, .94f, .55f);
		if (H.Ciega > 0) return FLinearColor(1, .84f, .24f);
		return FLinearColor(1, .24f, .16f);
	}
}

ALabEnemigo::ALabEnemigo()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 720, 0);
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();
	Salud = CreateDefaultSubobject<ULabSalud>(TEXT("Salud"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cil(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Esf(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	Cuerpo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cuerpo"));
	Cuerpo->SetupAttachment(RootComponent);
	Cuerpo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Cabeza = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cabeza"));
	Cabeza->SetupAttachment(RootComponent);
	Cabeza->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Cil.Succeeded()) Cuerpo->SetStaticMesh(Cil.Object);
	if (Esf.Succeeded()) Cabeza->SetStaticMesh(Esf.Object);
}

void ALabEnemigo::BeginPlay()
{
	Super::BeginPlay();
	Salud->AlMorir.AddDynamic(this, &ALabEnemigo::AlMorir);
	Casa = GetActorLocation();
}

void ALabEnemigo::Aspecto(FLinearColor Color, float Escala)
{
	GetCapsuleComponent()->SetCapsuleSize(Radio, 90.f * FMath::Max(.5f, Escala));
	Cuerpo->SetRelativeScale3D(FVector(Radio / 50.f * 1.2f, Radio / 50.f * 1.4f, 1.2f * Escala));
	Cabeza->SetRelativeLocation(FVector(Radio * .3f, 0, 80 * Escala));
	Cabeza->SetRelativeScale3D(FVector(.35f * Escala));
	if (UMaterialInterface* M = Cuerpo->GetMaterial(0))
	{
		MatCuerpo = UMaterialInstanceDynamic::Create(M, this);
		MatCuerpo->SetVectorParameterValue(TEXT("Color"), Color);
		Cuerpo->SetMaterial(0, MatCuerpo);
		Cabeza->SetMaterial(0, MatCuerpo);
	}
}

void ALabEnemigo::ConfigurarBestia(const FString& Id, const FLabBestia& B, bool bEsElite)
{
	Tipo = Id; Nombre = B.Nombre; bJefe = B.Jefe; bElite = bEsElite;
	Radio = Lab::Cm(B.Radio) * .8f;
	Danio = B.Danio * (bElite ? 1.5f : 1); Velocidad = Lab::CmS(B.Velocidad) * (bElite ? 1.1f : 1); Alerta = Lab::Cm(B.Alerta); Oro = B.Oro * (bElite ? 4 : 1);
	Enfriamiento = bJefe ? 1.3f : 1;
	Salud->Iniciar(B.Vida * (bElite ? 3 : 1));
	Salud->bPuedePerderMiembros = Id == TEXT("perro") || Id == TEXT("zombi");
	if (bElite) Nombre += TEXT(" de élite");
	Habilidades = FLabDatos::Get().HabilidadesDe(Id);
	HabCd.Init(0, Habilidades.Num());
	Aspecto(ColorDe(Id), bJefe ? 2.f : bElite ? 1.35f : Id == TEXT("cria") || Id == TEXT("gallina") ? .5f : 1.f);
	GetCharacterMovement()->MaxWalkSpeed = Velocidad;
}

void ALabEnemigo::ConfigurarCazador(const FString& Id, const FLabCazador& C, int32 SuNivel, bool bConLlave, ALabPersonaje* SuPresa)
{
	Tipo = Id; Nombre = C.Nombre; bCazador = true; Nivel = SuNivel; bLlave = bConLlave; Presa = SuPresa;
	Radio = Lab::Cm(C.Radio) * .8f; Danio = C.Danio; Velocidad = Lab::CmS(C.Velocidad); Enfriamiento = C.Enfriamiento;
	Alerta = Lab::Cm(Id == TEXT("roberto") ? 230 : 170);
	Salud->Iniciar(C.Vida);
	Salud->bPuedePerderMiembros = Id == TEXT("perro") || Id == TEXT("roberto") || Id == TEXT("ninja") || Id == TEXT("duende");
	Habilidades = FLabDatos::Get().HabilidadesDe(Id == TEXT("perro") ? FString(TEXT("jauria")) : Id);
	HabCd.Init(0, Habilidades.Num());
	Aspecto(ColorDe(Id), C.Escala);
	GetCharacterMovement()->MaxWalkSpeed = Velocidad;
}

void ALabEnemigo::Dormir()
{
	bDormido = true;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();   // si no, cae para siempre debajo del mapa
}

void ALabEnemigo::Despertar(const FVector& Lugar)
{
	bDormido = false;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	SetActorLocation(Lugar + FVector(0, 0, 100), false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	// unos segundos de gracia en las habilidades al salir (como en la web)
	for (float& Cd : HabCd) Cd = FMath::Max(Cd, FMath::FRandRange(5.f, 9.f));
}

void ALabEnemigo::AlRecibirGolpeDe(AActor* Quien)
{
	if (ALabPersonaje* P = Cast<ALabPersonaje>(Quien)) { Presa = P; Atencion = 1; bDescubrio = true; VistoEn = P->GetActorLocation(); PerdidoT = 0; }
}

bool ALabEnemigo::Ve(const ALabPersonaje* P) const
{
	if (!P || !P->Salud->Vivo() || P->bEscapo) return false;
	const float D = FVector::Dist2D(P->GetActorLocation(), GetActorLocation());
	if (D < Lab::Cm(15)) return true;
	const float Vis = Tipo == TEXT("roberto") ? P->Visibilidad() / (P->bFarolPrendido ? 1.f : .6f) : P->Visibilidad();
	if (D > Alerta * Vis) return false;
	const ALabLaberinto* L = ALabModoJuego::Laberinto(this);
	return Tipo == TEXT("fantasma") || !L || L->Visible(GetActorLocation(), P->GetActorLocation());
}

void ALabEnemigo::Sigilo(ALabPersonaje* P, float Dt)
{
	if (Ve(P))
	{
		const float D = FVector::Dist2D(P->GetActorLocation(), GetActorLocation());
		const float K = FMath::Clamp(1 - D / (Alerta * FMath::Max(.01f, P->Visibilidad())), 0.f, 1.f);
		Atencion = FMath::Min(1.f, Atencion + (.55f + 2.4f * K) * Dt);
		VistoEn = P->GetActorLocation(); PerdidoT = 0;
		if (Atencion >= 1 && !bDescubrio) { bDescubrio = true; Presa = P; }
	}
	else
	{
		PerdidoT += Dt;
		if (Atencion < 1) Atencion = FMath::Max(0.f, Atencion - .18f * Dt);
		else if (PerdidoT > 5) { Atencion = .6f; bDescubrio = false; }   // a los 5 s sin verte vuelve a buscar
	}
}

void ALabEnemigo::IrHacia(const FVector& Punto, float Vel, float Dt)
{
	GetCharacterMovement()->MaxWalkSpeed = Vel * Salud->MultVelocidad();
	ALabLaberinto* L = ALabModoJuego::Laberinto(this);
	FVector Hacia = Punto;
	if (L && Tipo != TEXT("fantasma") && !L->Visible(GetActorLocation(), Punto))
	{
		RutaT -= Dt;
		if (RutaT <= 0 || FVector::Dist2D(Meta, Punto) > Lab::Tile) { Ruta = L->Camino(GetActorLocation(), Punto); Meta = Punto; RutaT = .5f; }
		while (Ruta.Num() && FVector::Dist2D(Ruta[0], GetActorLocation()) < 60) Ruta.RemoveAt(0);
		if (Ruta.Num()) Hacia = Ruta[0];
	}
	const FVector Dir = (Hacia - GetActorLocation()).GetSafeNormal2D();
	if (!Dir.IsNearlyZero()) AddMovementInput(Dir, 1);
}

void ALabEnemigo::Pegar(ALabPersonaje* T, float Dt, float D)
{
	const float Rango = Radio + 45 + Lab::Cm(7);
	if (Ventana > 0)
	{
		Ventana -= Dt;
		SetActorRotation(FRotator(0, (T->GetActorLocation() - GetActorLocation()).Rotation().Yaw, 0));
		if (Ventana <= 0)
		{
			Estocada = .16f;
			if (FVector::Dist2D(T->GetActorLocation(), GetActorLocation()) < Rango + Lab::Cm(4))
			{
				FLabEfectos E;
				if (Tipo == TEXT("perro") && FMath::FRand() < .3f) E.Sangra = 3;
				if (Tipo == TEXT("vibora")) E.Veneno = 5;
				const float Dn = DanioVentana > 0 ? DanioVentana : Danio * (BuffT > 0 ? 1.3f : 1.f) * Salud->MultDanio();
				if (T->Salud->RecibirGolpe(Dn, this, Tipo == TEXT("ninja") || Tipo == TEXT("carnicero") ? ELabGolpe::Filo : ELabGolpe::Contundente, E) && Tipo == TEXT("bob"))
					Salud->Curar(Danio, false);
			}
			DanioVentana = 0;
		}
		return;
	}
	if (D < Rango && GolpeCd <= 0) { GolpeCd = Enfriamiento; Ventana = Tipo == TEXT("duende") || Tipo == TEXT("perro") ? .22f : bJefe ? .45f : .32f; }
	else if (D >= Rango) IrHacia(T->GetActorLocation(), Velocidad * (BuffT > 0 ? 1.5f : 1.f), Dt);
}

void ALabEnemigo::Aplicar(ALabPersonaje* P, const FLabHabilidad& H, float Dn)
{
	if (P->Salud->RecibirGolpe(Dn, this, H.Sangra > 0 ? ELabGolpe::Filo : ELabGolpe::Contundente, FLabEfectos::De(H)) && !H.Mensaje.IsEmpty() && !H.Tiene(TEXT("Mirada")))
		P->Mensaje(H.Mensaje);
}

bool ALabEnemigo::Habilidad(int32 i, ALabPersonaje* T, float D, bool bVe)
{
	const FLabHabilidad& H = *Habilidades[i];
	if (HabCd[i] > 0 || D < Lab::Cm(H.DistMin) || D > Lab::Cm(H.DistMax)) return false;
	if (H.RequiereVer && !bVe) return false;
	if (H.Tiene(TEXT("Furia")) && !bFuria) return false;
	if (H.Tiene(TEXT("SoloAlfa")) && !bAlfa) return false;
	UWorld* W = GetWorld();
	ALabLaberinto* L = ALabModoJuego::Laberinto(this);
	const float Yaw = (T->GetActorLocation() - GetActorLocation()).Rotation().Yaw;
	const FLinearColor Col = ColorHab(H);
	TWeakObjectPtr<ALabEnemigo> Yo(this);
	TWeakObjectPtr<ALabPersonaje> Obj(T);
	const FLabHabilidad* HP = &H;
	HabCd[i] = H.Enfriamiento * (bFuria ? .75f : 1.f);
	if (H.Aviso > 0 && H.Forma != TEXT("Salto")) Bloqueo = H.Aviso;
	SetActorRotation(FRotator(0, Yaw, 0));

	if (H.Forma == TEXT("Proyectil"))
	{
		auto Tirar = [Yo, Obj, HP]()
		{
			if (!Yo.IsValid()) return;
			const float Base = Obj.IsValid() ? (Obj->GetActorLocation() - Yo->GetActorLocation()).Rotation().Yaw : Yo->GetActorRotation().Yaw;
			for (int32 k = 0; k < HP->Cantidad; ++k)
			{
				const float Off = HP->Tiene(TEXT("Radial")) ? 360.f * k / HP->Cantidad : (HP->Cantidad > 1 ? FMath::RadiansToDegrees(HP->Arco) * (float(k) / (HP->Cantidad - 1) - .5f) : 0.f);
				ALabProyectil::Disparar(Yo.Get(), Base + Off, Lab::CmS(HP->Velocidad), HP->Duracion, HP->Danio, FLabEfectos::De(*HP), HP->Flags, ColorHab(*HP), Obj.Get());
			}
		};
		if (H.Tiene(TEXT("Rastrea")))
		{
			ALabAviso* A = ALabAviso::Crear(W, ELabFormaAviso::Linea, GetActorLocation(), Yaw, 0, Lab::Cm(H.DistMax), Lab::Cm(6), 0, H.Aviso, FLinearColor(.9f, .9f, .95f), this,
				[Yo, HP](ALabAviso* Av) { if (Yo.IsValid()) ALabProyectil::Disparar(Yo.Get(), Av->GetActorRotation().Yaw, Lab::CmS(HP->Velocidad), HP->Duracion, HP->Danio, FLabEfectos::De(*HP), HP->Flags, FLinearColor(.8f, .8f, .85f)); });
			if (A) { A->Seguir = this; A->Rastrear = T; A->FijarAntes = .2f; }
		}
		else if (H.Aviso > 0) ALabAviso::Despues(W, H.Aviso, this, [Tirar](ALabAviso*) { Tirar(); });
		else Tirar();
		return H.Aviso > 0;
	}
	if (H.Forma == TEXT("Linea"))
	{
		ALabAviso* A = ALabAviso::Crear(W, ELabFormaAviso::Linea, GetActorLocation(), Yaw, 0, Lab::Cm(H.Largo), Lab::Cm(H.Ancho), 0, H.Aviso, Col, this,
			[Yo, HP, L](ALabAviso* Av)
			{
				if (!Yo.IsValid()) return;
				for (ALabPersonaje* P : Av->Alcanzados()) if (!L || L->Visible(Av->GetActorLocation(), P->GetActorLocation())) Yo->Aplicar(P, *HP, HP->Danio);
				if (HP->Tiene(TEXT("CharcoFuego")))
					for (float s = Lab::Cm(12); s < Av->Largo; s += Lab::Cm(12))
					{
						const FVector Pt = Av->GetActorLocation() + Av->GetActorForwardVector() * s;
						if (L && L->EsSolido(Pt)) break;
						ALabCharco::Crear(Yo->GetWorld(), ELabCharco::Fuego, Pt, Lab::Cm(8), 4, Yo.Get());
					}
			});
		if (A && H.Tiene(TEXT("Propio"))) A->Seguir = this;
		if (A && H.Tiene(TEXT("Rastrea"))) { A->Rastrear = T; A->FijarAntes = .28f; }
		return true;
	}
	if (H.Forma == TEXT("Circulo"))
	{
		for (int32 k = 0; k < H.Cantidad; ++k)
		{
			const bool Propio = H.Tiene(TEXT("Propio"));
			FVector Lugar = Propio ? GetActorLocation() : T->GetActorLocation();
			if (k > 0 && L) Lugar = L->PuntoLibreCerca(T->GetActorLocation(), Lab::Cm(45));
			ALabAviso* A = ALabAviso::Crear(W, ELabFormaAviso::Circulo, Lugar, 0, Lab::Cm(H.Radio), 0, 0, 0, FMath::Max(.05f, H.Aviso + k * .15f), Col, this,
				[Yo, HP](ALabAviso* Av)
				{
					if (!Yo.IsValid()) return;
					float Robado = 0;
					for (ALabPersonaje* P : Av->Alcanzados()) { Yo->Aplicar(P, *HP, HP->Danio); Robado += HP->Danio; }
					if (HP->Tiene(TEXT("RobaVida"))) Yo->Salud->Curar(Robado, false);
					if (HP->Tiene(TEXT("Charco"))) ALabCharco::Crear(Yo->GetWorld(), ELabCharco::Veneno, Av->GetActorLocation(), Av->Radio, 5, Yo.Get());
					if (HP->Tiene(TEXT("CharcoFuego"))) ALabCharco::Crear(Yo->GetWorld(), ELabCharco::Fuego, Av->GetActorLocation(), Av->Radio * .7f, 2.5f, Yo.Get());
				});
			if (A && Propio) A->Seguir = this;
		}
		if (!H.Mensaje.IsEmpty() && H.Cantidad > 1) T->Mensaje(H.Mensaje);
		return H.Aviso > 0;
	}
	if (H.Forma == TEXT("Cono"))
	{
		ALabAviso* A = ALabAviso::Crear(W, ELabFormaAviso::Cono, GetActorLocation(), Yaw, Lab::Cm(H.Radio), 0, 0, H.Arco, H.Aviso, Col, this,
			[Yo, HP, L](ALabAviso* Av)
			{
				if (!Yo.IsValid()) return;
				for (ALabPersonaje* P : Av->Alcanzados())
				{
					if (!HP->Tiene(TEXT("Mirada"))) { Yo->Aplicar(P, *HP, HP->Danio); continue; }
					// si la estás mirando cuando termina, quedás de piedra; de espaldas te salvás
					const FVector AElla = (Yo->GetActorLocation() - P->GetActorLocation()).GetSafeNormal2D();
					const bool Mira = FMath::Acos(FVector::DotProduct(P->GetActorForwardVector(), AElla)) < 1.25f;
					if (Mira && (!L || L->Visible(Yo->GetActorLocation(), P->GetActorLocation()))) { P->Salud->Aturdido = FMath::Max(P->Salud->Aturdido, HP->Aturde); P->Mensaje(TEXT("La miraste a los ojos: sos piedra.")); }
					else P->Mensaje(TEXT("Le diste la espalda: te salvaste."));
				}
			});
		if (A) A->Seguir = this;
		if (!H.Mensaje.IsEmpty()) T->Mensaje(H.Mensaje);
		return true;
	}
	if (H.Forma == TEXT("Embestida"))
	{
		ALabAviso* A = ALabAviso::Crear(W, ELabFormaAviso::Linea, GetActorLocation(), Yaw, 0, Lab::Cm(H.Largo), Lab::Cm(H.Ancho), 0, H.Aviso, Col, this,
			[Yo, HP](ALabAviso* Av) { if (!Yo.IsValid()) return; Yo->DashVel = Av->GetActorForwardVector() * Lab::CmS(HP->Velocidad) * 1.6f; Yo->DashT = HP->Duracion; Yo->DashHab = HP; Yo->DashGolpeados.Reset(); });
		if (A) A->Seguir = this;
		return true;
	}
	if (H.Forma == TEXT("Salto"))
	{
		const FVector Destino = T->GetActorLocation();
		ALabAviso::Crear(W, ELabFormaAviso::Circulo, Destino, 0, Lab::Cm(H.Radio), 0, 0, 0, H.Aviso, Col, this, nullptr);
		SaltoA = GetActorLocation(); SaltoB = Destino; SaltoT = 0; SaltoDur = FMath::Max(.2f, H.Aviso); SaltoHab = &H;
		return true;
	}
	if (H.Forma == TEXT("Teletransporte"))
	{
		FVector Destino = GetActorLocation();
		if (H.Tiene(TEXT("Detras"))) Destino = T->GetActorLocation() - T->GetActorForwardVector() * Lab::Cm(14);
		else if (H.Tiene(TEXT("Cerca")) && L) { bool Ok = false; Destino = L->PuntoLibre([&](const FVector& P) { const float d = FVector::Dist2D(P, T->GetActorLocation()); return d > Lab::Cm(60) && d < Lab::Cm(110) && L->Visible(P, T->GetActorLocation()); }, &Ok); if (!Ok) return false; }
		else if (H.Tiene(TEXT("Lejos")) && L) Destino = L->PuntoLibreCerca(GetActorLocation(), Lab::Cm(70));
		if (L && L->EsSolido(Destino)) return false;
		if (H.Tiene(TEXT("Niebla")))
		{
			// se hace niebla: no se lo puede tocar mientras viaja y aparece atrás tuyo
			Intangible = H.Duracion; Salud->Intangible = H.Duracion; SetActorHiddenInGame(true);
			DashVel = (Destino - GetActorLocation()).GetSafeNormal2D() * FVector::Dist2D(Destino, GetActorLocation()) / FMath::Max(.1f, H.Duracion);
			DashT = H.Duracion; DashHab = nullptr;
			return true;
		}
		SetActorLocation(FVector(Destino.X, Destino.Y, GetActorLocation().Z), false, nullptr, ETeleportType::TeleportPhysics);
		if (H.Danio > 0) { GolpeCd = 0; Ventana = FMath::Max(.1f, H.Aviso); DanioVentana = H.Danio; Bloqueo = 0; }
		if (!H.Mensaje.IsEmpty()) T->Mensaje(H.Mensaje);
		return true;
	}
	if (H.Forma == TEXT("Invocar"))
	{
		ALabModoJuego* M = ALabModoJuego::Get(this);
		if (!M) return false;
		for (int32 k = 0; k < H.Cantidad; ++k)
		{
			const FVector P = L ? L->PuntoLibreCerca(GetActorLocation(), Lab::Cm(40)) : GetActorLocation();
			if (ALabEnemigo* E = M->CrearBestia(H.Invoca, P, false)) { E->Presa = T; E->Atencion = 1; E->bDescubrio = true; E->Oro = 0; }
		}
		if (!H.Mensaje.IsEmpty()) T->Mensaje(H.Mensaje);
		return H.Aviso > 0;
	}
	if (H.Forma == TEXT("Grito"))
	{
		for (TActorIterator<ALabEnemigo> It(W); It; ++It)
			if (It->Tipo == Tipo && FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < Lab::Cm(H.Radio)) It->BuffT = H.Duracion;
		if (!H.Mensaje.IsEmpty()) T->Mensaje(H.Mensaje);
		return true;
	}
	if (H.Forma == TEXT("Trampa"))
	{
		int32 N = 0;
		for (TActorIterator<ALabCharco> It(W); It; ++It) if (It->Tipo == ELabCharco::Cepo && It->bArmado && It->Duenio == this) ++N;
		if (N >= H.Cantidad) return false;
		ALabCharco::Crear(W, ELabCharco::Cepo, GetActorLocation(), Lab::Cm(H.Radio), 600, this);
		return false;
	}
	if (H.Forma == TEXT("Torbellino"))
	{
		ALabAviso* A = ALabAviso::Crear(W, ELabFormaAviso::Circulo, GetActorLocation(), 0, Lab::Cm(H.Radio), 0, 0, 0, H.Aviso, Col, this,
			[Yo, HP](ALabAviso*) { if (Yo.IsValid()) { Yo->GiroT = HP->Duracion * (Yo->bFuria ? 1.4f : 1.f); Yo->GiroHab = HP; Yo->GiroGolpe = 0; } });
		if (A) A->Seguir = this;
		return true;
	}
	return false;
}

void ALabEnemigo::TickEspeciales(float Dt)
{
	if (DashT > 0)
	{
		DashT -= Dt;
		const FVector Antes = GetActorLocation();
		FHitResult Choque;
		AddActorWorldOffset(DashVel * Dt, true, &Choque);
		if (DashHab)
		{
			for (TActorIterator<ALabPersonaje> It(GetWorld()); It; ++It)
				if (!DashGolpeados.Contains(*It) && It->Salud->Vivo() && It->Salud->Invulnerable <= 0 && FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < Radio + 60)
				{ DashGolpeados.Add(*It); Aplicar(*It, *DashHab, DashHab->Danio); }
			if (Choque.bBlockingHit && FVector::Dist2D(Antes, GetActorLocation()) < DashVel.Size() * Dt * .35f)
			{
				DashT = 0;
				if (DashHab->Tiene(TEXT("ChoquePared"))) { Salud->Aturdido = 1.6f; if (ALabPersonaje* P = Presa.Get()) P->Mensaje(Nombre + TEXT(" se dio contra la pared: ¡pegale!")); }
			}
		}
		if (DashT <= 0 && Intangible > 0) { Intangible = 0; Salud->Intangible = 0; SetActorHiddenInGame(false); GolpeCd = 0; }
		return;
	}
	if (SaltoT >= 0)
	{
		SaltoT += Dt / SaltoDur;
		const float F = FMath::Min(1.f, SaltoT);
		SetActorLocation(FMath::Lerp(SaltoA, SaltoB, F) + FVector(0, 0, FMath::Sin(F * PI) * 250), false, nullptr, ETeleportType::TeleportPhysics);
		if (SaltoT >= 1)
		{
			SaltoT = -1;
			for (TActorIterator<ALabPersonaje> It(GetWorld()); It; ++It)
				if (It->Salud->Vivo() && It->Salud->Invulnerable <= 0 && FVector::Dist2D(It->GetActorLocation(), SaltoB) < Lab::Cm(SaltoHab->Radio) + 30) Aplicar(*It, *SaltoHab, SaltoHab->Danio);
		}
		return;
	}
	if (GiroT > 0)
	{
		GiroT -= Dt; GiroGolpe -= Dt;
		AddActorWorldRotation(FRotator(0, 800 * Dt, 0));
		if (ALabPersonaje* P = Presa.Get()) IrHacia(P->GetActorLocation(), Velocidad * .8f, Dt);
		if (GiroGolpe <= 0 && GiroHab)
		{
			GiroGolpe = .3f;
			for (TActorIterator<ALabPersonaje> It(GetWorld()); It; ++It)
				if (It->Salud->Vivo() && FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < Lab::Cm(GiroHab->Radio) + 50) Aplicar(*It, *GiroHab, GiroHab->Danio);
		}
	}
}

void ALabEnemigo::Tick(float Dt)
{
	Super::Tick(Dt);
	if (bDormido || !Salud->Vivo()) return;
	for (float& Cd : HabCd) Cd -= Dt;
	GolpeCd -= Dt; BuffT -= Dt; Estocada -= Dt;
	// la estocada al pegar (provisorio: en la versión final es la animación del esqueleto)
	Cuerpo->SetRelativeLocation(FVector(Estocada > 0 ? FMath::Sin(Estocada / .16f * PI) * 30 : (Ventana > 0 ? -18.f : 0.f), 0, 0));
	if (!Salud->PuedeMoverse()) return;
	if (DashT > 0 || SaltoT >= 0 || GiroT > 0) { TickEspeciales(Dt); return; }
	if (Bloqueo > 0) { Bloqueo -= Dt; return; }
	if (bJefe && !bFuria && Salud->Vida < Salud->VidaMax * .5f)
	{
		bFuria = true; Velocidad *= 1.25f; Bloqueo = .9f;
		if (ALabModoJuego* M = ALabModoJuego::Get(this)) M->Anunciar(FString::Printf(TEXT("¡%s SE ENFURECE!"), *Nombre.ToUpper()), TEXT("Ahora pega más fuerte y más seguido."));
		return;
	}
	ALabModoJuego* M = ALabModoJuego::Get(this);
	ALabPersonaje* J = M ? M->Jugador() : nullptr;
	if (!J || !J->Salud->Vivo() || J->bEscapo) { if (bJefe || bElite) return; IrHacia(Patrulla.IsZero() ? Casa : Patrulla, Velocidad * .45f, Dt); return; }
	if (Tipo == TEXT("gallina"))
	{
		const float Dg = FVector::Dist2D(J->GetActorLocation(), GetActorLocation());
		if (Dg < Lab::Cm(55)) AddMovementInput((GetActorLocation() - J->GetActorLocation()).GetSafeNormal2D(), 1);
		return;
	}
	const float D = FVector::Dist2D(J->GetActorLocation(), GetActorLocation());
	if (!bCazador && !bDescubrio && D > Alerta * 2.2f)
	{
		// patrulla cerca de su casa (jefes y élites no se alejan)
		if (bJefe || bElite) return;
		PatrullaT -= Dt;
		ALabLaberinto* L = ALabModoJuego::Laberinto(this);
		if ((PatrullaT <= 0 || FVector::Dist2D(Patrulla, GetActorLocation()) < 60) && L) { Patrulla = L->PuntoLibreCerca(Casa, Lab::Cm(150)); PatrullaT = 5 + FMath::FRand() * 5; }
		IrHacia(Patrulla, Velocidad * .45f, Dt);
		return;
	}
	Sigilo(J, Dt);
	if (bCazador && Atencion < 1)
	{
		// el cazador sabe más o menos dónde estás y va a buscarte a esa zona
		SupuestoT -= Dt;
		if (SupuestoT <= 0 || FVector::Dist2D(Supuesto, GetActorLocation()) < Lab::Cm(14))
		{
			ALabLaberinto* L = ALabModoJuego::Laberinto(this);
			Supuesto = L ? L->PuntoLibreCerca(J->GetActorLocation(), Lab::Cm(24 + 120 * .8f)) : J->GetActorLocation();
			SupuestoT = 4 + FMath::FRand() * 3;
		}
		IrHacia(Supuesto, Velocidad * .8f, Dt);
		return;
	}
	if (!bCazador && !bDescubrio)
	{
		if (Atencion > .3f) IrHacia(VistoEn, Velocidad * .7f, Dt);   // va a mirar
		return;
	}
	if (!bJefe && !bElite && !bCazador && FVector::Dist2D(Casa, GetActorLocation()) > Lab::Cm(320)) { bDescubrio = false; Atencion = 0; return; }
	if ((bJefe || bElite) && FVector::Dist2D(Casa, J->GetActorLocation()) > Lab::Cm(bJefe ? 190 : 150)) { bDescubrio = false; Atencion = 0; IrHacia(Casa, Velocidad * .5f, Dt); if (bJefe) Salud->Curar(20 * Dt, false); return; }
	const bool bVe = Ve(J);
	if (Ventana <= 0) for (int32 i = 0; i < Habilidades.Num(); ++i) if (Habilidad(i, J, D, bVe)) return;
	Pegar(J, Dt, D);
}

void ALabEnemigo::AlMorir(AActor* Victima, AActor* Asesino)
{
	if (ALabModoJuego* M = ALabModoJuego::Get(this)) M->AlMorirEnemigo(this, Asesino);
	GetCharacterMovement()->DisableMovement();
	SetActorEnableCollision(false);
	SetLifeSpan(6);
	Cuerpo->SetRelativeRotation(FRotator(90, 0, 0));   // cae (en la versión final: ragdoll y desmembramiento)
	Cabeza->SetRelativeLocation(FVector(Radio, 0, 20));
}
