// Fill out your copyright notice in the Description page of Project Settings.

#include "AssetTemplate.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "LootActor.h"




AAssetTemplate::AAssetTemplate()
{
	SetReplicates(true);
}

void AAssetTemplate::PopulateLootSockets()
{
	//Get all static meshes
	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);

	//only proceed if we have a mesh to work on
	if (Meshes.Num() > 0)
	{
		for (int i = 0; i < Meshes.Num(); i++)
		{
			//get the socket name list for this mesh
			TArray<FName> Socket_Names = Meshes[i]->GetAllSocketNames();

			//check each socket individually
			for (int a = 0; a < Socket_Names.Num(); a++)
			{
				//all sockets should contain the "Loot" keyword but if we use socket for other purposes this is merely for safety checks
				if (Socket_Names[a].ToString().Contains("Loot"))
				{
					//Spawning parameters
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					FVector SpawnLocation = Meshes[i]->GetSocketLocation(Socket_Names[a]);
					FRotator SpawnRotation = Meshes[i]->GetSocketRotation(Socket_Names[a]);

					//spawn our loot object
					ALootActor* SpawnedLoot = GetWorld()->SpawnActor<ALootActor>(ALootActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
					SpawnedLoot->SetOwner(this);
					
					//add it to the list
					Loot.Add(SpawnedLoot);
				}
			}
		}
	}
}
