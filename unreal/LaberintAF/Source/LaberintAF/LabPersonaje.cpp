#include "LabPersonaje.h"
#include "LabSalud.h"
#include "LabDatos.h"
#include "LabEnemigo.h"
#include "LabCofre.h"
#include "LabModoJuego.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "EngineUtils.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

ALabPersonaje::ALabPersonaje()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(Lab::Cm(4) * .8f, 90.f);
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = Lab::CmS(58);
	GetCharacterMovement()->BrakingDecelerationWalking = 4000;

	Brazo = CreateDefaultSubobject<USpringArmComponent>(TEXT("Brazo"));
	Brazo->SetupAttachment(RootComponent);
	Brazo->SetUsingAbsoluteRotation(true);
	Brazo->SetWorldRotation(FRotator(-58, 0, 0));
	Brazo->TargetArmLength = 1700;
	Brazo->bDoCollisionTest = false;
	Brazo->bEnableCameraLag = true;
	Brazo->CameraLagSpeed = 8;
	Camara = CreateDefaultSubobject<UCameraComponent>(TEXT("Camara"));
	Camara->SetupAttachment(Brazo);
	Camara->SetFieldOfView(55);

	// farol: la única luz fuerte del juego, con sombras (con Lumen rebota en muros y piso)
	Farol = CreateDefaultSubobject<UPointLightComponent>(TEXT("Farol"));
	Farol->SetupAttachment(RootComponent);
	Farol->SetRelativeLocation(FVector(-20, -25, 70));
	Farol->SetLightColor(FLinearColor(1.f, .72f, .42f));
	Farol->SetIntensity(9000);
	Farol->SetAttenuationRadius(Lab::Cm(64));
	Farol->SetSourceRadius(4);
	Farol->SetCastShadows(true);

	Salud = CreateDefaultSubobject<ULabSalud>(TEXT("Salud"));
	Salud->bEsJugador = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cil(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Esf(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cubo(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Cuerpo = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cuerpo"));
	Cuerpo->SetupAttachment(RootComponent);
	Cuerpo->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Cuerpo->SetRelativeScale3D(FVector(.5f, .6f, 1.3f));
	Cabeza = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cabeza"));
	Cabeza->SetupAttachment(RootComponent);
	Cabeza->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Cabeza->SetRelativeLocation(FVector(5, 0, 85));
	Cabeza->SetRelativeScale3D(FVector(.32f));
	Arma = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arma"));
	Arma->SetupAttachment(RootComponent);
	Arma->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Arma->SetRelativeLocation(FVector(45, 35, 20));
	Arma->SetRelativeScale3D(FVector(.5f, .05f, .05f));
	if (Cil.Succeeded()) Cuerpo->SetStaticMesh(Cil.Object);
	if (Esf.Succeeded()) Cabeza->SetStaticMesh(Esf.Object);
	if (Cubo.Succeeded()) Arma->SetStaticMesh(Cubo.Object);
}

void ALabPersonaje::BeginPlay()
{
	Super::BeginPlay();
	Salud->Iniciar(100);
	Salud->AlMorir.AddDynamic(this, &ALabPersonaje::AlMorir);
	if (UMaterialInterface* M = Cuerpo->GetMaterial(0))
	{
		UMaterialInstanceDynamic* D = UMaterialInstanceDynamic::Create(M, this);
		D->SetVectorParameterValue(TEXT("Color"), FLinearColor(.42f, .33f, .19f));
		Cuerpo->SetMaterial(0, D);
		UMaterialInstanceDynamic* P = UMaterialInstanceDynamic::Create(M, this);
		P->SetVectorParameterValue(TEXT("Color"), FLinearColor(.82f, .6f, .45f));
		Cabeza->SetMaterial(0, P);
	}
}

void ALabPersonaje::SetupPlayerInputComponent(UInputComponent* Entrada)
{
	Super::SetupPlayerInputComponent(Entrada);
	auto Accion = [this](const TCHAR* Nombre, EInputActionValueType Tipo) { UInputAction* A = NewObject<UInputAction>(this, MakeUniqueObjectName(this, UInputAction::StaticClass(), Nombre)); A->ValueType = Tipo; return A; };
	AMover = Accion(TEXT("Mover"), EInputActionValueType::Axis2D);
	ACorrer = Accion(TEXT("Correr"), EInputActionValueType::Boolean);
	ARodar = Accion(TEXT("Rodar"), EInputActionValueType::Boolean);
	AAgachar = Accion(TEXT("Agachar"), EInputActionValueType::Boolean);
	AAtacar = Accion(TEXT("Atacar"), EInputActionValueType::Boolean);
	AUsar = Accion(TEXT("Usar"), EInputActionValueType::Boolean);
	APocion = Accion(TEXT("Pocion"), EInputActionValueType::Boolean);
	AFarol = Accion(TEXT("Farol"), EInputActionValueType::Boolean);

	Contexto = NewObject<UInputMappingContext>(this, MakeUniqueObjectName(this, UInputMappingContext::StaticClass(), TEXT("Teclas")));
	// W/S van por X (arriba de la pantalla) y A/D por Y, como en la versión web
	auto Eje = [this](FKey Tecla, bool bY, bool bNeg)
	{
		FEnhancedActionKeyMapping& M = Contexto->MapKey(AMover, Tecla);
		if (bY) { UInputModifierSwizzleAxis* S = NewObject<UInputModifierSwizzleAxis>(this); S->Order = EInputAxisSwizzle::YXZ; M.Modifiers.Add(S); }
		if (bNeg) M.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	};
	Eje(EKeys::W, false, false); Eje(EKeys::S, false, true); Eje(EKeys::D, true, false); Eje(EKeys::A, true, true);
	Eje(EKeys::Up, false, false); Eje(EKeys::Down, false, true); Eje(EKeys::Right, true, false); Eje(EKeys::Left, true, true);
	Contexto->MapKey(ACorrer, EKeys::LeftShift);
	Contexto->MapKey(ARodar, EKeys::SpaceBar);
	Contexto->MapKey(AAgachar, EKeys::C);
	Contexto->MapKey(AAtacar, EKeys::LeftMouseButton);
	Contexto->MapKey(AAtacar, EKeys::J);
	Contexto->MapKey(AUsar, EKeys::E);
	Contexto->MapKey(APocion, EKeys::Q);
	Contexto->MapKey(AFarol, EKeys::F);
	// mando: palanca izquierda mueve, A rueda, X pega, B usa
	{ FEnhancedActionKeyMapping& G = Contexto->MapKey(AMover, EKeys::Gamepad_Left2D); G.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(this)); }   // la palanca arriba = arriba de la pantalla (+X)
	Contexto->MapKey(ARodar, EKeys::Gamepad_FaceButton_Bottom);
	Contexto->MapKey(AAtacar, EKeys::Gamepad_FaceButton_Left);
	Contexto->MapKey(AUsar, EKeys::Gamepad_FaceButton_Right);
	Contexto->MapKey(ACorrer, EKeys::Gamepad_LeftTrigger);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			Sub->AddMappingContext(Contexto, 0);

	if (UEnhancedInputComponent* E = Cast<UEnhancedInputComponent>(Entrada))
	{
		E->BindAction(AMover, ETriggerEvent::Triggered, this, &ALabPersonaje::Mover);
		E->BindAction(AMover, ETriggerEvent::Completed, this, &ALabPersonaje::MoverFin);
		E->BindAction(ACorrer, ETriggerEvent::Started, this, &ALabPersonaje::CorrerSi);
		E->BindAction(ACorrer, ETriggerEvent::Completed, this, &ALabPersonaje::CorrerNo);
		E->BindAction(ARodar, ETriggerEvent::Started, this, &ALabPersonaje::Rodar);
		E->BindAction(AAgachar, ETriggerEvent::Started, this, &ALabPersonaje::Agachar);
		E->BindAction(AAtacar, ETriggerEvent::Started, this, &ALabPersonaje::AtaqueSi);
		E->BindAction(AAtacar, ETriggerEvent::Completed, this, &ALabPersonaje::AtaqueNo);
		E->BindAction(AUsar, ETriggerEvent::Started, this, &ALabPersonaje::Usar);
		E->BindAction(APocion, ETriggerEvent::Started, this, &ALabPersonaje::Pocion);
		E->BindAction(AFarol, ETriggerEvent::Started, this, &ALabPersonaje::CambiarFarol);
	}
}

const FLabArma& ALabPersonaje::ArmaActual() const
{
	static FLabArma Punos = [] { FLabArma A; A.Nombre = TEXT("Puños"); A.Ranura = TEXT("melee"); A.Danio = 6; A.Alcance = 11; A.Barrido = 1.2f; A.Enfriamiento = .35f; return A; }();
	const FLabArma* A = FLabDatos::Get().Armas.Find(ArmaId);
	return A ? *A : Punos;
}

void ALabPersonaje::Equipar(FName Id)
{
	ArmaId = Id;
	Arma->SetRelativeScale3D(FVector(FMath::Max(.3f, ArmaActual().Alcance / 30.f), .05f, .05f));
}

float ALabPersonaje::Visibilidad() const
{
	float V = 1;
	if (!bFarolPrendido) V *= .6f;
	if (bAgachado) V *= .55f;
	const float Rap = GetVelocity().Size2D();
	if (Rap > Lab::CmS(70)) V *= 1.25f; else if (Rap < 20) V *= .85f;
	if (Salud->Sangrado > 0) V *= 1.15f;
	return V;
}

void ALabPersonaje::Mover(const FInputActionValue& V) { Movimiento = V.Get<FVector2D>(); }

void ALabPersonaje::Rodar(const FInputActionValue& V)
{
	if (RodarT > 0 || RodarCd > 0 || Aire < 22 || !Salud->PuedeMoverse()) return;
	Aire -= 22;
	RodarT = .3f; RodarCd = .55f;
	RodarDir = Movimiento.SizeSquared() > .01f ? FVector(Movimiento.X, Movimiento.Y, 0).GetSafeNormal() : GetActorForwardVector();
	Salud->Invulnerable = .3f;
	bAgachado = false;
}

void ALabPersonaje::Agachar(const FInputActionValue& V) { bAgachado = !bAgachado; }

void ALabPersonaje::AtaqueNo(const FInputActionValue& V)
{
	bAtaqueApretado = false;
	const bool Cargado = Carga >= .45f && Aire >= 18;
	Carga = 0;
	if (Enfriamiento <= 0 && RodarT <= 0 && Salud->PuedeMoverse())
	{
		if (Cargado) Aire -= 18;
		Golpear(Cargado);
	}
}

void ALabPersonaje::Golpear(bool bCargado)
{
	const FLabArma& W = ArmaActual();
	if (bCargado) Combo = 0; else { Combo = ComboT > 0 ? (Combo % 3) + 1 : 1; ComboT = .75f; }
	const bool Fin = !bCargado && Combo == 3;
	const float Mult = bCargado ? 2.2f : Fin ? 1.6f : 1.f;
	const float Alc = Lab::Cm(W.Alcance * (bCargado ? 1.25f : 1.f));
	const float Arc = W.Barrido * (bCargado ? 1.25f : Fin ? 1.3f : 1.f);
	Enfriamiento = W.Enfriamiento * (bCargado ? 1.4f : Fin ? 1.25f : 1.f);
	GolpeT = .18f;
	const ELabGolpe Tipo = (ArmaId == TEXT("maza") || ArmaId == TEXT("martillo")) ? ELabGolpe::Contundente : ELabGolpe::Filo;
	const FVector Yo = GetActorLocation(), Frente = GetActorForwardVector();
	LaunchCharacter(Frente * (bCargado ? 450 : Fin ? 300 : 150), false, false);
	for (TActorIterator<ALabEnemigo> It(GetWorld()); It; ++It)
	{
		ALabEnemigo* E = *It;
		if (!E->Salud || !E->Salud->Vivo() || E->bDormido) continue;
		const FVector D = E->GetActorLocation() - Yo;
		const float Dist = D.Size2D(), R = E->GetCapsuleComponent()->GetScaledCapsuleRadius();
		if (Dist > Alc + R) continue;
		const float Ang = FMath::Acos(FVector::DotProduct(D.GetSafeNormal2D(), Frente));
		if (Ang > Arc / 2 && Dist > R + 40) continue;
		FLabEfectos Ef;
		if (W.Aturde > 0 && FMath::FRand() < W.ProbAturdir * (bCargado ? 1.6f : 1)) Ef.Aturde = W.Aturde;
		E->Salud->RecibirGolpe(W.Danio * Mult * Salud->MultDanio() * FMath::FRandRange(.9f, 1.1f), this, Tipo, Ef);
		E->AlRecibirGolpeDe(this);
		const float Emp = W.Empuje + (bCargado ? 6 : Fin ? 3 : 0);
		if (Emp > 0 && !E->bJefe) E->LaunchCharacter(D.GetSafeNormal2D() * Lab::Cm(Emp) * 6, true, false);
	}
}

FString ALabPersonaje::TextoUsar() const
{
	ALabModoJuego* M = ALabModoJuego::Get(this);
	if (M && FVector::Dist2D(GetActorLocation(), M->LugarSalida()) < Lab::Cm(24))
		return bTieneLlave ? (M->EscotillaAbierta() ? TEXT("E · Escapar") : TEXT("E · Abrir la escotilla (aguantá 10 s)")) : TEXT("Necesitás tu llave: matá a tu cazador.");
	for (TActorIterator<ALabCofre> It(GetWorld()); It; ++It)
		if (!It->bAbierto && FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < Lab::Cm(16)) return It->TextoAbrir();
	return FString();
}

void ALabPersonaje::Usar(const FInputActionValue& V)
{
	ALabModoJuego* M = ALabModoJuego::Get(this);
	if (M && FVector::Dist2D(GetActorLocation(), M->LugarSalida()) < Lab::Cm(24))
	{
		if (!bTieneLlave) { Mensaje(TEXT("Necesitás tu llave para salir.")); return; }
		M->UsarEscotilla(this);
		return;
	}
	for (TActorIterator<ALabCofre> It(GetWorld()); It; ++It)
		if (!It->bAbierto && FVector::Dist2D(It->GetActorLocation(), GetActorLocation()) < Lab::Cm(16)) { It->Abrir(this); return; }
}

void ALabPersonaje::Pocion(const FInputActionValue& V)
{
	if (Pociones <= 0 || Salud->Vida >= Salud->VidaMax) return;
	--Pociones;
	Salud->Curar(45, true);
	Mensaje(TEXT("Tomaste una poción."));
}

void ALabPersonaje::CambiarFarol(const FInputActionValue& V)
{
	bFarolPrendido = !bFarolPrendido;
	Farol->SetVisibility(bFarolPrendido);
}

void ALabPersonaje::ApuntarAlMouse()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	FHitResult H;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, H))
	{
		const FVector D = H.ImpactPoint - GetActorLocation();
		if (D.SizeSquared2D() > 100) SetActorRotation(FRotator(0, D.Rotation().Yaw, 0));
	}
	else if (Movimiento.SizeSquared() > .01f) SetActorRotation(FRotator(0, FVector(Movimiento.X, Movimiento.Y, 0).Rotation().Yaw, 0));
}

void ALabPersonaje::Tick(float Dt)
{
	Super::Tick(Dt);
	if (MensajeT > 0) MensajeT -= Dt;
	if (!Salud->Vivo() || bEscapo) return;
	Enfriamiento -= Dt; ComboT -= Dt; RodarCd -= Dt; GolpeT -= Dt;
	if (bAtaqueApretado && RodarT <= 0) Carga += Dt;
	ApuntarAlMouse();
	const bool Mov = Movimiento.SizeSquared() > .01f;
	const bool Corre = bCorriendo && Mov && Aire > 4 && RodarT <= 0 && !bAgachado;
	if (RodarT > 0)
	{
		RodarT -= Dt;
		GetCharacterMovement()->Velocity = RodarDir * Lab::CmS(165);
		AddMovementInput(RodarDir, 1);
	}
	else
	{
		Aire = FMath::Clamp(Aire + (Corre ? -21.f : 22.f) * Dt, 0.f, 100.f);
		const float Vel = (Corre ? Lab::CmS(88) : Lab::CmS(58)) * Salud->MultVelocidad() * (Carga > .15f ? .6f : 1.f) * (bAgachado ? .5f : 1.f);
		GetCharacterMovement()->MaxWalkSpeed = FMath::Max(1.f, Vel);
		if (Mov && Salud->PuedeMoverse()) AddMovementInput(FVector(Movimiento.X, Movimiento.Y, 0), 1);
	}
	// el arma acompaña el golpe (provisorio, hasta que haya animaciones con el esqueleto)
	const float F = GolpeT > 0 ? 1 - GolpeT / .18f : 0;
	Arma->SetRelativeRotation(FRotator(0, GolpeT > 0 ? FMath::Lerp(-60.f, 60.f, F) : (Carga > .15f ? 70.f : 0.f), 0));
	Cuerpo->SetRelativeScale3D(FVector(.5f, .6f, bAgachado ? .9f : 1.3f));
	Farol->SetAttenuationRadius(Salud->Ciego > 0 ? Lab::Cm(14) : Lab::Cm(64));
}

void ALabPersonaje::AlMorir(AActor* Victima, AActor* Asesino)
{
	GetCharacterMovement()->DisableMovement();
	if (ALabModoJuego* M = ALabModoJuego::Get(this)) M->Terminar(false, Asesino);
}
