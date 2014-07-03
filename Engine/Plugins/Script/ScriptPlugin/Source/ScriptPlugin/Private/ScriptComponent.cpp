// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved. 
#include "ScriptPluginPrivatePCH.h"

//////////////////////////////////////////////////////////////////////////

UScriptComponent::UScriptComponent(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = false;
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	Context = NULL;
}

void UScriptComponent::OnRegister()
{
	Super::OnRegister();

	auto ScriptClass = UScriptBlueprintGeneratedClass::GetScriptGeneratedClass(GetClass());
	if (ScriptClass && GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
	{
		Context = ScriptClass->CreateContext();
		if (Context)
		{
			bool bDoNotTick = true;
			if (Context->Initialize(ScriptClass->SourceCode, this))
			{
				bDoNotTick = !Context->CanTick();
				// Push values set by CDO
				Context->PushScriptPropertyValues(ScriptClass, this);
			}

			if (bDoNotTick)
			{
				bAutoActivate = false;
				PrimaryComponentTick.bCanEverTick = false;
			}
		}
	}
}

void UScriptComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (Context)
	{
		auto ScriptClass = UScriptBlueprintGeneratedClass::GetScriptGeneratedClass(GetClass());
		Context->PushScriptPropertyValues(ScriptClass, this);
		Context->BeginPlay();
		Context->FetchScriptPropertyValues(ScriptClass, this);
	}
}

void UScriptComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Context)
	{
		auto ScriptClass = UScriptBlueprintGeneratedClass::GetScriptGeneratedClass(GetClass());
		Context->PushScriptPropertyValues(ScriptClass, this);
		Context->Tick(DeltaTime);
		Context->FetchScriptPropertyValues(ScriptClass, this);
	}
};

void UScriptComponent::OnUnregister()
{
	if (Context)
	{
		Context->Destroy();
		delete Context;
		Context = NULL;
	}

	Super::OnUnregister();
}

bool UScriptComponent::CallScriptFunction(FString FunctionName)
{
	bool bSuccess = false;
	if (Context)
	{
		auto ScriptClass = UScriptBlueprintGeneratedClass::GetScriptGeneratedClass(GetClass());
		Context->PushScriptPropertyValues(ScriptClass, this);
		bSuccess = Context->CallFunction(FunctionName);
		Context->FetchScriptPropertyValues(ScriptClass, this);
	}
	return bSuccess;
}
