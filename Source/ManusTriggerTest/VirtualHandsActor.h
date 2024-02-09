// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <array>
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VirtualHandsActor.generated.h"

typedef struct ManusVec3
{
	float x;
	float y;
	float z;
} ManusVec3;

typedef struct ManusQuaternion
{
	float w;
	float x;
	float y;
	float z;
} ManusQuaternion;

typedef struct ManusTransform
{
	ManusVec3 position;
	ManusQuaternion rotation;
	ManusVec3 scale;
} ManusTransform;

typedef bool (*FPManusStartAndRun)();
typedef void (*FPManusGetData)(std::array<ManusTransform, 42>&);
typedef void (*FPManusShutDown)();

UCLASS()
class MANUSTRIGGERTEST_API AVirtualHandsActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVirtualHandsActor();
	AVirtualHandsActor(const FObjectInitializer& ObjectInitializer);
	~AVirtualHandsActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneComponent* RootNodeSceneComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<USceneComponent*> HandNodes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UStaticMeshComponent> HandJointMeshComp{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	double RootScale{};
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	double ChildPosMultiplier{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NodesIndexOffset{};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void Init();
	void SetupNodes();
	void SetupManus();

	static void* ManusDLLHandle;

	static FPManusStartAndRun ManusStartAndRun;
	static FPManusGetData ManusGetData;
	static FPManusShutDown ManusShutDown;

	static bool bManusStarted;
	static bool bManusConnected;
	std::array<ManusTransform, 42> skeletonNodes{};
};
