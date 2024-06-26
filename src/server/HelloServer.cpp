/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpc++/grpc++.h>
#include <grpc/grpc_usb.h>
#include <grpc/grpc.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "build/HelloWorld.grpc.pb.h"
#endif
#include <sys/socket.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
	Status SayHello(ServerContext *context, const HelloRequest *request, HelloReply *reply) override
	{
		std::string prefix("Hello ");
		reply->set_message(prefix + request->name());
		return Status::OK;
	}
};

void RunServer(int VID=0x0525 , int PID = 0xa4a0)
{
	// std::string server_address("0.0.0.0:50051");
	GreeterServiceImpl service;
	grpc_init();
	ServerBuilder builder;
	// builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<Server> server(builder.BuildAndStart());
	GPR_ASSERT(server != nullptr);

	grpc_completion_queue *cq = grpc_completion_queue_create_for_next(nullptr);
	GPR_ASSERT(cq != nullptr);

	grpc_server_register_completion_queue(server->c_server(), cq, nullptr);
	std::cout << "Completion queue registered" << std::endl;
	grpc_channel *channel = grpc_server_add_insecure_channel_from_usb(
		server->c_server(), (void *)0x12341241251, VID, PID); // Replace with your VID and PID
	GPR_ASSERT(channel != nullptr);
	std::cout << "Channel created" << std::endl;

	std::cout << "Server started..." << std::endl;
	int num_requests_handled = 0;
	while (num_requests_handled < 10) {
		std::cout << "Waiting for requests..." << std::endl;
		grpc_event event;
		event = grpc_completion_queue_next(cq, gpr_inf_future(GPR_CLOCK_REALTIME), nullptr);

		if (event.type == GRPC_OP_COMPLETE) {
			num_requests_handled++;
		}
	}
	server->Wait();

	// Shutdown the server
	grpc_server_shutdown_and_notify(server->c_server(), cq, nullptr);
	grpc_server_destroy(server->c_server());
	grpc_completion_queue_destroy(cq);
	std::cout << "Server shutdown" << std::endl;
}

int main(int argc, char **argv)
{
	// get the VID and PID from the command line
	if (argc < 3) {
		std::cerr << "Usage: " << argv[0] << " <VID> <PID>" << std::endl;
		return 1;
	}
	int VID = int(strtol(argv[1], NULL, 16));
	int PID = int(strtol(argv[2], NULL, 16));
	std::cout << "VID: " << VID << ", PID: " << PID << std::endl;
	RunServer(VID, PID);

	return 0;
}
