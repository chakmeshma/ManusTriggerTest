// Fill out your copyright notice in the Description page of Project Settings.


#include "VirtualHandsActor.h"

// Sets default values
AVirtualHandsActor::AVirtualHandsActor() : Super()
{
	Init();
}

AVirtualHandsActor::AVirtualHandsActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Init();
}

AVirtualHandsActor::~AVirtualHandsActor()
{
	if (ManusDLLHandle) {
		if (bManusStarted)
			ManusShutDown();

		FPlatformProcess::FreeDllHandle(ManusDLLHandle);
		ManusDLLHandle = nullptr;
	}
}

// Called when the game starts or when spawned
void AVirtualHandsActor::BeginPlay()
{
	Super::BeginPlay();

	SetupNodes();

	SetupManus();
}

// Called every frame
void AVirtualHandsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *FPaths::ProjectPluginsDir());

	if (bManusConnected) {
		ManusGetData(skeletonNodes);

		/*RootComponent->SetWorldLocation(FVector{
			skeletonNodes[0].position.x,
			skeletonNodes[0].position.y,
			skeletonNodes[0].position.z });*/

		RootComponent->SetWorldRotation(FQuat{
			skeletonNodes[0 + NodesIndexOffset].rotation.x,
			skeletonNodes[0 + NodesIndexOffset].rotation.y,
			skeletonNodes[0 + NodesIndexOffset].rotation.z,
			skeletonNodes[0 + NodesIndexOffset].rotation.w });

		RootComponent->SetWorldScale3D(FVector{
			skeletonNodes[0 + NodesIndexOffset].scale.x * RootScale,
			skeletonNodes[0 + NodesIndexOffset].scale.y * RootScale,
			skeletonNodes[0 + NodesIndexOffset].scale.z * RootScale });

		for (uint16 i = 0; i < 20; i++) {
			HandNodes[i]->SetRelativeLocation(FVector{
				skeletonNodes[i + 1 + NodesIndexOffset].position.x * ChildPosMultiplier,
				skeletonNodes[i + 1 + NodesIndexOffset].position.y * ChildPosMultiplier,
				skeletonNodes[i + 1 + NodesIndexOffset].position.z * ChildPosMultiplier });

			HandNodes[i]->SetRelativeRotation(FQuat{
				skeletonNodes[i + 1 + NodesIndexOffset].rotation.x,
				skeletonNodes[i + 1 + NodesIndexOffset].rotation.y,
				skeletonNodes[i + 1 + NodesIndexOffset].rotation.z,
				skeletonNodes[i + 1 + NodesIndexOffset].rotation.w });

			HandNodes[i]->SetRelativeScale3D(FVector{
				skeletonNodes[i + 1 + NodesIndexOffset].scale.x,
				skeletonNodes[i + 1 + NodesIndexOffset].scale.y,
				skeletonNodes[i + 1 + NodesIndexOffset].scale.z });
		}
	}
}

void AVirtualHandsActor::Init()
{
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;

	RootNodeSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootNodeSceneComponent"));
	RootComponent = RootNodeSceneComponent;
}

void AVirtualHandsActor::SetupNodes()
{
	for (uint16 finger = 0; finger < 5; finger++)
	{
		for (uint16 digit = 0; digit < 4; digit++)
		{
			USceneComponent* parentComp = (digit == 0) ? RootComponent.Get() : HandNodes.Last();

			UStaticMeshComponent* newComp = NewObject<UStaticMeshComponent>(this, HandJointMeshComp);
			newComp->RegisterComponent();
			newComp->AttachToComponent(parentComp, FAttachmentTransformRules::KeepRelativeTransform);

			HandNodes.Add(newComp);
		}
	}
}

void AVirtualHandsActor::SetupManus()
{
	if (!ManusDLLHandle) {

		FString filePath = FPaths::Combine(*FPaths::ProjectDir(), TEXT("SDKMinimalClient.dll"));

		if (FPaths::FileExists(filePath))
		{
			ManusDLLHandle = FPlatformProcess::GetDllHandle(*filePath); // Load DLL

			if (ManusDLLHandle)
			{
				// DLL Loaded successfully, you can now call your functions
				UE_LOG(LogTemp, Warning, TEXT("ManusWrapper DLL Loaded"));

				ManusStartAndRun = reinterpret_cast<FPManusStartAndRun>(FPlatformProcess::GetDllExport(ManusDLLHandle, TEXT("StartAndRun")));
				ManusGetData = reinterpret_cast<FPManusGetData>(FPlatformProcess::GetDllExport(ManusDLLHandle, TEXT("GetData")));
				ManusShutDown = reinterpret_cast<FPManusShutDown>(FPlatformProcess::GetDllExport(ManusDLLHandle, TEXT("ShutDown")));

				UE_LOG(LogTemp, Warning, TEXT("ManusStartAndRun: %ld"), reinterpret_cast<uint64>(ManusStartAndRun));
				UE_LOG(LogTemp, Warning, TEXT("ManusGetData: %ld"), reinterpret_cast<uint64>(ManusGetData));
				UE_LOG(LogTemp, Warning, TEXT("ManusShutDown: %ld"), reinterpret_cast<uint64>(ManusShutDown));

				if (ManusStartAndRun()) {
					bManusStarted = true;
					bManusConnected = true;
				}

				if (bManusConnected) {
					UE_LOG(LogTemp, Warning, TEXT("Manus connected"));
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Manus couldn't connect"));
				}
			}
			else
			{
				// DLL Could not be loaded
				UE_LOG(LogTemp, Warning, TEXT("ManusWrapper DLL Load Error"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("ManusWrapper DLL Load Error (Not Found)"));
		}
	}
}


void* AVirtualHandsActor::ManusDLLHandle = nullptr;

FPManusStartAndRun AVirtualHandsActor::ManusStartAndRun = nullptr;
FPManusGetData AVirtualHandsActor::ManusGetData = nullptr;
FPManusShutDown AVirtualHandsActor::ManusShutDown = nullptr;

bool AVirtualHandsActor::bManusStarted = false;
bool AVirtualHandsActor::bManusConnected = false;