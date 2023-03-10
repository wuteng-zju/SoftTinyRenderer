#pragma once
#include "Camera.h"

RENDERER_NAMESPACE_BEGIN

class OrthoGraphicCamera :public Camera
{
public:
	~OrthoGraphicCamera() = default;
	OrthoGraphicCamera();
	OrthoGraphicCamera(float fSize, float fAspectRatio, float fNearClip = -10.f, float fFarClip = 10.f);

	void SetOrthoSize(float fSize);
	float GetOrthoSize()const;

private:
	virtual void RecalculateProjectionMatrix() override;
private:
	float m_fOrthoGraphicSize;
};

RENDERER_NAMESPACE_END