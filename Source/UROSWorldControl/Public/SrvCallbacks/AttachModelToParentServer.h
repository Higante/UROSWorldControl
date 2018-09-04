#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROSBridgeHandler.h"
#include "ROSBridgeSrvServer.h"
#include "ROSBridgeSrv.h"
#include "UROSWorldControlManager.h"
#include "AttachModelToParent.h"


class FROSAttachModelToParentServer final : public FROSBridgeSrvServer
{
	
private:
	FROSAttachModelToParentServer(){};

	UWorld* World;
	FThreadSafeBool ServiceSuccess;

	UROSWorldControlManager* Controller;


public:
	FROSAttachModelToParentServer(FString Namespace, FString Name, UWorld* InWorld,
	                              UROSWorldControlManager* InController) :
		FROSBridgeSrvServer(Namespace + TEXT("/") + Name, TEXT("world_control_msgs/AttachModelToParent"))
	{
		World = InWorld;
		Controller = InController;
	}

	TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override;

	TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> Request) override;
};
