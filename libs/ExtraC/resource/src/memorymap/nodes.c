#include "./mem.h"

errvt getNodeFromRelation(mmnode* startnode, mmnode** returnnode ,char relation[]){
	u32 count = 0;
	mmnode* currentnode = startnode;
	char numstore[5], optstore[1];

	while(relation[count]!= '\0'){
		numstore[0] = '1';
		numstore[1] = '\0';

		optstore[0] = relation[count];

		//getting the number of times to loop
		if(relation[count+1] > '0' && relation[count+1] < '9'){
			loop(i, 5){
				numstore[i] = relation[count+1];
				count++;
				if(relation[count+1] > '0' && relation[count+1] < '9')
					continue;
				else
				 	break;
			}
		}
		switch (relation[count]) {
		case 'v':{
			loop(i, atoi(numstore)){
			    if(currentnode->firstchild == NULL) return ERR(
		   		RESOURCERR_TOKENINDEX, "Failed to index firstchild since it is null");
			    currentnode = currentnode->firstchild; 
			}
		break;}
		case '>':{
			loop(i, atoi(numstore)){
			    if(currentnode->nextsibling == NULL) return ERR(
				RESOURCERR_TOKENINDEX, "Failed to index nextsibling since it is null"); 
			    currentnode = currentnode->nextsibling; 
			}
		break;}
		case '<':{
			
			u32 childnum = 0, repeat = 0, childsectnum = currentnode->token.nodenum;
			currentnode = currentnode->parent;
			loop(i, atoi(numstore))
				repeat++;


			childnum = childsectnum - currentnode->token.nodenum;
			currentnode = currentnode->firstchild;

			loop(i, childnum - repeat)
				currentnode = currentnode->nextsibling;
		break;}
		case '^':{
			loop(i, atoi(numstore)){
			    if(currentnode->parent == NULL) return ERR(
				RESOURCERR_TOKENINDEX, "Failed to index parent since it is null");
			    currentnode = currentnode->parent;
			}	
		break;}
		}
	}
return OK;
}

errvt freeNode(inst(MemoryMap) self, mmnode* node){
	
	u32* leafs = List.GetPointer(node->contents, 0);
	Pool.Allocator.Delete(priv->nodes, node);

	del(node->contents);
	
	*node = (mmnode){0};
	
return OK;
}

errvt addNode(inst(MemoryMap) self, mmnode* maptonode, memid* returntoken){
	
	mmnode* currentnode,** mappednode;

	if(maptonode->childrennum > 0){
		currentnode = maptonode->firstchild;

		// we subtract 1 because we start at the map to nodes first child
		loop(i, maptonode->childrennum - 1){
			currentnode = currentnode->nextsibling;
		}
		mappednode = &currentnode->nextsibling;
	}
	else {	currentnode = maptonode;
		mappednode = &currentnode->firstchild; }


	(*mappednode) = Pool.Allocator.New(priv->nodes, 1);
	if((*mappednode) == NULL) return ERR(
		RESOURCERR_TOKENCREATE, "failed to get a new node");
	
	*returntoken = *(memid*)&currentnode->token;

return OK;
}

errvt getNode(inst(MemoryMap) self,mmnode** returnnode, memiddef token){

	mmnode* currentnode;

	currentnode = List.GetPointer(priv->active_nodes, token.nodenum);

	if(currentnode == NULL) return ERR(
		RESOURCERR_TOKENINDEX, "unable to get node");

	*returnnode = currentnode;

return OK;
}

errvt methodimpl(MemoryMap, GetNode,, memid token, mmnode_info* metadata){
	
	mmnode* node = NULL;

	nonull(self, return err);
	nonull(metadata, return err;);

	if(MemoryMap.validateMemID(self, token))
		return ERR(RESOURCERR_TOKENINDEX, "invalid memid");
	
	
	if(getNode(self, &node, *(memiddef*)&token) != ERR_NONE) 
		return ERR(RESOURCERR_TOKENINDEX, "unable to get node");
	
	*metadata = (mmnode_info){
		.childrennum = node->childrennum,
		.contentsnum = List.Size(node->contents),
		.protections = node->protections,
	};


return OK;
};
memid methodimpl(MemoryMap, AddNode,, memid maptotoken, mmprot_info* metadata){

	mmnode* maptonode = NULL, * node = NULL;

	nonull(self, return err);

	if(MemoryMap.validateMemID(self, maptotoken)) return ERR(
		RESOURCERR_TOKENINDEX, "invalid memid");

	if(getNode(self, &maptonode, *(memiddef*)&maptotoken)) return ERR( 
		RESOURCERR_TOKENINDEX, "could not get node");

	memid result = 0;
	if(addNode(self, maptonode, &result)) return ERR( 
		RESOURCERR_TOKENCREATE, "could not add node");
	
	getNode(self, &maptonode, *(memiddef*)&result);

	maptonode->childrennum++;
	
	if(maptonode->firstchild == NULL)
		maptonode->firstchild = node;
	else{
		mmnode* child_node = maptonode->firstchild;
		
		while(child_node->nextsibling != NULL)
			child_node = child_node->nextsibling;
		
		child_node->nextsibling = node;
	}

	*node = (mmnode){
		.protections = metadata->protections,
		.contents = newList(mmleaf*, 10),
		.token = *(memiddef*)result,
		.childrennum = 0,
		.firstchild = NULL,
		.nextsibling = NULL,
		.parent = maptonode
	};

return OK;
}
errvt methodimpl(MemoryMap, CutNode,, memid token){

	mmnode* prevnode,* currentnode;
	
	if(getNode(self, &currentnode ,*(memiddef*)&token)) return ERR(
	   RESOURCERR_TOKENMODIFY, "could not get nodes from specified token");
	

	if(getNodeFromRelation(currentnode, &prevnode, "<")) return ERR(
	   RESOURCERR_TOKENMODIFY, "could not get nodes from specified token");
		
	//case for root
	if(prevnode->token.nodenum == currentnode->token.nodenum){
		freeEntireTree(self);
	}
	else {
	   if(prevnode->nextsibling != NULL && prevnode->nextsibling->token.nodenum != ((memiddef*)&token)->nodenum)
		prevnode->nextsibling = prevnode->nextsibling->nextsibling;
	   else 
		prevnode->firstchild = prevnode->firstchild->nextsibling;
		
	freeNode(self, currentnode);
	}

return OK;


}
errvt methodimpl(MemoryMap, MoveNode,, memid* token, memid maptotoken){

	//NOT YET IMPLEMENTED

return OK;
}

