ABASECHARACTER

ABaseCharacter()
{
	Allow actor to tick
	set default turn rates
	stop the camera from rotating based on the controller
	setup character movement
	turn off the capsule's physics
	
	create the PickupLoc scenecomponent and attach it to the root
	
	ensure HeldObject starts as nullptr
}

BeginPlay()
{
	call super
}

TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	call super
}

SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	call super
}

SetMaxSpeed(float speed)
{
	set the character movement component's MaxWalkSpeed to the argument
}

GetMaxSpeed()
{
	return the character movement component's MaxWalkSpeed
}

MoveForward(float Value)
{
	If the player has a controller
		AddMovementInput of the corrent direction and value on the X axis
}

MoveRight(float Value)
{
	If the player has a controller
		AddMovementInput of the corrent direction and value on the Y axis
}

Turn(float Val)
{
	if they aren't in the rotation submode
		increase Yaw
}

LookUp(float Val)
{
		if they aren't in the rotation submode
			Increase Pitch
}

TurnAtRate(float Rate)
{
	if they aren't in the rotation submode
			Increase Yaw based on their turn rate, the number passed in and deltaseconds
}

LookUpAtRate(float Rate)
{
	if they aren't in the rotation submode
			Increase Pitch based on their turn rate, the number passed in and deltaseconds
}

PerformRayCast(FName TraceProfile, FHitResult &OutHit)
{
	using the player's view point perform a raycast against the profile given and have the result populate the OutHit passed in.
	return the boolean success of the cast
}

Interact()
{
	if they are already interacting with something
		stop interacting with it, drop it and set HeldObject to nullptr
	else 
		create an FHitResult variable
		perform a raycast against GrabbableTrace, passing in the variable we just made
		
		if the raycast succeeded
			grab what we hit
}

Grab(FHitResult Hit)
{
	if the Hit.GetActor() isn't nullptr
		if it is grabbable
			heldobject = Cast it to a AGrabbableStaticMeshActor
			if that succeeded
				we are interacting now
				pickup the held object
		else if the actor is a trap
			nothing yet but maybe disarm eventually
}

ThrowObject()
{
	if heldobject isn't nullptr
		heldobject->Throw()
		set heldobject to nullptr
		set bisinteracting to false
}

PlaceTrap()
{
	nothing, this is actioned by derived classes
}

ZoomObject(float value)
{
	if we are holding somehting
		HeldObject->Zoom(Value)
}




ABASETRAPACTOR

ABaseTrapActor()
{
	allow this actor to tick
	Setup collision on the staticmeshcomponent
	set the staticmeshcomponment to simulate physics and gernate opverlap events
	
	register OnComponentBeginOverlap with the static mesh component
	
	Set the Default start and active meshes with constructionhelpers as Traps are never blueprinted
	
	Set this staticmesh component's mobility to movable so the meshes can change
	
	Add a tag "Trap"
}

TickActor(float DeltaSeconds)
{
	call super
}

HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if the trap hasn't already been triggred
		make sure the trap isn't being triggered by itself and that the other actor actually exists, also that the otheractor isn't this trap's owner
		
		cast the otheractor to ABaseCharacter
		if cast succeeded
			if we have a mesh for when activated
				Setstatic mesh to that
			Settarget to the casted actor
			ApplyDebuff()
			set bIsTriggered to true;
			
}

Die()
{
	Destroy();
}

SetTarget(class ABaseCharacter* character)
{
	m_Target = character;
}




ASLOWTRAPACTOR

ASlowTrapActor()
{
	Set the TrapDebuff enum variable to slow
	Set the debuff length
	
	set the 2 mesh variables
	SetStaticMesh to the first of those variables
}

ApplyDebuff()
{
	set TargetOriginalSpeed to the Target's current MaxSpeed
	change the target's max speed to much slower
	start the debufftimer, set to call RemoveDebuff and to the debufflength
}

RemoveDebuff()
{
	Set the target's speed back to its original
	Die
}





ASTOPTRAPACTOR

AStopTrapActor()
{
	Set the TrapDebuff enum variable to stop
	Set the debuff length
	
	set the 2 mesh variables
	SetStaticMesh to the first of those variables
}

ApplyDebuff()
{
	set TargetOriginalSpeed to the Target's current MaxSpeed
	change the target's max speed 0
	start the debufftimer, set to call RemoveDebuff and to the debufflength
}

RemoveDebuff()
{
	Set the target's speed back to its original
	Die
}




ADOORACTOR

ADoorActor()
{
	Setup collision, turn off physics and gravity on the staticmeshcomponent
	create the arrow component and attach it to the root
	
	create the sphere comp and turn off all physics
}

