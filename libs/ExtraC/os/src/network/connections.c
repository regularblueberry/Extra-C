#pragma once
#define __NETWORK_SOURCE_DEF__
#include "posix.h"


errvt methodimpl(Connection, Send,, msg_packet message){
	nonull(self);
	nonull(message.buff);

	if(-1 == send(priv->fd, message.buff, message.size, 0) ) 
		return ERR(NETERR_CONNSEND, "could not send data through connect");

	priv->io_ready &= 0;
return OK;
}
errvt methodimpl(Connection, Recieve,, msg_packet message){
	nonull(self);
	nonull(message.buff);
	
	if(-1 == recv(priv->fd, message.buff, message.size, 0) )
		return ERR(NETERR_CONNRECV, "could not recieve data through connect");
	priv->io_ready &= 0;
return OK;
}
errvt methodimpl(Connection, GroupSend,, msg_packet message){
	nonull(self);
	nonull(message.buff);

	if(-1 == sendto(priv->fd, 
		  	message.buff, 
		  	message.size, 
		  	0, 
		  	&priv->addresses[1], 
		  	priv->sizeofaddr) )
		return ERR(NETERR_CONNSEND, "could not send data through connect");
	priv->io_ready &= 0;
return OK;
}
errvt methodimpl(Connection, GroupRecive,, msg_packet message){
	nonull(self);
	nonull(message.buff);

	if(-1 == recvfrom(priv->fd, 
		  	message.buff, 
		  	message.size, 
		  	0, 
		  	&priv->addresses[1], 
		  	&priv->sizeofaddr) ) 
		return ERR(NETERR_CONNRECV, "could not recieve data through connect");
	priv->io_ready &= 0;
return OK;
}

errvt methodimpl(Connection, UnWatch){
	nonull(self);

	if(epoll_fd == 0){
		assert((epoll_fd = epoll_create1(0)) == -1);
	}
	if(connections == NULL){
		connections = newMap(Number, data(Connection));
		assert(connections != NULL);
	}

	if(!priv->watched) 
		return ERR(NETERR_WATCH, "connection is already being watched");

	if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_DEL, priv->fd, NULL) ) 
		return ERR(NETERR_WATCH, "could not set up watch for the connection");
	
	priv->watched = false;
return OK;
}
errvt methodimpl(Connection, Watch){
	nonull(self);

	if(epoll_fd == 0){
		assert((epoll_fd = epoll_create1(0)) == -1);
	}
	if(connections == NULL){
		connections = newMap(Number, data(Connection));
		assert(connections != NULL);
	}

	if(priv->watched)
		return ERR(NETERR_WATCH, "connection is already being watched");

	if(-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, priv->fd, NULL) )
		return ERR(NETERR_WATCH, "could not set up watch for the connection");
	
	priv->watched = true;
return OK;
}
bool methodimpl(Connection, Check){
	
	nonull(self, return false;)

	if(epoll_fd == 0) {
		assert((epoll_fd = epoll_create1(0)) == -1);
		return false;
	}
	if(connections == NULL){
		connections = newMap(Number, data(Connection));
		assert(connections != NULL);
		return false;
	}

	if(!priv->watched) {
	      ERR(NETERR_WATCH, "connection is not being watched");
		return false;
	}

	if(priv->io_ready) return true;
	else{
		struct epoll_event events[10];
		int num_events = 0;
		for(;;){
		    if(-1 == (num_events = epoll_wait(epoll_fd, events, 10, 0)) ){
			    ERR(NETERR_WATCH, "failed to check epoll queue");
			    return false;
			}
		    if(num_events == 0) break;
		    loop(i, 10){
			inst(Connection) connection =
				Map.Search(connections, n(events[i].data.fd));
			
			assert(connection->__private->fd == events[i].data.fd);

			assert(connection->__private->watched);

			connection->__private->io_ready = true;
		    }

		}
	}
	if(priv->io_ready) return true;

return false;
}
errvt methodimpl(Connection, GroupJoin,, void* address, void* interface_addr){

	nonull(self);
	nonull(address);

	if(priv->settings.domain != SOCKET_PROTOCOL_UDP ||
	   priv->settings.domain != SOCKET_PROTOCOL_RAW) 
		return ERR(
			NETERR_CONNECT, "only UPD and RAW protocols can join a multicast group");

	if(0 == XCAddrToUnix(
		    priv->settings.domain, 
		    address,
		    &priv->addresses[1]) == 0 ) return ERR(
		NETERR_CONNECT, "invalid connection configuration");

	switch (priv->settings.domain) {

	case SOCKET_DOMAIN_IPV4:{

		struct ip_mreq group_address = {
			.imr_multiaddr = ((struct sockaddr_in*)&priv->addresses[1])->sin_addr,
			.imr_interface = inet_addr("0.0.0.0")
		};

		if(interface_addr != NULL){
			struct sockaddr_in temp;
			XCAddrToUnix(
			    priv->settings.domain, 
			    address,
			    (struct sockaddr*)&temp
			);
			group_address.imr_interface = temp.sin_addr;	
		}

			if(setsockopt(
			    priv->fd,
			    IPPROTO_IP,
			    IP_ADD_MEMBERSHIP,
			    &group_address,
			    sizeof(struct ip_mreq)) < 0)
			{
				ERR(NETERR_CONNECT, "could not join group");
				*(struct sockaddr_in*)&priv->addresses[1] = 
					(struct sockaddr_in){0};
			}
		 
	break;}
	case SOCKET_DOMAIN_IPV6:{
			
		struct ipv6_mreq group_address = {
			.ipv6mr_multiaddr = ((struct sockaddr_in6*)&priv->addresses[1])->sin6_addr,
			.ipv6mr_interface = interface_addr == NULL ?
				0 :
				if_nametoindex(interface_addr)
		};

			if(setsockopt(
			    priv->fd,
			    IPPROTO_IPV6,
			    IPV6_ADD_MEMBERSHIP,
			    &group_address,
			    sizeof(struct ipv6_mreq)))
			{
				ERR(NETERR_CONNECT, "could not join group");
				*(struct sockaddr_in6*)&priv->addresses[1] = (struct sockaddr_in6){0};
		 	}
	break;}
	default:{
	    return 
		ERR(NETERR_CONNECT, "multicast groups are not avaliable for this domain");
	}
	}
}
errvt methodimpl(Connection,GroupLeave){
	nonull(self);

	switch (priv->settings.domain) {
	case SOCKET_DOMAIN_IPV4:{
		if(setsockopt(
			priv->fd,
			IPPROTO_IP,
			IP_ADD_MEMBERSHIP,
			&priv->addresses[1],
			sizeof(struct ip_mreq)) < 0)
		{
			ERR(NETERR_CONNECT, "could not leave group");
			*(struct sockaddr_in*)&priv->addresses[1] = (struct sockaddr_in){0};
		}
	break;}
	case SOCKET_DOMAIN_IPV6:{
		if(setsockopt(
			priv->fd,
			IPPROTO_IP,
			IP_ADD_MEMBERSHIP,
			&priv->addresses[1],
			sizeof(struct ip_mreq)) < 0)
		{
		 	ERR(NETERR_CONNECT, "could not join group");
			*(struct sockaddr_in6*)&priv->addresses[1] = (struct sockaddr_in6){0};
		}
	break;}
	default:{
	    return 
		ERR(NETERR_CONNECT, "multicast groups are not avaliable for this domain");
	}
	}
return OK;
}

socket_settings methodimpl(Connection, GetSettings){
	nonull(self, return (socket_settings){0};);
	return priv->settings;
}
errvt methodimpl(Connection, GetAddress,, void* address, void* multicast_address){
	nonull(self)	
	nonull(address)	

	errvt error = unixAddrToXCAddr(
		priv->settings.domain, 
		&priv->addresses[0],
		address);

	if(error) return error;

	if(multicast_address)
	    error = unixAddrToXCAddr(
		priv->settings.domain, 
		&priv->addresses[1],
		multicast_address);
return error;
}
errvt imethodimpl(Connection, Close){
	self(Connection)
	nonull(self);
	
	if(priv->watched)
		Connection.UnWatch(self);
	
	close(priv->fd);

return OK;
}



construct(Connection,
	.GetSettings = Connection_GetSettings,
	.UnWatch = Connection_UnWatch,
	.Watch = Connection_Watch,
	.Check = Connection_Check,
	.GroupJoin = Connection_GroupJoin,
	.GroupSend = Connection_GroupSend,
	.GroupRecive = Connection_GroupRecive,
	.GroupLeave = Connection_GroupLeave,
	.Send = Connection_Send,
	.Recieve = Connection_Recieve,
	.Object = {.__DESTROY = Connection_Close}
){
	
	if(epoll_fd == 0){
		assert((epoll_fd = epoll_create1(0)) == -1);
	}
	if(connections == NULL){
		connections = newMap(Number, data(Connection));
		assert(connections != NULL);
	}

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

	if(-1 == domain ){
	      ERR(NETERR_CONNECT, "invalid domain setting");
	      return NULL;
	}
	
	if(-1 == protocol ){
	      ERR(NETERR_CONNECT, "invalid domain setting");
	      return NULL;
	}

	setpriv(Connection){
		.fd = socket(domain, protocol, 0),
		.settings = args.settings,
		.sizeofaddr = XCAddrToUnix(
			args.settings.domain,
			args.address,
			&priv->addresses[0]
		)
	};
	if(-1 == priv->fd ) {
	      ERR(NETERR_CONNECT, "could not create connection socket");
	      ; return NULL;
	}

	if(-1 == connect(priv->fd, &priv->addresses[0], priv->sizeofaddr) ) {
	      ERR(NETERR_CONNECT , "could not initialize connection");
	      ; return NULL;
	}
	
	Map.Insert(connections, n(priv->fd), &self);

return self;
}
