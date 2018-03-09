// Fill out your copyright notice in the Description page of Project Settings.

#include "Relocator.h"
#include "ROSBridge/srv/RelocateSrv.h"

// Sets default values
ARelocator::ARelocator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARelocator::BeginPlay()
{
	Super::BeginPlay();

	// Set websocket server address to ws 
	Handler = MakeShareable<FROSBridgeHandler>(new FROSBridgeHandler(ServerAdress, ServerPort));

	// Add service clients and servers
	TSharedPtr<FROSRelocationServer> ServiceServer = MakeShareable<FROSRelocationServer>(new FROSRelocationServer(TEXT("relocation_service"), this));
	Handler->AddServiceServer(ServiceServer);


	// Connect to ROSBridge Websocket server.
	Handler->Connect();

}

// Called every frame
void ARelocator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	while (MoveAtNextTick.Num() != 0) {
		MoveAssetParams params = MoveAtNextTick.Pop(true);
		Relocate(params.Actor,
			params.Location,
			params.Rotator);
	}


}



void ARelocator::EndPlay(const EEndPlayReason::Type Reason)
{
	Handler->Disconnect();
	// Disconnect the handler before parent ends

	Super::EndPlay(Reason);
}


bool ARelocator::Relocate(AActor* Actor, FVector Location, FRotator Rotator)
{

	if (Actor->TeleportTo(Location, Rotator) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not set %s to locaiton: %s, with Rotation: %s"),
			*Actor->GetName(), *Location.ToString(), *Rotator.ToString());
		return false;
	}

	return true;
}


TSharedPtr<FROSBridgeSrv::SrvRequest> ARelocator::FROSRelocationServer::FromJson(TSharedPtr<FJsonObject> JsonObject) const
{
	TSharedPtr<FROSBridgeRelocatSrv::Request> Request_ =
		MakeShareable(new FROSBridgeRelocatSrv::Request());
	Request_->FromJson(JsonObject);
	return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request_);
}

TSharedPtr<FROSBridgeSrv::SrvResponse> ARelocator::FROSRelocationServer::Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request)
{

	TSharedPtr<FROSBridgeRelocatSrv::Request> Request_ =
		StaticCastSharedPtr<FROSBridgeRelocatSrv::Request>(Request);

	//Get Actor for given ID
	AActor** Actor = Parent->Controller->IDMap.Find(Request_->GetUtagId());


	if (!Actor) {
		// Couldn't find Actor for ID 
		return MakeShareable<FROSBridgeSrv::SrvResponse>
			(new FROSBridgeRelocatSrv::Response(false));
	}

	//Actor was found and will be relocated.
	//bool worked = Parent->Relocate(*Actor, Request_->GetLocation(), Request_->GetRotator());

	// Setup params
	MoveAssetParams Params;
	Params.Actor = *Actor;
	Params.Location = Request_->GetLocation();
	Params.Rotator = Request_->GetRotator();
	// Add to Que for next Tick
	Parent->MoveAtNextTick.Add(Params);

	return MakeShareable<FROSBridgeSrv::SrvResponse>
		(new FROSBridgeRelocatSrv::Response(true));
}
