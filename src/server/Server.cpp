#include "GrpcServer.h"

int main(int argc, char **argv)
{
	std::string server_address("0.0.0.0:50051");
	GrpcServiceImpl service;

	for (int i = 0; i < argc; i++)
		std::cout << "argument" << argv[i] << std::endl;

	if (argc > 1) {
		std::string location(argv[1]);
		service.setMountLocation(location);
	}

	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	//   builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	std::unique_ptr<Server> server(builder.BuildAndStart());
	int vid = 0x1d6b;
	int pid = 0x0003;

	// grpc::AddInsecureChannelFromUsb(server.get(), vid, pid);
	std::cout << "Server listening on " << server_address << std::endl;

	// Wait for the server to shutdown. Note that some other thread must be
	// responsible for shutting down the server for this call to ever return.
	server->Wait();
}
