#include "pch.h"
#include "MirrorMode.h"






void MirrorMode::mirrorCamera(SafetyHookContext& ctx)
{
	std::scoped_lock<std::mutex> lock(mDestructionGuard);
	PLOG_DEBUG << "calculating mirrored camera";
		matrix* cameraData = (matrix*)get().p_cameraData;

		matrix newMatrix = *cameraData * get().reflectionMatrix;
		PLOG_DEBUG << "mirrored camera calculated, writing";
		std::memcpy(cameraData, &newMatrix, sizeof(matrix));
		//patch_memory(cameraData, &newMatrix, sizeof(matrix));
        //*cameraData = newMatrix;
	
}