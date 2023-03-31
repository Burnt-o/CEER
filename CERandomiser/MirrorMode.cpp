#include "pch.h"
#include "MirrorMode.h"






void MirrorMode::mirrorCamera(SafetyHookContext& ctx)
{

		matrix* cameraData = (matrix*)get().p_cameraData;

		matrix newMatrix = *cameraData * get().reflectionMatrix;
        *cameraData = newMatrix;
	
}