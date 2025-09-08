#define __NETWORK_SOURCE_DEF__
#include "posix.h"

socket_settings methodimpl(Socket, GetSettings){
	return priv->settings;
}

errvt methodimpl(Socket, Bind,, void* address){

	priv->sizeofaddr = XCAddrToUnix(
			priv->settings.domain,
			address,
			&priv->address
	);

	if(0 != priv->sizeofaddr ) return ERR( 
	      NETERR_SOCKBIND, "invalid socket configuration");
	
	if(bind(priv->fd, &priv->address, priv->sizeofaddr) < 0){ 
	  	priv->address = (struct sockaddr){0}; priv->sizeofaddr = 0;
       		return ERR(NETERR_SOCKBIND, "failed to bind to address");
	}

return OK;
}

errvt methodimpl(Socket, Listen,, u32 num_connects){
	nonull(self);

	if(-1 == listen(priv->fd, num_connects) ) return ERR(
		NETERR_SOCKLISTEN, "failed to listen for connections");
return OK;
}

inst(Connection) methodimpl(Socket, Accept){
	struct sockaddr address;
	socklen_t len;
	int fd;
	if(-1 ==(fd = accept(priv->fd, &address, &len)) ){
		ERR(NETERR_CONNECT, "could not accept incoming socket connection");
	      	return NULL;
	}
	
	inst(Connection) res = calloc(1, sizeof(Connection_Instance));
	res->__private = calloc(1, sizeof(Connection_Private));
	*res->__private = (Connection_Private){
		.settings = priv->settings,
		.fd = fd,
		.addresses[0] = address,
		.sizeofaddr = len
	};
	res->__methods = &Connection;
return res;
}

errvt methodimpl(Socket, GetAddress,, void* address){
	nonull(socket);
	nonull(address);

return unixAddrToXCAddr(priv->settings.domain, &priv->address, address);
}

errvt imethodimpl(Socket, Close,){
	self(Socket)

	nonull(socket);

	close(priv->fd);
return OK;
}


construct(Socket,
	.Accept = Socket_Accept,
	.Bind = Socket_Bind,
	.GetSettings = Socket_GetSettings,
	.Listen = Socket_Listen,
	.GetAddress = Socket_GetAddress,
	.Object = { .__DESTROY = Socket_Close }
){

	int domain = 
		args.settings.domain == SOCKET_DOMAIN_IPV4 ? AF_INET  :
		args.settings.domain == SOCKET_DOMAIN_IPV6 ? AF_INET6 :
		args.settings.domain == SOCKET_DOMAIN_LOCAL ? AF_LOCAL :
		-1;

	int protocol = 
		args.settings.protocol == SOCKET_PROTOCOL_UDP ? SOCK_DGRAM  :
		args.settings.protocol == SOCKET_PROTOCOL_TCP ? SOCK_STREAM :
		args.settings.protocol == SOCKET_PROTOCOL_RAW ? SOCK_RAW :
		-1;

	if(-1 == domain ) {ERR(
	      NETERR_SOCKINVAL, "invalid domain setting"); return NULL;}
	
	if(-1 == protocol ) {ERR(
	      NETERR_SOCKINVAL, "invalid protocol setting"); return NULL;}

	setpriv(Socket){
		.fd = socket(domain, protocol, 0),
		.settings = args.settings
	};
	
return self;
}
