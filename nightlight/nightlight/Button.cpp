#include "Button.h"

Button::Button(XMFLOAT3 position, float rotation, RenderObject* renderObject, int coordX, int coordY, std::string activatesName)
	: GameObject(position, rotation, renderObject, coordX, coordY)
{
	this->activatesName = activatesName;
	this->isActivated = isActivated;
	this->coordX = coordX;
	this->coordY = coordY;
	this->clickID = clickID;
	this->clicked = clicked;
}

Button::~Button()
{
}

void Button::ActivateStartButton()
{
	isActivated = true;

	setClickID(1);

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y -= 45.0f;
	this->SetRotationDeg(rot);

	if (activatesStartButton!= nullptr) {
		activatesStartButton->setIsStartActivated(true);
	}
}

void Button::DeactivateStartButton()
{
	isActivated = false;

	setClickID(0);

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y += 45.0f;
	this->SetRotationDeg(rot);

	if (activatesStartButton != nullptr) {

		activatesStartButton->DeactivateStartButton();

		activatesStartButton->setIsStartActivated(false);
	}
}


void Button::ActivateContinueButton()
{
	isActivated = true;

	setClickID(2);

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y -= 25.0f;
	this->SetRotationDeg(rot);

	if (activatesContinueButton != nullptr) {
		activatesContinueButton->setIsContinueActivated(true);
	}
}


void Button::DeactivateContinueButton()
{
	isActivated = false;

	setClickID(0);

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y += 25.0f;
	this->SetRotationDeg(rot);

	if (activatesContinueButton != nullptr) {
		activatesContinueButton->DeactivateContinueButton();
		activatesContinueButton->setIsContinueActivated(false);
	}
}


void Button::ActivateExitButton()
{
	isActivated = true;

	setClickID(3);

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y -= 15.0f;
	this->SetRotationDeg(rot);

	if (activatesExitButton != nullptr) {
		activatesExitButton->setIsExitActivated(true);
	}
}

void Button::DeactivateExitButton()
{
	isActivated = false;

	setClickID(0);

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y += 15.0f;
	this->SetRotationDeg(rot);

	if (activatesExitButton != nullptr) {
		activatesExitButton->DeactivateExitButton();
		activatesExitButton->setIsExitActivated(false);
	}
}

void Button::ClickedStart()
{
	clicked = true;

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y += 5.0f;
	this->SetRotationDeg(rot);

	//if (activatesStartButton != nullptr) {
	//	activatesStartButton->setIsStartActivated(false);
	//}
}

void Button::ClickedContinue()
{
	clicked = true;

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y += 90.0f;
	this->SetRotationDeg(rot);

	//if (activatesStartButton != nullptr) {
	//	activatesStartButton->setIsStartActivated(false);
	//}
}

void Button::ClickedEnd()
{
	clicked = true;

	XMFLOAT3 rot = this->GetRotationDeg();
	rot.y += 50.0f;
	this->SetRotationDeg(rot);

	//if (activatesStartButton != nullptr) {
	//	activatesStartButton->setIsStartActivated(false);
	//}
}