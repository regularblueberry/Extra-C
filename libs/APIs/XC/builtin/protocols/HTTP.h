#include "../../os.h"

Enum(HTTPMethod,
	HTTP_GET,
	HTTP_POST,
	HTTP_PUT
);

#define HTTPRequest(method, route, ...)	\
	msg(HTTP_Request, HTTP_##method, route, \
     	(HTTPHeader[]){				\
     		__VA_ARGS__			\
	})
#define HTTPResponse(status, ...)		\
	msg(HTTP_Response, status, 		\
     	(HTTPHeader[]){				\
     		__VA_ARGS__			\
	})

Type(HTTPHeader,
	cstr key, value;
)

Protocol(HTTP_Request,
	HTTPMethod method;
	cstr route;
	HTTPHeader* headers;
){}

Protocol(HTTP_Response,
	u16 statuscode;
	HTTPHeader* headers;
){}


Class(HTTPServer,
__INIT(),
__FIELD(),
	
)



void testo(){

	inst(HTTPServer) server = new(HTTPServer);

	Packet packet;
	bool send = true;

    	msgBuild(packet, 
	    if(!send){
		HTTPRequest(GET, "/index.html", 
	 	    {"Age", "54325"},
	      	){
	      	}
	    }else{
		HTTPResponse(200){
		msgText(
			<div> Hello, World! </div>
		)
		};
	    }
	);

	msgSend(NULL, __parent_packet = packet;)
	
}
