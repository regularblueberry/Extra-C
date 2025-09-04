#include "../backends/dbus.c"

#include "__systems.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define ipv4addr_to_unix(ipv4addr) (*((in_addr_t*)ipv4addr))
#define ipv6addr_to_unix(ipv6addr) (*((struct in6_addr*)ipv6addr))
#define as_xcaddr(type, ptr) (*((type##Address*)ptr))
#define as_unaddr(type, ptr) (*((struct sockaddr_##type*)ptr))

static socklen_t XCAddrToUnix(u8 type, socketAddress* xc_addr, struct sockaddr* unix_addr){

	switch(type){
	case D_IPV4:{
	
		memcpy(unix_addr, &(struct sockaddr_in){
 			.sin_family = AF_INET,
 			.sin_port = xc_addr->data.ipv4.port,
 			.sin_addr = ipv4addr_to_unix(xc_addr->data.ipv4.address)
 		},
   		sizeof(struct sockaddr_in));
	return sizeof(struct sockaddr_in);
  	}
	case D_IPV6:{
 		memcpy(unix_addr, &(struct sockaddr_in6){
 			.sin6_family = AF_INET6,
 			.sin6_port = xc_addr->data.ipv6.port,
 			.sin6_addr = ipv6addr_to_unix(xc_addr->data.ipv6.address)
 		},
   		sizeof(struct sockaddr_in));
	return sizeof(struct sockaddr_in);
	}
	case D_LOCAL:{
 		memcpy(unix_addr, &(struct sockaddr_un){
			.sun_family = AF_LOCAL,
			.sun_path = {0}
 		},
   		sizeof(struct sockaddr_un));
		strncpy(
			((struct sockaddr_un*)unix_addr)->sun_path,
			xc_addr->data.local.path->txt,
			100
		);
	return sizeof(struct sockaddr_un);
	}
	default:{
		ERR(NETERR_SOCKINVAL, "invalid domain type");
	}
	}
return 0;
}
static errvt unixAddrToXCAddr(u8 type, struct sockaddr* unix_addr, socketAddress* xc_addr){

	switch (type) {
	case D_IPV4:{
		xc_addr->data.ipv4 = (ipv4Address){
			.address = {
				((u8*)&(as_unaddr(in, unix_addr).sin_addr.s_addr))[0],
				((u8*)&(as_unaddr(in, unix_addr).sin_addr.s_addr))[1],
				((u8*)&(as_unaddr(in, unix_addr).sin_addr.s_addr))[2],
				((u8*)&(as_unaddr(in, unix_addr).sin_addr.s_addr))[3],
			},
			.port = as_unaddr(in, unix_addr).sin_port
		};
	}
	case D_IPV6:{
		xc_addr->data.ipv6 = (ipv6Address){
			.address = {
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[0],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[1],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[2],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[3],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[4],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[5],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[6],
				((u16*)&(as_unaddr(in6, unix_addr).sin6_addr))[7],
			},
			.port = as_unaddr(in, unix_addr).sin_port
		};
	}
	case D_LOCAL:{
		xc_addr->data.local = (localAddress){
			.path = str_cast(as_unaddr(un, unix_addr).sun_path, 100)
		};
	}
	default:{
		return ERR(NETERR_SOCKINVAL, "invalid domain type");
	}
	}

return OK;
}

errvt vmethodimpl(LinuxNetwork, initSystem){
	iferr(initNetObjectSystem()){
		return err;
	}
return OK;
}
errvt vmethodimpl(LinuxNetwork, exitSystem){
	iferr(exitNetObjectSystem()){
		return err;
	}
return OK;
}

networkHandle vmethodimpl(LinuxNetwork, SocketInit, socketType type){
	if(type > 0b1111){
		ERR(NETERR_SOCKINVAL, "invalid socket type out of range");
		return NULL;
	}
	int protocalFlag = (type & 0b11), domainFlag = (type & 0b1100) >> 2; 


	int domain = 
		domainFlag == D_IPV4  ? AF_INET  :
		domainFlag == D_IPV6  ? AF_INET6 :
		domainFlag == D_LOCAL ? AF_LOCAL :
		-1;

	int protocal = 
		protocalFlag == P_UDP ? SOCK_DGRAM  :
		protocalFlag == P_TCP ? SOCK_STREAM :
		protocalFlag == P_RAW ? SOCK_RAW :
		-1;

	if(-1 == domain ) {ERR(
	      NETERR_SOCKINVAL, "invalid domain type"); return NULL;}
	
	if(-1 == protocal ) {ERR(
	      NETERR_SOCKINVAL, "invalid protocal type"); return NULL;}


	LinuxNetworkHandle* result = new(LinuxNetworkHandle,
		.type = NetData_Socket,
		.data.socket = {
			.fd = socket(domain, protocal, 0),
			.protocal = protocal,
			.domain = domain,
		}		
	);

return result;
}
errvt vmethodimpl(LinuxNetwork, SocketBind, networkHandle handle, socketAddress* address){
	nonull(handle, return err);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	socket->sizeofaddr = XCAddrToUnix(
			socket->domain,
			address,
			&socket->address
	);

	if(0 != socket->sizeofaddr) return ERR( 
	      NETERR_SOCKBIND, "invalid socket configuration");
	
	if(bind(socket->fd, &socket->address, socket->sizeofaddr) < 0){ 
	  	socket->address = (struct sockaddr){0}; socket->sizeofaddr = 0;
       		return ERR(NETERR_SOCKBIND, "failed to bind to address");
	}

return OK;
}
errvt vmethodimpl(LinuxNetwork, SocketListen, networkHandle handle, u32 num_connect){
	nonull(handle, return err);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	if(-1 == listen(socket->fd, num_connect)) return ERR(
		NETERR_SOCKLISTEN, "failed to listen for connections");
return OK;
}
networkHandle vmethodimpl(LinuxNetwork, SocketConnect, socketType type, socketAddress* address){

	int protocalFlag = (type & 0b11), domainFlag = (type & 0b1100) >> 2; 

	if(domainFlag != address->type){
		ERR(ERR_INVALID, "socket domain type does not match address type");
		return NULL;
	}

	LinuxNetworkHandle* handle = LinuxNetwork_SocketInit(type);

	if(!handle){
		return NULL;
	}
	
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	socket->sizeofaddr = XCAddrToUnix(address->type, address, &socket->address);

	if(-1 == connect(socket->fd, &socket->address, socket->sizeofaddr) ) {
	      ERR(NETERR_CONNECT , "could not initialize connection");
	      return NULL;
	}

return handle;
}
networkHandle vmethodimpl(LinuxNetwork, SocketAccept, networkHandle handle){
	nonull(handle, return NULL);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	struct sockaddr address;
	socklen_t len;
	int fd;
	if(-1 ==(fd = accept(socket->fd, &address, &len)) ){
		ERR(NETERR_CONNECT, "could not accept incoming socket connection");
	      	return NULL;
	}
	
	LinuxNetworkHandle* result = new(LinuxNetworkHandle,
		.type = NetData_Socket,
		.data.socket = {
			.address = address,
			.sizeofaddr = len,
			.fd = fd,
			.blocking = socket->blocking,
			.domain = socket->domain,
			.protocal = socket->protocal,
		}
	);

return result;
}
errvt vmethodimpl(LinuxNetwork, SocketSend, networkHandle handle, inst(Buffer) message){
	nonull(handle, return NULL);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	if(-1 == send(socket->fd, Buffer.getPointer(message), Buffer.getTotalSize(message), 0) ) 
		return ERR(NETERR_CONNSEND, "could not send data through connect");
return OK;
}
errvt vmethodimpl(LinuxNetwork, SocketRecv, networkHandle handle, inst(Buffer) message){
	nonull(handle, return NULL);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);
	
	if(-1 == recv(socket->fd, Buffer.getPointer(message), Buffer.Allocator.getBytesAlloced(generic message), 0) )
		return ERR(NETERR_CONNRECV, "could not recieve data through connect");
return OK;

}
errvt vmethodimpl(LinuxNetwork, SocketGroupJoin, networkHandle handle, socketAddress address, const char* interface_name){
	nonull(handle, return err);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	if(socket->domain != P_UDP ||
	   socket->domain != P_RAW) 
		return ERR(
			NETERR_CONNECT, "only UPD and RAW protocals can join a multicast group");

	if(0 == XCAddrToUnix(
		    socket->domain, 
		    &address,
		    &socket->group_address) == 0 ) return ERR(
		NETERR_CONNECT, "invalid connection configuration");

	switch (socket->domain) {

	case D_IPV4:{

		struct ip_mreq group_address = {
			.imr_multiaddr = ((struct sockaddr_in*)&socket->group_address)->sin_addr,
			.imr_interface = inet_addr("0.0.0.0")
		};

		struct sockaddr_in temp = *(struct sockaddr_in*)&socket->group_address;
		group_address.imr_interface = temp.sin_addr;	

		if(setsockopt(
		    socket->fd,
		    IPPROTO_IP,
		    IP_ADD_MEMBERSHIP,
		    &group_address,
		    sizeof(struct ip_mreq)) < 0)
		{
			ERR(NETERR_CONNECT, "could not join group");
			*(struct sockaddr_in*)&socket->group_address = 
				(struct sockaddr_in){0};
		}
	break;}
	case D_IPV6:{
			
		struct ipv6_mreq group_address = {
			.ipv6mr_multiaddr = ((struct sockaddr_in6*)&socket->group_address)->sin6_addr,
			.ipv6mr_interface = interface_name == NULL ?
				0 :
				if_nametoindex(interface_name)
		};

		if(setsockopt(
		    socket->fd,
		    IPPROTO_IPV6,
		    IPV6_ADD_MEMBERSHIP,
		    &group_address,
		    sizeof(struct ipv6_mreq)))
		{
			ERR(NETERR_CONNECT, "could not join group");
			*(struct sockaddr_in6*)&socket->group_address = (struct sockaddr_in6){0};
		}
	break;}
	default:{
	    return 
		ERR(NETERR_CONNECT, "multicast groups are not avaliable for this domain");
	}
	}
return OK;
}
errvt vmethodimpl(LinuxNetwork, SocketGroupLeave, networkHandle handle){
	nonull(handle, return err);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	switch (socket->domain) {
	case D_IPV4:{
		if(setsockopt(
			socket->fd,
			IPPROTO_IP,
			IP_ADD_MEMBERSHIP,
			&socket->group_address,
			sizeof(struct ip_mreq)) < 0)
		{
			ERR(NETERR_CONNECT, "could not leave group");
			*(struct sockaddr_in*)&socket->group_address = (struct sockaddr_in){0};
		}
	break;}
	case D_IPV6:{
		if(setsockopt(
			socket->fd,
			IPPROTO_IP,
			IP_ADD_MEMBERSHIP,
			&socket->group_address,
			sizeof(struct ip_mreq)) < 0)
		{
		 	ERR(NETERR_CONNECT, "could not join group");
			*(struct sockaddr_in6*)&socket->group_address = (struct sockaddr_in6){0};
		}
	break;}
	default:{
	    return 
		ERR(NETERR_CONNECT, "multicast groups are not avaliable for this domain");
	}
	}
return OK;

}
errvt vmethodimpl(LinuxNetwork, SocketGroupSend, networkHandle handle, inst(Buffer) message){
	nonull(handle, return err);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	if(-1 == sendto(socket->fd, 
		  	Buffer.getPointer(message), 
		  	Buffer.getTotalSize(message), 
		  	0, 
		  	&socket->group_address, 
		  	socket->sizeofaddr) )
		return ERR(NETERR_CONNSEND, "could not send data through connect");

}
errvt vmethodimpl(LinuxNetwork, SocketGroupRecive, networkHandle handle, inst(Buffer) message){
	nonull(handle, return err);
	shortName(((LinuxNetworkHandle*)handle)->data.socket, socket);

	if(-1 == recvfrom(socket->fd, 
		  	Buffer.getPointer(message), 
		  	Buffer.Allocator.getBytesAlloced(generic message), 
		  	0, 
		  	&socket->group_address, 
		  	&socket->sizeofaddr) ) 
		return ERR(NETERR_CONNRECV, "could not recieve data through connect");
return OK;
}

	
networkHandle vmethodimpl(LinuxNetwork, DeviceGrab, networkDevice* device);
errvt vmethodimpl(LinuxNetwork, DeviceDrop, networkHandle);
networkDevice* vmethodimpl(LinuxNetwork, DeviceEnumerate, u64* numDevices);
errvt vmethodimpl(LinuxNetwork, DeviceFilter);
errvt vmethodimpl(LinuxNetwork, DeviceSend, inst(Buffer) message);
errvt vmethodimpl(LinuxNetwork, DeviceRecv, inst(Buffer) message);
errvt vmethodimpl(LinuxNetwork, DeviceWait, inst(Buffer) message);



errvt vmethodimpl(LinuxNetwork, handleEvents, networkHandle handle, Queue(OSEvent) evntQueue);
u64   vmethodimpl(LinuxNetwork, pollEvents);

const ImplAs(network, LinuxNetwork){
	.initSystem = LinuxNetwork_initSystem,
	.exitSystem = LinuxNetwork_exitSystem,
	.pollEvents = LinuxNetwork_pollEvents,
	.handleEvents = LinuxNetwork_handleEvents,
	.socket = {
		.type = {
			.TCP = 0b1,
			.UDP = 0b10,
			.RAW = 0b11,
			.IPV4 = 0b100,
			.IPV6 = 0b1000,
			.LOCAL = 0b1100
		},
		.init = LinuxNetwork_SocketInit,
		.bind = LinuxNetwork_SocketBind,
		.send = LinuxNetwork_SocketSend,
		.recv = LinuxNetwork_SocketRecv,
		.listen = LinuxNetwork_SocketListen,
		.accept = LinuxNetwork_SocketAccept,
		.connect = LinuxNetwork_SocketConnect,
		.group = {
			.join = LinuxNetwork_SocketGroupJoin,
			.leave = LinuxNetwork_SocketGroupLeave,
			.send = LinuxNetwork_SocketGroupSend,
			.recive = LinuxNetwork_SocketGroupRecive,
		}
	},
	.device = {
		.enumerate = LinuxNetwork_DeviceEnumerate,
		.grab      = LinuxNetwork_DeviceGrab,
		.send	   = LinuxNetwork_DeviceSend,
		.recv  	   = LinuxNetwork_DeviceRecv,
		.drop	   = LinuxNetwork_DeviceDrop,
		.wait	   = LinuxNetwork_DeviceWait,
		.filter    = LinuxNetwork_DeviceFilter
	},
	.obj = {
		.implemented = true,
		.init = LinuxNetwork_NetObjectInit,
		.implement = LinuxNetwork_NetObjectImplement,
		.getInfo   = LinuxNetwork_NetObjectGetInfo,
		.call  = LinuxNetwork_NetObjectCall,
		.find  = LinuxNetwork_NetObjectFind,
		.close = LinuxNetwork_NetObjectClose,
		.get   = LinuxNetwork_NetObjectGet,
		.set   = LinuxNetwork_NetObjectSet
	}			
};
