#pragma once

#include <d3d12.h>

namespace Floodlight {
	
	D3D12_RESOURCE_STATES& GetResourceState(ID3D12Resource* Resource);
	void TransitionResourceState(ID3D12Resource* Resource, D3D12_RESOURCE_STATES After);

}