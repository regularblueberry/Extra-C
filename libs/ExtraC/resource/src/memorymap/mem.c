#include "./mem.h"
#include "./nodes.c"
#include "./leaf.c"



bool methodimpl(MemoryMap, validateMemID,, memid id){
	
	return 
		((memiddef*)&id)->nodenum < priv->maxvalidtoken.nodenum &&
		((memiddef*)&id)->leafnum < priv->maxvalidtoken.leafnum;
}

errvt methodimpl(MemoryMap, registerAllocator,, intf(Allocator) interface, void* allocator){
	
	nonull(allocator, return err)

	List.Append(priv->allocators, &(mmalloc){
		.alloc_interface = interface,
		.allocator = allocator,
		.num_resources = 0
	}, 1);

return OK;
}

Constructor(MemoryMap,
__METHODS(
	.AddNode = MemoryMap_AddNode,
	.GetNode = MemoryMap_GetNode,
	.CutNode = MemoryMap_CutNode,
	.MoveNode = MemoryMap_MoveNode,
	.AddLeaf = MemoryMap_AddLeaf,
	.GetLeaf = MemoryMap_GetLeaf,
	.CutLeaf = MemoryMap_CutLeaf,
	.MoveLeaf = MemoryMap_MoveLeaf,
	.validateMemID = MemoryMap_validateMemID,
	.registerAllocator = MemoryMap_registerAllocator
),
__PRIVATE(
	.maxvalidtoken = (memiddef){0},
	.nodes = new(Pool, sizeof(mmnode), 10),  
	.leafs = new(Pool, sizeof(mmleaf), 10),
	.active_nodes = newList(mmnode*, 10),
	.allocators = newList(mmalloc, 10)
),
	if(priv->nodes == NULL)
	    	{ERR(ERR_INITFAIL, "failed to create node pool"); break;}

	if(priv->leafs == NULL)
	    	{ERR(ERR_INITFAIL, "failed to create leaf pool"); break;}

	if(priv->active_nodes == NULL)
	    	{ERR(ERR_INITFAIL, "failed to create active nodes list"); break;}

	if(priv->allocators == NULL)
	    	{ERR(ERR_INITFAIL, "failed to create allocators list"); break;}
)



