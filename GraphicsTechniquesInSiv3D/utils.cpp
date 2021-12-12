#include "utils.h"

DirectX::XMVECTOR ToDXVec(const Vec3& vec)
{
	return DirectX::XMVectorSet(vec.x, vec.y, vec.z, 0.0f);
}
