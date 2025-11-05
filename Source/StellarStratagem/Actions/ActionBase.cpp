#include "ActionBase.h"

FActionResult UActionBase::PerformAction(AGameManager* GameManager, AStellarPlayerController* Player)
{
	return {false, "Action did not go through. Please restart game."};
}
