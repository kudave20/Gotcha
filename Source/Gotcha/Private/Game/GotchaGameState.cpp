// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GotchaGameState.h"

void AGotchaGameState::ScoreTeam(ETeam Team)
{
	++TeamScores[Team];
}
